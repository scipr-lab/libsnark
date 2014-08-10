/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MULTIEXP_TCC_
#define MULTIEXP_TCC_

#include "encoding/multiexp.hpp"

#include <algorithm>
#include <cassert>
#include <type_traits>

#include "common/profiling.hpp"
#include "common/utils.hpp"

namespace libsnark {

template<typename T1, typename T2, mp_size_t n>
knowledge_commitment<T1,T2> opt_window_wnaf_exp(const knowledge_commitment<T1,T2> &neutral,
                                                const knowledge_commitment<T1,T2> &base,
                                                const bigint<n> &scalar, const size_t scalar_bits)
{
    return knowledge_commitment<T1,T2>(opt_window_wnaf_exp(neutral.g, base.g, scalar, scalar_bits),
                                       opt_window_wnaf_exp(neutral.h, base.h, scalar, scalar_bits));
}

template<mp_size_t n>
class ordered_exponent {
// to use std::push_heap and friends later
public:
    size_t idx;
    bigint<n> r;

    ordered_exponent(const size_t idx, const bigint<n> &r) : idx(idx), r(r) {};

    bool operator<(const ordered_exponent<n> &other) const
    {
#if defined(__x86_64__) && defined(USE_ASM)
    if (n == 3)
    {
        long res;
        __asm__
            ("// check for overflow           \n\t"
             "mov $0, %[res]                  \n\t"
             ADD_CMP(16)
             ADD_CMP(8)
             ADD_CMP(0)
             "jmp done%=                      \n\t"
             "subtract%=:                     \n\t"
             "mov $1, %[res]                  \n\t"
             "done%=:                         \n\t"
             : [res] "=&r" (res)
             : [A] "r" (other.r.data), [mod] "r" (this->r.data)
             : "cc", "%rax");
        return res;
    }
    else if (n == 4)
    {
        long res;
        __asm__
            ("// check for overflow           \n\t"
             "mov $0, %[res]                  \n\t"
             ADD_CMP(24)
             ADD_CMP(16)
             ADD_CMP(8)
             ADD_CMP(0)
             "jmp done%=                      \n\t"
             "subtract%=:                     \n\t"
             "mov $1, %[res]                  \n\t"
             "done%=:                         \n\t"
             : [res] "=&r" (res)
             : [A] "r" (other.r.data), [mod] "r" (this->r.data)
             : "cc", "%rax");
        return res;
    }
    else if (n == 5)
    {
        long res;
        __asm__
            ("// check for overflow           \n\t"
             "mov $0, %[res]                  \n\t"
             ADD_CMP(32)
             ADD_CMP(24)
             ADD_CMP(16)
             ADD_CMP(8)
             ADD_CMP(0)
             "jmp done%=                      \n\t"
             "subtract%=:                     \n\t"
             "mov $1, %[res]                  \n\t"
             "done%=:                         \n\t"
             : [res] "=&r" (res)
             : [A] "r" (other.r.data), [mod] "r" (this->r.data)
             : "cc", "%rax");
        return res;
    }
    else
#endif
    {
        return (mpn_cmp(this->r.data, other.r.data, n) < 0);
    }
    }
};

template<typename T, typename FieldT>
T naive_exp(const T &neutral,
            typename std::vector<T>::const_iterator vec_start,
            typename std::vector<T>::const_iterator vec_end,
            typename std::vector<FieldT>::const_iterator scalar_start,
            typename std::vector<FieldT>::const_iterator scalar_end)
{
    T result(neutral);

    typename std::vector<T>::const_iterator vec_it;
    typename std::vector<FieldT>::const_iterator scalar_it;

    for (vec_it = vec_start, scalar_it = scalar_start; vec_it != vec_end; ++vec_it, ++scalar_it)
    {
        bigint<FieldT::num_limbs> scalar_bigint = scalar_it->as_bigint();
        result = result + opt_window_wnaf_exp(neutral, *vec_it, scalar_bigint, scalar_bigint.num_bits());
    }
    assert(scalar_it == scalar_end);

    return result;
}

/*
   The multi-exponentiation algorithm below is a variant of the Bos-Coster algorithm
   [Bos and Coster, "Addition chain heuristics", CRYPTO '89].
   The implementation uses suggestions from
   [Bernstein, Duif, Lange, Schwabe, and Yang, "High-speed high-security signatures", CHES '11].
*/
template<typename T, typename FieldT>
T multi_exp_inner(const T &neutral,
            typename std::vector<T>::const_iterator vec_start,
            typename std::vector<T>::const_iterator vec_end,
            typename std::vector<FieldT>::const_iterator scalar_start,
            typename std::vector<FieldT>::const_iterator scalar_end)
{
    const mp_size_t n = std::remove_reference<decltype(*scalar_start)>::type::num_limbs;

    if (vec_start == vec_end)
    {
        return neutral;
    }

    if (vec_start + 1 == vec_end)
    {
        return (*scalar_start)*(*vec_start);
    }

    std::vector<ordered_exponent<n> > opt_q;
    const size_t vec_len = scalar_end - scalar_start;
    const size_t odd_vec_len = (vec_len % 2 == 1 ? vec_len : vec_len + 1);
    opt_q.reserve(odd_vec_len);
    std::vector<T> g;
    g.reserve(odd_vec_len);

    typename std::vector<T>::const_iterator vec_it;
    typename std::vector<FieldT>::const_iterator scalar_it;
    size_t i;
    for (i=0, vec_it = vec_start, scalar_it = scalar_start; vec_it != vec_end; ++vec_it, ++scalar_it, ++i)
    {
        g.emplace_back(*vec_it);

        opt_q.emplace_back(ordered_exponent<n>(i, scalar_it->as_bigint()));
    }
    std::make_heap(opt_q.begin(),opt_q.end());
    assert(scalar_it == scalar_end);

    if (vec_len != odd_vec_len)
    {
        g.emplace_back(neutral);
        opt_q.emplace_back(ordered_exponent<n>(odd_vec_len - 1, bigint<n>(0ul)));
    }
    assert(g.size() % 2 == 1);
    assert(opt_q.size() == g.size());

    T opt_result = neutral;

    while (true)
    {
        ordered_exponent<n> &a = opt_q[0];
        ordered_exponent<n> &b = (opt_q[1] < opt_q[2] ? opt_q[2] : opt_q[1]);

        const size_t abits = a.r.num_bits();

        if (b.r.is_zero())
        {
            // opt_result = opt_result + (a.r * g[a.idx]);
            opt_result = opt_result + opt_window_wnaf_exp(neutral, g[a.idx], a.r, abits);
            break;
        }

        const size_t bbits = b.r.num_bits();
        const size_t limit = (abits-bbits >= 20 ? 20 : abits-bbits);

        if (bbits < 1u<<limit)
        {
            /*
               In this case, exponentiating to the power of a is cheaper than
               subtracting b from a multiple times, so let's do it directly
            */
            // opt_result = opt_result + (a.r * g[a.idx]);
            opt_result = opt_result + opt_window_wnaf_exp(neutral, g[a.idx], a.r, abits);
#ifdef DEBUG
            printf("Skipping the following pair (%zu bit number vs %zu bit):\n", abits, bbits);
            a.r.print();
            b.r.print();
#endif
            a.r.clear();
        }
        else
        {
            // x A + y B => (x-y) A + y (B+A)
            mpn_sub_n(a.r.data, a.r.data, b.r.data, n);
            g[b.idx] = g[b.idx] + g[a.idx];
        }

        // regardless of whether a was cleared or subtracted from we push it down, then take back up

        /* heapify A down */
        size_t a_pos = 0;
        while (2*a_pos + 2< odd_vec_len)
        {
            // this is a max-heap so to maintain a heap property we swap with the largest of the two
            if (opt_q[2*a_pos+1] < opt_q[2*a_pos+2])
            {
                std::swap(opt_q[a_pos], opt_q[2*a_pos+2]);
                a_pos = 2*a_pos+2;
            }
            else
            {
                std::swap(opt_q[a_pos], opt_q[2*a_pos+1]);
                a_pos = 2*a_pos+1;
            }
        }

        /* now heapify A up appropriate amount of times */
        while (a_pos > 0 && opt_q[(a_pos-1)/2] < opt_q[a_pos])
        {
            std::swap(opt_q[a_pos], opt_q[(a_pos-1)/2]);
            a_pos = (a_pos-1) / 2;
        }
    }

    return opt_result;
}

template<typename T, typename FieldT>
T multi_exp(const T &neutral,
               typename std::vector<T>::const_iterator vec_start,
               typename std::vector<T>::const_iterator vec_end,
               typename std::vector<FieldT>::const_iterator scalar_start,
               typename std::vector<FieldT>::const_iterator scalar_end,
               const size_t chunks,
               const bool use_multiexp)
{
    const size_t total = vec_end - vec_start;
    if (total < chunks)
    {
        return naive_exp<T, FieldT>(neutral, vec_start, vec_end, scalar_start, scalar_end);
    }

    const size_t one = total/chunks;

    std::vector<T> partial(chunks, neutral);

    if (use_multiexp)
    {
#ifdef MULTICORE
#pragma omp parallel for
#endif
        for (size_t i = 0; i < chunks; ++i)
        {
            partial[i] = multi_exp_inner<T, FieldT>(neutral,
                                                    vec_start + i*one,
                                                    (i == chunks-1 ? vec_end : vec_start + (i+1)*one),
                                                    scalar_start + i*one,
                                                    (i == chunks-1 ? scalar_end : scalar_start + (i+1)*one));
        }
    }
    else
    {
#ifdef MULTICORE
#pragma omp parallel for
#endif
        for (size_t i = 0; i < chunks; ++i)
        {
            partial[i] = naive_exp<T, FieldT>(neutral,
                                              vec_start + i*one,
                                              (i == chunks-1 ? vec_end : vec_start + (i+1)*one),
                                              scalar_start + i*one,
                                              (i == chunks-1 ? scalar_end : scalar_start + (i+1)*one));
        }
    }

    T final = neutral;

    for (size_t i = 0; i < chunks; ++i)
    {
        final = final + partial[i];
    }

    return final;
}

template<typename T1, typename T2, typename FieldT>
knowledge_commitment<T1, T2> kc_multi_exp_with_fast_add_special(const knowledge_commitment<T1, T2> &neutral,
                                                           const knowledge_commitment_vector<T1, T2> &vec,
                                                           const size_t min_idx,
                                                           const size_t max_idx,
                                                           typename std::vector<FieldT>::const_iterator scalar_start,
                                                           typename std::vector<FieldT>::const_iterator scalar_end,
                                                           const size_t chunks,
                                                           const bool use_multiexp)
{
    if (vec.is_sparse)
    {
        enter_block("Process scalar vector");
        auto index_it = std::lower_bound(vec.indices.begin(), vec.indices.end(), min_idx);
        const size_t offset = index_it - vec.indices.begin();

        auto value_it = vec.values.begin() + offset;

        const FieldT zero = FieldT::zero();
        const FieldT one = FieldT::one();

        std::vector<FieldT> p;
        std::vector<knowledge_commitment<T1, T2> > g;

        knowledge_commitment<T1, T2> acc = neutral;

        size_t num_skip = 0;
        size_t num_add = 0;
        size_t num_other = 0;

        while (index_it != vec.indices.end() && *index_it < max_idx)
        {
            const FieldT scalar = *(scalar_start + ((*index_it) - min_idx));

            if (scalar == zero)
            {
                // do nothing
                ++num_skip;
            }
            else if (scalar == one)
            {
#ifdef USE_ADD_SPECIAL
                acc.g = acc.g.fast_add_special(value_it->g);
                acc.h = acc.h.fast_add_special(value_it->h);
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

        print_indent(); printf("* Elements of w skipped: %zu (%0.2f%%)\n", num_skip, 100.*num_skip/(num_skip+num_add+num_other));
        print_indent(); printf("* Elements of w processed with special addition: %zu (%0.2f%%)\n", num_add, 100.*num_add/(num_skip+num_add+num_other));
        print_indent(); printf("* Elements of w remaining: %zu (%0.2f%%)\n", num_other, 100.*num_other/(num_skip+num_add+num_other));
        leave_block("Process scalar vector");

        return acc + multi_exp<knowledge_commitment<T1, T2>, FieldT>(neutral, g.begin(), g.end(), p.begin(), p.end(), chunks, use_multiexp);
    }
    else
    {
        return multi_exp<knowledge_commitment<T1, T2>, FieldT>(neutral, vec.values.begin() + min_idx, vec.values.begin() + max_idx, scalar_start, scalar_end, chunks, use_multiexp);
    }
}

template<typename T, typename FieldT>
T multi_exp_with_fast_add_special(const T &neutral,
                             typename std::vector<T>::const_iterator vec_start,
                             typename std::vector<T>::const_iterator vec_end,
                             typename std::vector<FieldT>::const_iterator scalar_start,
                             typename std::vector<FieldT>::const_iterator scalar_end,
                             const size_t chunks,
                             const bool use_multiexp)
{
    enter_block("Process scalar vector");
    auto value_it = vec_start;
    auto scalar_it = scalar_start;

    const FieldT zero = FieldT::zero();
    const FieldT one = FieldT::one();
    std::vector<FieldT> p;
    std::vector<T> g;

    T acc = neutral;

    size_t num_skip = 0;
    size_t num_add = 0;
    size_t num_other = 0;

    for (; scalar_it != scalar_end; ++scalar_it, ++value_it)
    {
        if (*scalar_it == zero)
        {
            // do nothing
            ++num_skip;
        }
        else if (*scalar_it == one)
        {
#ifdef USE_ADD_SPECIAL
            acc = acc.fast_add_special(*value_it);
#else
            acc = acc + (*value_it);
#endif
            ++num_add;
        }
        else
        {
            p.emplace_back(*scalar_it);
            g.emplace_back(*value_it);
            ++num_other;
        }
    }
    print_indent(); printf("* Elements of w skipped: %zu (%0.2f%%)\n", num_skip, 100.*num_skip/(num_skip+num_add+num_other));
    print_indent(); printf("* Elements of w processed with special addition: %zu (%0.2f%%)\n", num_add, 100.*num_add/(num_skip+num_add+num_other));
    print_indent(); printf("* Elements of w remaining: %zu (%0.2f%%)\n", num_other, 100.*num_other/(num_skip+num_add+num_other));

    leave_block("Process scalar vector");

    return acc + multi_exp<T, FieldT>(neutral, g.begin(), g.end(), p.begin(), p.end(), chunks, use_multiexp);
}

template<typename T>
size_t get_exp_window_size(const size_t exp_count)
{
    if (T::fixed_base_exp_window_table.empty())
    {
#ifdef LOWMEM
        return 14;
#else
        return 17;
#endif
    }
    size_t window = 1;
    for (long i = T::fixed_base_exp_window_table.size()-1; i >= 0; --i)
    {
#ifdef DEBUG
        printf("%zu %zu %zu\n", i, exp_count, T::fixed_base_exp_window_table[i]);
#endif
        if (T::fixed_base_exp_window_table[i] != 0 && exp_count >= T::fixed_base_exp_window_table[i])
        {
            window = i+1;
            break;
        }
    }

    print_indent(); printf("Choosing window size %zu for %zu elements\n", window, exp_count);

#ifdef LOWMEM
    window = std::min(14, window);
#endif
    return window;
}

template<typename T>
window_table<T> get_window_table(const size_t exp_size,
                                 const T &neutral,
                                 const size_t window,
                                 const T &g)
{
    const size_t in_window = 1u<<window;
    const size_t outerc = (exp_size+window-1)/window;
    const size_t last_in_window = 1u<<(exp_size - (outerc-1)*window);
#ifdef DEBUG
    print_indent(); printf("* exp_size=%zu; window=%zu; in_window=%zu; outerc=%zu\n", exp_size, window, in_window, outerc);
#endif

    window_table<T> powers_of_g(outerc, std::vector<T>(in_window, neutral));

    T gouter = g;

    for (size_t outer = 0; outer < outerc; ++outer)
    {
        T ginner = neutral;
        size_t cur_in_window = outer == outerc-1 ? last_in_window : in_window;
        for (size_t inner = 0; inner < cur_in_window; ++inner)
        {
            powers_of_g[outer][inner] = ginner;
            ginner = ginner + gouter;
        }

        for (size_t i = 0; i < window; ++i)
        {
            gouter = gouter + gouter;
        }
    }

    return powers_of_g;
}

template<typename T, typename FieldT>
T windowed_exp(const size_t exp_size,
               const size_t window,
               const window_table<T> &powers_of_g,
               const FieldT &pow)
{
    const size_t outerc = (exp_size+window-1)/window;
    const bigint<FieldT::num_limbs> pow_val = pow.as_bigint();

    /* exp */
    T res = powers_of_g[0][0];

    for (size_t outer = 0; outer < outerc; ++outer)
    {
        size_t inner = 0;
        for (size_t i = 0; i < window; ++i)
        {
            if (pow_val.test_bit(outer*window + i))
            {
                inner |= 1u << i;
            }
        }

        res = res + powers_of_g[outer][inner];
    }

    return res;
}

template<typename T, typename FieldT>
std::vector<T> batch_exp(const size_t exp_size,
                         const size_t window,
                         const window_table<T> &table,
                         const std::vector<FieldT> &v)
{
    if (!inhibit_profiling_info)
    {
        print_indent();
    }
    std::vector<T> res(v.size(), table[0][0]);

#ifdef MULTICORE
#pragma omp parallel for
#endif
    for (size_t i = 0; i < v.size(); ++i)
    {
        res[i] = windowed_exp(exp_size, window, table, v[i]);

        if (!inhibit_profiling_info && (i % 10000 == 0))
        {
            printf(".");
            fflush(stdout);
        }
    }

    if (!inhibit_profiling_info)
    {
        printf(" DONE!\n");
    }

    return res;
}

template<typename T>
void batch_to_special(std::vector<T> &vec)
{
    enter_block("Batch-convert elements to special form");

    std::vector<T> non_zero_vec;
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (!vec[i].is_zero())
        {
            non_zero_vec.emplace_back(vec[i]);
        }
    }

    batch_to_special_all_non_zeros<T>(non_zero_vec);
    auto it = non_zero_vec.begin();
    T zero_special = T::zero();
    zero_special.to_special();

    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (!vec[i].is_zero())
        {
            vec[i] = *it;
            ++it;
        }
        else
        {
            vec[i] = zero_special;
        }
    }
    leave_block("Batch-convert elements to special form");
}

template<typename T1, typename T2>
void kc_batch_to_special(std::vector<knowledge_commitment<T1, T2> > &vec)
{
    enter_block("Batch-convert knowledge-commitments to special form");

    std::vector<T1> g_vec;
    g_vec.reserve(vec.size());

    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (!vec[i].g.is_zero())
        {
            g_vec.emplace_back(vec[i].g);
        }
    }

    batch_to_special_all_non_zeros<T1>(g_vec);
    auto g_it = g_vec.begin();
    T1 T1_zero_special = T1::zero();
    T1_zero_special.to_special();

    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (!vec[i].g.is_zero())
        {
            vec[i].g = *g_it;
            ++g_it;
        }
        else
        {
            vec[i].g = T1_zero_special;
        }
    }

    g_vec.clear();

    std::vector<T2> h_vec;
    h_vec.reserve(vec.size());

    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (!vec[i].h.is_zero())
        {
            h_vec.emplace_back(vec[i].h);
        }
    }

    batch_to_special_all_non_zeros<T2>(h_vec);
    auto h_it = h_vec.begin();
    T2 T2_zero_special = T2::zero();
    T2_zero_special.to_special();

    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (!vec[i].h.is_zero())
        {
            vec[i].h = *h_it;
            ++h_it;
        }
        else
        {
            vec[i].h = T2_zero_special;
        }
    }

    g_vec.clear();

    leave_block("Batch-convert knowledge-commitments to special form");
}

template<typename T1, typename T2, typename FieldT>
knowledge_commitment_vector<T1, T2> kc_batch_exp_internal(const size_t exp_size,
                                                          const size_t T1_window,
                                                          const size_t T2_window,
                                                          const window_table<T1> &T1_table,
                                                          const window_table<T2> &T2_table,
                                                          const FieldT &T1_coeff,
                                                          const FieldT &T2_coeff,
                                                          const std::vector<FieldT> v,
                                                          const size_t start_pos,
                                                          const size_t end_pos,
                                                          const bool is_sparse,
                                                          const size_t expected_size)
{
    knowledge_commitment_vector<T1, T2> res;
    res.is_sparse = is_sparse;

    res.values.reserve(expected_size);

    if (is_sparse)
    {
        res.indices.reserve(expected_size);
    }

    for (size_t pos = start_pos; pos != end_pos; ++pos)
    {
        if (!is_sparse || !v[pos].is_zero())
        {
            res.values.emplace_back(knowledge_commitment<T1, T2>(windowed_exp(exp_size, T1_window, T1_table, T1_coeff * v[pos]),
                                                                 windowed_exp(exp_size, T2_window, T2_table, T2_coeff * v[pos])));
        }

        if (is_sparse && !v[pos].is_zero())
        {
            res.indices.emplace_back(pos);
        }
    }

    return res;
}

template<typename T1, typename T2, typename FieldT>
knowledge_commitment_vector<T1, T2> kc_batch_exp(const size_t exp_size,
                                                 const size_t T1_window,
                                                 const size_t T2_window,
                                                 const window_table<T1> &T1_table,
                                                 const window_table<T2> &T2_table,
                                                 const FieldT &T1_coeff,
                                                 const FieldT &T2_coeff,
                                                 const std::vector<FieldT> &v,
                                                 const bool is_sparse,
                                                 const size_t chunks)
{
    knowledge_commitment_vector<T1, T2> res;
    res.is_sparse = is_sparse;
    res.original_size = v.size();

    size_t nonzero = 0;
    for (size_t i = 0; i < v.size(); ++i)
    {
        nonzero += (v[i].is_zero() ? 0 : 1);
    }

    if (!inhibit_profiling_info)
    {
        print_indent(); printf("Non-zero coordinate count: %zu/%zu (%0.2f%%)\n", nonzero, v.size(), 100.*nonzero/v.size());
    }

    std::vector<knowledge_commitment_vector<T1, T2> > tmp(chunks);
    std::vector<size_t> chunk_pos(chunks+1);

    const size_t chunk_size = (is_sparse ? nonzero / chunks : v.size() / chunks);
    const size_t last_chunk = (is_sparse ? nonzero : v.size()) - chunk_size * (chunks - 1);

    if (is_sparse)
    {
        chunk_pos[0] = 0;

        size_t cnt = 0;
        size_t chunkno = 1;

        for (size_t i = 0; i < v.size(); ++i)
        {
            cnt += (v[i].is_zero() ? 0 : 1);
            if (cnt == chunk_size)
            {
                chunk_pos[chunkno] = i;
                cnt = 0;
                ++chunkno;
            }
        }
    }
    else
    {
        const size_t chunk_size = v.size() / chunks;
        for (size_t i = 0; i < chunks; ++i)
        {
            chunk_pos[i] = i * chunk_size;
        }
    }
    chunk_pos[chunks] = v.size();

#ifdef MULTICORE
#pragma omp parallel for
#endif
    for (size_t i = 0; i < chunks; ++i)
    {
        tmp[i] = kc_batch_exp_internal<T1, T2, FieldT>(exp_size, T1_window, T2_window, T1_table, T2_table, T1_coeff, T2_coeff, v,
                                                       chunk_pos[i], chunk_pos[i+1], is_sparse, i == chunks - 1 ? last_chunk : chunk_size);
#ifdef USE_ADD_SPECIAL
        kc_batch_to_special<T1, T2>(tmp[i].values);
#endif
    }

    if (chunks == 1)
    {
        tmp[0].original_size = v.size();
        return tmp[0];
    }
    else
    {
        for (size_t i = 0; i < chunks; ++i)
        {
            res.values.insert(res.values.end(), tmp[i].values.begin(), tmp[i].values.end());
            if (is_sparse)
            {
                res.indices.insert(res.indices.end(), tmp[i].indices.begin(), tmp[i].indices.end());
            }
        }
        return res;
    }
}

} // libsnark
#endif // MULTIEXP_TCC_
