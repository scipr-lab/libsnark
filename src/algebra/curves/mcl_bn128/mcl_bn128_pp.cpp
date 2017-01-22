/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/mcl_bn128/mcl_bn128_pp.hpp"
#include "common/profiling.hpp"

namespace libsnark {

void mcl_bn128_pp::init_public_params()
{
    init_mcl_bn128_params();
}

mcl_bn128_GT mcl_bn128_pp::final_exponentiation(const mcl_bn128_GT &elt)
{
    return mcl_bn128_final_exponentiation(elt);
}

mcl_bn128_ate_G1_precomp mcl_bn128_pp::precompute_G1(const mcl_bn128_G1 &P)
{
    return mcl_bn128_ate_precompute_G1(P);
}

mcl_bn128_ate_G2_precomp mcl_bn128_pp::precompute_G2(const mcl_bn128_G2 &Q)
{
    return mcl_bn128_ate_precompute_G2(Q);
}

mcl_bn128_Fq12 mcl_bn128_pp::miller_loop(const mcl_bn128_ate_G1_precomp &prec_P,
                                 const mcl_bn128_ate_G2_precomp &prec_Q)
{
    enter_block("Call to miller_loop<mcl_bn128_pp>");
    mcl_bn128_Fq12 result = mcl_bn128_ate_miller_loop(prec_P, prec_Q);
    leave_block("Call to miller_loop<mcl_bn128_pp>");
    return result;
}

mcl_bn128_Fq12 mcl_bn128_pp::double_miller_loop(const mcl_bn128_ate_G1_precomp &prec_P1,
                                        const mcl_bn128_ate_G2_precomp &prec_Q1,
                                        const mcl_bn128_ate_G1_precomp &prec_P2,
                                        const mcl_bn128_ate_G2_precomp &prec_Q2)
{
    enter_block("Call to double_miller_loop<mcl_bn128_pp>");
    mcl_bn128_Fq12 result = mcl_bn128_double_ate_miller_loop(prec_P1, prec_Q1, prec_P2, prec_Q2);
    leave_block("Call to double_miller_loop<mcl_bn128_pp>");
    return result;
}

mcl_bn128_Fq12 mcl_bn128_pp::pairing(const mcl_bn128_G1 &P,
                             const mcl_bn128_G2 &Q)
{
    enter_block("Call to pairing<mcl_bn128_pp>");
    mcl_bn128_ate_G1_precomp prec_P = mcl_bn128_pp::precompute_G1(P);
    mcl_bn128_ate_G2_precomp prec_Q = mcl_bn128_pp::precompute_G2(Q);

    mcl_bn128_Fq12 result = mcl_bn128_pp::miller_loop(prec_P, prec_Q);
    leave_block("Call to pairing<mcl_bn128_pp>");
    return result;
}

mcl_bn128_GT mcl_bn128_pp::reduced_pairing(const mcl_bn128_G1 &P,
                                   const mcl_bn128_G2 &Q)
{
    enter_block("Call to reduced_pairing<mcl_bn128_pp>");
    const mcl_bn128_Fq12 f = mcl_bn128_pp::pairing(P, Q);
    const mcl_bn128_GT result = mcl_bn128_pp::final_exponentiation(f);
    leave_block("Call to reduced_pairing<mcl_bn128_pp>");
    return result;
}

} // libsnark
