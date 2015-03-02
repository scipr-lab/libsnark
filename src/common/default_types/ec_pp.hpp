/** @file
 *****************************************************************************

 This file defines default_ec_pp based on the CURVE=... make flag, which selects
 which elliptic curve is used to implement group arithmetic and pairings.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef EC_PP_HPP_
#define EC_PP_HPP_

/************************ Pick the elliptic curve ****************************/

#ifdef CURVE_ALT_BN128
#include "algebra/curves/alt_bn128/alt_bn128_pp.hpp"
namespace libsnark {
typedef alt_bn128_pp default_ec_pp;
} // libsnark
#endif

#ifdef CURVE_BN128
#include "algebra/curves/bn128/bn128_pp.hpp"
namespace libsnark {
typedef bn128_pp default_ec_pp;
} // libsnark
#endif

#ifdef CURVE_EDWARDS
#include "algebra/curves/edwards/edwards_pp.hpp"
namespace libsnark {
typedef edwards_pp default_ec_pp;
} // libsnark
#endif

#endif // EC_PP_HPP_
