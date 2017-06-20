/** @file
 *****************************************************************************

 Parameters for *single-predicate* ppzkPCD for R1CS.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_SP_PPZKPCD_PARAMS_HPP_
#define R1CS_SP_PPZKPCD_PARAMS_HPP_

#include <libff/algebra/curves/public_params.hpp>
#include "../compliance_predicate/compliance_predicate.hpp"
#include "../r1cs_pcd_params.hpp"

namespace libsnark {

template<typename PCD_ppT>
using r1cs_sp_ppzkpcd_compliance_predicate = r1cs_pcd_compliance_predicate<libff::Fr<typename PCD_ppT::curve_A_pp> >;

template<typename PCD_ppT>
using r1cs_sp_ppzkpcd_message = r1cs_pcd_message<libff::Fr<typename PCD_ppT::curve_A_pp> >;

template<typename PCD_ppT>
using r1cs_sp_ppzkpcd_local_data = r1cs_pcd_local_data<libff::Fr<typename PCD_ppT::curve_A_pp> >;

template<typename PCD_ppT>
using r1cs_sp_ppzkpcd_primary_input = r1cs_pcd_compliance_predicate_primary_input<libff::Fr<typename PCD_ppT::curve_A_pp> >;

template<typename PCD_ppT>
using r1cs_sp_ppzkpcd_auxiliary_input = r1cs_pcd_compliance_predicate_auxiliary_input<libff::Fr<typename PCD_ppT::curve_A_pp> >;

} // libsnark

#endif // R1CS_SP_PPZKPCD_PARAMS_HPP_
