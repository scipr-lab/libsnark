/** @file
 *****************************************************************************

 This file defines the default PCD cycle.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_PPZKPCD_PP_HPP_
#define R1CS_PPZKPCD_PP_HPP_

/*********************** Define default PCD cycle ***************************/

#include <libff/algebra/curves/mnt/mnt4/mnt4_pp.hpp>
#include <libff/algebra/curves/mnt/mnt6/mnt6_pp.hpp>

namespace libsnark {

class default_r1cs_ppzkpcd_pp {
public:
    typedef libff::mnt4_pp curve_A_pp;
    typedef libff::mnt6_pp curve_B_pp;

    typedef libff::Fr<curve_A_pp> scalar_field_A;
    typedef libff::Fr<curve_B_pp> scalar_field_B;

    static void init_public_params();
};

} // libsnark

#endif // R1CS_PPZKPCD_PP_HPP_
