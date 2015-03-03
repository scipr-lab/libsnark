/** @file
 *****************************************************************************

 Declaration of interfaces for a delegated random-access memory.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef DELEGATED_RA_MEMORY_HPP_
#define DELEGATED_RA_MEMORY_HPP_

#include <map>
#include <vector>

#include "relations/ram_computations/memory/memory_interface.hpp"

namespace libsnark {

/**
 * A delegated random-access memory maintains the memory's contents via
 * - a map from addresses to values, and
 * - a map from addresses to hashes.
 *
 * The second map maintains the intermediate hashes of a Merkle tree built
 * atop the values currently in memory. Besides offering methods to load and
 * store values, the class offers methods to retrieve the root of the Merkle
 * tree and to obtain the authentication paths for (the value at) a given address.
 */
template<typename HashT>
class delegated_ra_memory : public memory_interface {
private:

    typedef typename HashT::hash_value_type hash_value_type;
    typedef typename HashT::merkle_authentication_path_type merkle_authentication_path_type;

    hash_value_type int_to_hash(const size_t i) const;

public:

    std::vector<hash_value_type> hash_defaults;
    memory_contents values;
    std::map<size_t, hash_value_type> hashes;
    size_t depth;

    delegated_ra_memory(const size_t num_addresses, const size_t value_size);
    delegated_ra_memory(const size_t num_addresses, const size_t value_size, const std::vector<size_t> &contents_as_vector);
    delegated_ra_memory(const size_t num_addresses, const size_t value_size, const memory_contents &contents);

    size_t get_value(const size_t address) const;
    void set_value(const size_t address, const size_t value);

    hash_value_type get_root() const;
    merkle_authentication_path_type get_path(const size_t address) const;

    void dump() const;
};

} // libsnark

#include "relations/ram_computations/memory/delegated_ra_memory.tcc"

#endif // DELEGATED_RA_MEMORY_HPP_
