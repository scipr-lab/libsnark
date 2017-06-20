/** @file
 *****************************************************************************

 This file defines the default architecture and curve choices for RAM
 ppzk-SNARK.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef TINYRAM_PPZKSNARK_PP_HPP_
#define TINYRAM_PPZKSNARK_PP_HPP_

#include "r1cs_ppzksnark_pp.hpp"
#include "../../relations/ram_computations/rams/tinyram/tinyram_params.hpp"

namespace libsnark {

class default_tinyram_ppzksnark_pp {
public:
    typedef default_r1cs_ppzksnark_pp snark_pp;
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    typedef ram_tinyram<FieldT> machine_pp;

    static void init_public_params();
};

} // libsnark

#endif // TINYRAM_PPZKSNARK_PP_HPP_
