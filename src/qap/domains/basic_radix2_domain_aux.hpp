/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef BASIC_RADIX2_DOMAIN_AUX_HPP_
#define BASIC_RADIX2_DOMAIN_AUX_HPP_

template<typename FieldT>
void _basic_radix2_FFT(std::vector<FieldT> &a, const FieldT &omega);

template<typename FieldT>
void _parallel_basic_radix2_FFT(std::vector<FieldT> &a, const FieldT &omega);

template<typename FieldT>
void _multiply_by_coset(std::vector<FieldT> &a, const FieldT &g);

template<typename FieldT>
std::vector<FieldT> _basic_radix2_lagrange_coeffs(const size_t m, const FieldT &t);

#include "qap/domains/basic_radix2_domain_aux.tcc"

namespace libsnark {

} // libsnark
#endif // BASIC_RADIX2_DOMAIN_AUX_HPP_
