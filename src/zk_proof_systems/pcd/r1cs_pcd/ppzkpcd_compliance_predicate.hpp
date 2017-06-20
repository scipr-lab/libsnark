/** @file
 *****************************************************************************

 Template aliasing for prettifying R1CS PCD interfaces.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef PPZKPCD_COMPLIANCE_PREDICATE_HPP_
#define PPZKPCD_COMPLIANCE_PREDICATE_HPP_

#include <libff/algebra/curves/public_params.hpp>
#include "compliance_predicate/compliance_predicate.hpp"

namespace libsnark {

/* template aliasing for R1CS (multi-predicate) ppzkPCD: */

template<typename PCD_ppT>
using r1cs_mp_ppzkpcd_compliance_predicate = r1cs_pcd_compliance_predicate<libff::Fr<typename PCD_ppT::curve_A_pp> >;

template<typename PCD_ppT>
using r1cs_mp_ppzkpcd_message = r1cs_pcd_message<libff::Fr<typename PCD_ppT::curve_A_pp> >;

template<typename PCD_ppT>
using r1cs_mp_ppzkpcd_local_data = r1cs_pcd_local_data<libff::Fr<typename PCD_ppT::curve_A_pp> >;

template<typename PCD_ppT>
using r1cs_mp_ppzkpcd_variable_assignment = r1cs_variable_assignment<libff::Fr<typename PCD_ppT::curve_A_pp> >;

}

#endif // PPZKPCD_COMPLIANCE_PREDICATE_HPP_

