/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef WNAF_HPP_
#define WNAF_HPP_

#include "encoding/knowledge_commitment.hpp"

namespace libsnark {

/*
  Weighted non-adjacent form exponentiation.
*/

template<mp_size_t n>
std::vector<long> find_wNAF(const size_t w, const bigint<n> &exp);

template<typename T, mp_size_t n>
T fixed_window_wnaf_exp(const size_t window, const T &neutral, const T &base, const bigint<n> &scalar);

template<typename T, mp_size_t n>
T opt_window_wnaf_exp(const T &neutral, const T &base, const bigint<n> &scalar, const size_t scalar_bits);

} // libsnark

#include "common/wnaf.tcc"
#endif // WNAF_HPP_
