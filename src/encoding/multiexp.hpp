/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MULTIEXP_HPP_
#define MULTIEXP_HPP_

#include "common/wnaf.hpp"
#include "encoding/knowledge_commitment.hpp"

namespace libsnark {

/*
 Multi-exponentiation algorithms.
*/

template<typename T1, typename T2, mp_size_t n>
knowledge_commitment<T1,T2> opt_window_wnaf_exp(const knowledge_commitment<T1,T2> &neutral,
                                                const knowledge_commitment<T1,T2> &base,
                                                const bigint<n> &scalar, const size_t scalar_bits);

template<typename T, typename FieldT>
T naive_exp(const T &neutral,
            typename std::vector<T>::const_iterator vec_start,
            typename std::vector<T>::const_iterator vec_end,
            typename std::vector<FieldT>::const_iterator scalar_start,
            typename std::vector<FieldT>::const_iterator scalar_end);

template<typename T, typename FieldT>
T multi_exp(const T &neutral,
               typename std::vector<T>::const_iterator vec_start,
               typename std::vector<T>::const_iterator vec_end,
               typename std::vector<FieldT>::const_iterator scalar_start,
               typename std::vector<FieldT>::const_iterator scalar_end,
               const size_t chunks,
               const bool use_multiexp=false);

template<typename T1, typename T2, typename FieldT>
knowledge_commitment<T1, T2> kc_multi_exp_with_fast_add_special(const knowledge_commitment<T1, T2> &neutral,
                                                           const knowledge_commitment_vector<T1, T2> &vec,
                                                           const size_t min_idx,
                                                           const size_t max_idx,
                                                           typename std::vector<FieldT>::const_iterator scalar_start,
                                                           typename std::vector<FieldT>::const_iterator scalar_end,
                                                           const size_t chunks,
                                                           const bool use_multiexp=false);

template<typename T, typename FieldT>
T multi_exp_with_fast_add_special(const T &neutral,
                             typename std::vector<T>::const_iterator vec_start,
                             typename std::vector<T>::const_iterator vec_end,
                             typename std::vector<FieldT>::const_iterator scalar_start,
                             typename std::vector<FieldT>::const_iterator scalar_end,
                             const size_t chunks,
                             const bool use_multiexp);

template<typename T>
using window_table = std::vector<std::vector<T> >;

template<typename T>
size_t get_exp_window_size(const size_t exp_count);

template<typename T>
window_table<T> get_window_table(const size_t exp_size,
                                 const T &neutral,
                                 const size_t window,
                                 const T &g);

template<typename T, typename FieldT>
T windowed_exp(const size_t exp_size,
               const size_t window,
               const window_table<T> &powers_of_g,
               const FieldT &pow);

template<typename T, typename FieldT>
std::vector<T> batch_exp(const size_t exp_size,
                         const size_t window,
                         const window_table<T> &table,
                         const std::vector<FieldT> &v);

// defined in every curve
template<typename T>
void batch_to_special_all_non_zeros(std::vector<T> &vec);

template<typename T>
void batch_to_special(std::vector<T> &vec);

template<typename T1, typename T2>
void kc_batch_to_special(std::vector<knowledge_commitment<T1, T2> > &vec);

template<typename T1, typename T2, typename FieldT>
knowledge_commitment_vector<T1, T2> kc_batch_exp(const size_t exp_size,
                                                 const size_t T1_window,
                                                 const size_t T2_window,
                                                 const window_table<T1> &T1_table,
                                                 const window_table<T2> &T2_table,
                                                 const FieldT &T1_coeff,
                                                 const FieldT &T2_coeff,
                                                 const std::vector<FieldT> &v,
                                                 const bool is_sparse);

} // libsnark
#include "encoding/multiexp.tcc"

#endif // MULTIEXP_HPP_
