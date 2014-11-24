/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/bn128/bn128_pp.hpp"
#include "common/profiling.hpp"

namespace libsnark {

template<>
void init_public_params<bn128_pp>()
{
    init_bn128_params();
}

template<>
bn128_GT final_exponentiation<bn128_pp>(const bn128_GT &elt)
{
    return bn128_final_exponentiation(elt);
}

template<>
bn128_ate_G1_precomp precompute_G1<bn128_pp>(const bn128_G1 &P)
{
    return bn128_ate_precompute_G1(P);
}

template<>
bn128_ate_G2_precomp precompute_G2<bn128_pp>(const bn128_G2 &Q)
{
    return bn128_ate_precompute_G2(Q);
}


template<>
bn128_Fq12 miller_loop<bn128_pp>(const bn128_ate_G1_precomp &prec_P,
                                 const bn128_ate_G2_precomp &prec_Q)
{
    enter_block("Call to miller_loop<bn128_pp>");
    bn128_Fq12 result = bn128_ate_miller_loop(prec_P, prec_Q);
    leave_block("Call to miller_loop<bn128_pp>");
    return result;
}

template<>
bn128_Fq12 double_miller_loop<bn128_pp>(const bn128_ate_G1_precomp &prec_P1,
                                        const bn128_ate_G2_precomp &prec_Q1,
                                        const bn128_ate_G1_precomp &prec_P2,
                                        const bn128_ate_G2_precomp &prec_Q2)
{
    enter_block("Call to double_miller_loop<bn128_pp>");
    bn128_Fq12 result = bn128_double_ate_miller_loop(prec_P1, prec_Q1, prec_P2, prec_Q2);
    leave_block("Call to double_miller_loop<bn128_pp>");
    return result;
}

template<>
bn128_Fq12 pairing<bn128_pp>(const bn128_G1 &P,
                             const bn128_G2 &Q)
{
    enter_block("Call to pairing<bn128_pp>");
    bn128_ate_G1_precomp prec_P = precompute_G1<bn128_pp>(P);
    bn128_ate_G2_precomp prec_Q = precompute_G2<bn128_pp>(Q);

    bn128_Fq12 result = miller_loop<bn128_pp>(prec_P, prec_Q);
    leave_block("Call to pairing<bn128_pp>");
    return result;
}

template<>
bn128_GT reduced_pairing<bn128_pp>(const bn128_G1 &P,
                                   const bn128_G2 &Q)
{
    enter_block("Call to reduced_pairing<bn128_pp>");
    const bn128_Fq12 f = pairing<bn128_pp>(P, Q);
    const bn128_GT result = final_exponentiation<bn128_pp>(f);
    leave_block("Call to reduced_pairing<bn128_pp>");
    return result;
}

} // libsnark
