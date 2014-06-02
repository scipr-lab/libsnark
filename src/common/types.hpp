/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef TYPES_HPP_
#define TYPES_HPP_

#ifdef CURVE_ALT_BN128
#include "algebra/curves/alt_bn128/alt_bn128_pp.hpp"
namespace libsnark {
typedef alt_bn128_pp default_pp;
} // libsnark
#endif

#ifdef CURVE_BN128
#include "algebra/curves/bn128/bn128_pp.hpp"
namespace libsnark {
typedef bn128_pp default_pp;
} // libsnark
#endif

#ifdef CURVE_EDWARDS
#include "algebra/curves/edwards/edwards_pp.hpp"
namespace libsnark {
typedef edwards_pp default_pp;
} // libsnark
#endif

#endif // TYPES_HPP_
