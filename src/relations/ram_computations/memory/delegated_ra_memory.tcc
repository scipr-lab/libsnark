/** @file
 *****************************************************************************

 Implementation of interfaces for a delegated random-access memory.

 See delegated_ra_memory.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef DELEGATED_RA_MEMORY_TCC
#define DELEGATED_RA_MEMORY_TCC

#include <algorithm>

#include "common/profiling.hpp"
#include "common/utils.hpp"

namespace libsnark {

template<typename HashT>
typename HashT::hash_value_type two_to_one_CRH(const typename HashT::hash_value_type &l,
                                               const typename HashT::hash_value_type &r)
{
    typename HashT::hash_value_type new_input;
    new_input.insert(new_input.end(), l.begin(), l.end());
    new_input.insert(new_input.end(), r.begin(), r.end());

    const size_t digest_size = HashT::get_digest_len();
    assert(l.size() == digest_size);
    assert(r.size() == digest_size);

    return HashT::get_hash(new_input);
}

template<typename HashT>
typename HashT::hash_value_type delegated_ra_memory<HashT>::int_to_hash(const size_t i) const
{
    const size_t digest_size = HashT::get_digest_len();
    hash_value_type result = int_list_to_bits({ i }, value_size);
    std::reverse(result.begin(), result.end());
    result.resize(digest_size);
    return result;
}

template<typename HashT>
delegated_ra_memory<HashT>::delegated_ra_memory(const size_t num_addresses,
                                                const size_t value_size) :
    memory_interface(num_addresses, value_size)
{
    const size_t digest_size = HashT::get_digest_len();
    HashT::sample_randomness(2*digest_size);
    depth = log2(num_addresses);
    printf("%zu %zu\n", num_addresses, depth);

    hash_value_type last = int_to_hash(0);
    hash_defaults.emplace_back(last);
    for (size_t i = 0; i < depth; ++i)
    {
        last = two_to_one_CRH<HashT>(last, last);
        hash_defaults.emplace_back(last);
    }

    std::reverse(hash_defaults.begin(), hash_defaults.end());
}

template<typename HashT>
delegated_ra_memory<HashT>::delegated_ra_memory(const size_t num_addresses,
                                                const size_t value_size,
                                                const std::vector<size_t> &contents_as_vector) :
    delegated_ra_memory<HashT>(num_addresses, value_size)
{
    assert(contents_as_vector.size() < 1ul<<depth);
    for (size_t address = 0; address < contents_as_vector.size(); ++address)
    {
        const size_t idx = address + (1ul<<depth) - 1;
        values[idx] = contents_as_vector[address];
        hashes[idx] = int_to_hash(contents_as_vector[address]);
    }

    size_t idx_begin = (1ul<<depth) - 1;
    size_t idx_end = contents_as_vector.size() + ((1ul<<depth) - 1);

    for (int layer = depth; layer > 0; --layer)
    {
        for (size_t idx = idx_begin; idx < idx_end; idx += 2)
        {
            hash_value_type l = hashes[idx]; // this is sound, because idx_begin is always a left child
            hash_value_type r = (idx + 1 < idx_end ? hashes[idx+1] : hash_defaults[layer]);

            hash_value_type h = two_to_one_CRH<HashT>(l, r);
            hashes[(idx-1)/2] = h;
        }

        idx_begin = (idx_begin-1)/2;
        idx_end = (idx_end-1)/2;
    }
}

template<typename HashT>
delegated_ra_memory<HashT>::delegated_ra_memory(const size_t num_addresses,
                                                const size_t value_size,
                                                const std::map<size_t, size_t> &contents) :
    delegated_ra_memory<HashT>(num_addresses, value_size)
{
    enter_block("Construct delegated_ra_memory from memory_contents");
    printf("%zu %zu\n", num_addresses, depth);

    if (!contents.empty())
    {
        printf("%zu %zu\n", contents.rbegin()->first, depth);
        assert(contents.rbegin()->first < 1ul<<depth);

        for (auto it = contents.begin(); it != contents.end(); ++it)
        {
            const size_t address = it->first;
            const size_t value = it->second;
            const size_t idx = address + (1ul<<depth) - 1;

            values[address] = value;
            hashes[idx] = int_to_hash(value);
        }

        auto last_it = hashes.end();

        for (int layer = depth; layer > 0; --layer)
        {
            auto next_last_it = hashes.begin();

            for (auto it = hashes.begin(); it != last_it; ++it)
            {
                const size_t idx = it->first;
                const hash_value_type hash = it->second;

                if (idx % 2 == 0)
                {
                    // this is the right child of its parent and by invariant we are missing the left child
                    hashes[(idx-1)/2] = two_to_one_CRH<HashT>(hash_defaults[layer], hash);
                }
                else
                {
                    if (std::next(it) == last_it || std::next(it)->first != idx + 1)
                    {
                        // this is the left child of its parent and is missing its right child
                        hashes[(idx-1)/2] = two_to_one_CRH<HashT>(hash, hash_defaults[layer]);
                    }
                    else
                    {
                        // typical case: this is the left child of the parent and adjecent to it there is a right child
                        hashes[(idx-1)/2] = two_to_one_CRH<HashT>(hash, std::next(it)->second);
                        ++it;
                    }
                }
            }

            last_it = next_last_it;
        }
    }

    leave_block("Construct delegated_ra_memory from memory_contents");
}

template<typename HashT>
size_t delegated_ra_memory<HashT>::get_value(const size_t address) const
{
    assert(address < 1ul<<depth);

    auto it = values.find(address);
    return (it == values.end() ? 0 : it->second);
}

template<typename HashT>
void delegated_ra_memory<HashT>::set_value(const size_t address,
                                           const size_t value)
{
    assert(address < 1ul<<depth);
    size_t idx = address + (1ul<<depth) - 1;

    values[address] = value;
    hashes[idx] = int_to_hash(value);
    for (int layer = depth-1; layer >=0; --layer)
    {
        idx = (idx-1)/2;

        auto it = hashes.find(2*idx+1);
        hash_value_type l = (it == hashes.end() ? hash_defaults[layer+1] : it->second);

        it = hashes.find(2*idx+2);
        hash_value_type r = (it == hashes.end() ? hash_defaults[layer+1] : it->second);

        hash_value_type h = two_to_one_CRH<HashT>(l, r);
        hashes[idx] = h;
    }
}

template<typename HashT>
typename HashT::hash_value_type delegated_ra_memory<HashT>::get_root() const
{
    auto it = hashes.find(0);
    return (it == hashes.end() ? hash_defaults[0] : it->second);
}

template<typename HashT>
typename HashT::merkle_authentication_path_type delegated_ra_memory<HashT>::get_path(const size_t address) const
{
    typename HashT::merkle_authentication_path_type result(depth);
    assert(address < 1ul<<depth);
    size_t idx = address + (1ul<<depth) - 1;

    for (size_t layer = depth; layer > 0; --layer)
    {
        size_t sibling_idx = ((idx + 1) ^ 1) - 1;
        auto it = hashes.find(sibling_idx);
        if (layer == depth)
        {
            auto it2 = values.find(sibling_idx - ((1ul<<depth) - 1));
            const size_t v = (it2 == values.end() ? 0 : it2->second);
            result[layer-1].aux_digest = int_to_hash(v);
        }
        else
        {
            result[layer-1].aux_digest = (it == hashes.end() ? hash_defaults[layer] : it->second);
        }
        result[layer-1].computed_is_right = ((idx & 1) == 0);

        idx = (idx-1)/2;
    }

    return result;
}

template<typename HashT>
void delegated_ra_memory<HashT>::dump() const
{
    for (size_t i = 0; i < num_addresses; ++i)
    {
        auto it = values.find(i);
        size_t value = (it == values.end() ? 0 : it->second);
        printf("%zu ", value);
    }
    printf("\n");
}

} // libsnark

#endif // DELEGATED_RA_MEMORY_TCC
