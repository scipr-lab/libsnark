/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef KC_MULTIEXP_TCC_
#define KC_MULTIEXP_TCC_

namespace libsnark {

template<typename T1, typename T2, mp_size_t n>
knowledge_commitment<T1,T2> opt_window_wnaf_exp(const knowledge_commitment<T1,T2> &base,
                                                const libff::bigint<n> &scalar, const size_t scalar_bits)
{
    return knowledge_commitment<T1,T2>(opt_window_wnaf_exp(base.g, scalar, scalar_bits),
                                       opt_window_wnaf_exp(base.h, scalar, scalar_bits));
}

template<typename T1, typename T2, typename FieldT, libff::multi_exp_method Method>
knowledge_commitment<T1, T2> kc_multi_exp_with_mixed_addition(const knowledge_commitment_vector<T1, T2> &vec,
                                                                const size_t min_idx,
                                                                const size_t max_idx,
                                                                typename std::vector<FieldT>::const_iterator scalar_start,
                                                                typename std::vector<FieldT>::const_iterator scalar_end,
                                                                const size_t chunks)
{
    const size_t scalar_length = std::distance(scalar_start, scalar_end);
#ifndef NDEBUG
    assert((size_t)(scalar_length) <= vec.domain_size_);
#else
    libff::UNUSED(scalar_length);
#endif

    libff::enter_block("Process scalar vector");
    auto index_it = std::lower_bound(vec.indices.begin(), vec.indices.end(), min_idx);
    const size_t offset = index_it - vec.indices.begin();

    auto value_it = vec.values.begin() + offset;

    const FieldT zero = FieldT::zero();
    const FieldT one = FieldT::one();

    std::vector<FieldT> p;
    std::vector<knowledge_commitment<T1, T2> > g;

    knowledge_commitment<T1, T2> acc = knowledge_commitment<T1, T2>::zero();

    size_t num_skip = 0;
    size_t num_add = 0;
    size_t num_other = 0;

    while (index_it != vec.indices.end() && *index_it < max_idx)
    {
        const size_t scalar_position = (*index_it) - min_idx;
#ifdef DEBUG
        assert(scalar_position < scalar_length);
#endif

        const FieldT scalar = *(scalar_start + scalar_position);

        if (scalar == zero)
        {
            // do nothing
            ++num_skip;
        }
        else if (scalar == one)
        {
#ifdef USE_MIXED_ADDITION
            acc.g = acc.g.mixed_add(value_it->g);
            acc.h = acc.h.mixed_add(value_it->h);
#else
            acc.g = acc.g + value_it->g;
            acc.h = acc.h + value_it->h;
#endif
            ++num_add;
        }
        else
        {
            p.emplace_back(scalar);
            g.emplace_back(*value_it);
            ++num_other;
        }

        ++index_it;
        ++value_it;
    }
    
    if (!libff::inhibit_profiling_info)
    {
    libff::print_indent(); printf("* Elements of w skipped: %zu (%0.2f%%)\n", num_skip, 100.*num_skip/(num_skip+num_add+num_other));
    libff::print_indent(); printf("* Elements of w processed with special addition: %zu (%0.2f%%)\n", num_add, 100.*num_add/(num_skip+num_add+num_other));
    libff::print_indent(); printf("* Elements of w remaining: %zu (%0.2f%%)\n", num_other, 100.*num_other/(num_skip+num_add+num_other));
    }
    
    libff::leave_block("Process scalar vector");

    return acc + libff::multi_exp<knowledge_commitment<T1, T2>, FieldT, Method>(g.begin(), g.end(), p.begin(), p.end(), chunks);
}

template<typename T1, typename T2, typename FieldT>
knowledge_commitment_vector<T1, T2> kc_batch_exp_internal(const size_t scalar_size,
                                                          const size_t T1_window,
                                                          const size_t T2_window,
                                                          const libff::window_table<T1> &T1_table,
                                                          const libff::window_table<T2> &T2_table,
                                                          const FieldT &T1_coeff,
                                                          const FieldT &T2_coeff,
                                                          const std::vector<FieldT> &v,
                                                          const size_t start_pos,
                                                          const size_t end_pos,
                                                          const size_t expected_size)
{
    knowledge_commitment_vector<T1, T2> res;

    res.values.reserve(expected_size);
    res.indices.reserve(expected_size);

    for (size_t pos = start_pos; pos != end_pos; ++pos)
    {
        if (!v[pos].is_zero())
        {
            res.values.emplace_back(knowledge_commitment<T1, T2>(windowed_exp(scalar_size, T1_window, T1_table, T1_coeff * v[pos]),
                                                                 windowed_exp(scalar_size, T2_window, T2_table, T2_coeff * v[pos])));
            res.indices.emplace_back(pos);
        }
    }

    return res;
}

template<typename T1, typename T2, typename FieldT>
knowledge_commitment_vector<T1, T2> kc_batch_exp(const size_t scalar_size,
                                                 const size_t T1_window,
                                                 const size_t T2_window,
                                                 const libff::window_table<T1> &T1_table,
                                                 const libff::window_table<T2> &T2_table,
                                                 const FieldT &T1_coeff,
                                                 const FieldT &T2_coeff,
                                                 const std::vector<FieldT> &v,
                                                 const size_t suggested_num_chunks)
{
    knowledge_commitment_vector<T1, T2> res;
    res.domain_size_ = v.size();

    size_t nonzero = 0;
    for (size_t i = 0; i < v.size(); ++i)
    {
        nonzero += (v[i].is_zero() ? 0 : 1);
    }

    const size_t num_chunks = std::max((size_t)1, std::min(nonzero, suggested_num_chunks));

    if (!libff::inhibit_profiling_info)
    {
        libff::print_indent(); printf("Non-zero coordinate count: %zu/%zu (%0.2f%%)\n", nonzero, v.size(), 100.*nonzero/v.size());
    }

    std::vector<knowledge_commitment_vector<T1, T2> > tmp(num_chunks);
    std::vector<size_t> chunk_pos(num_chunks+1);

    const size_t chunk_size = nonzero / num_chunks;
    const size_t last_chunk = nonzero - chunk_size * (num_chunks - 1);

    chunk_pos[0] = 0;

    size_t cnt = 0;
    size_t chunkno = 1;

    for (size_t i = 0; i < v.size(); ++i)
    {
        cnt += (v[i].is_zero() ? 0 : 1);
        if (cnt == chunk_size && chunkno < num_chunks)
        {
            chunk_pos[chunkno] = i;
            cnt = 0;
            ++chunkno;
        }
    }

    chunk_pos[num_chunks] = v.size();

#ifdef MULTICORE
#pragma omp parallel for
#endif
    for (size_t i = 0; i < num_chunks; ++i)
    {
        tmp[i] = kc_batch_exp_internal<T1, T2, FieldT>(scalar_size, T1_window, T2_window, T1_table, T2_table, T1_coeff, T2_coeff, v,
                                                       chunk_pos[i], chunk_pos[i+1], i == num_chunks - 1 ? last_chunk : chunk_size);
#ifdef USE_MIXED_ADDITION
        libff::batch_to_special<knowledge_commitment<T1, T2>>(tmp[i].values);
#endif
    }

    if (num_chunks == 1)
    {
        tmp[0].domain_size_ = v.size();
        return tmp[0];
    }
    else
    {
        for (size_t i = 0; i < num_chunks; ++i)
        {
            res.values.insert(res.values.end(), tmp[i].values.begin(), tmp[i].values.end());
            res.indices.insert(res.indices.end(), tmp[i].indices.begin(), tmp[i].indices.end());
        }
        return res;
    }
}

} // libsnark

#endif // KC_MULTIEXP_TCC_
