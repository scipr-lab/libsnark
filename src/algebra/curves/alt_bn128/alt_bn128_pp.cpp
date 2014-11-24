/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/alt_bn128/alt_bn128_pp.hpp"

namespace libsnark {

template<>
void init_public_params<alt_bn128_pp>()
{
    init_alt_bn128_params();
}

template<>
alt_bn128_GT final_exponentiation<alt_bn128_pp>(const alt_bn128_Fq12 &elt)
{
    return alt_bn128_final_exponentiation(elt);
}

template<>
alt_bn128_G1_precomp precompute_G1<alt_bn128_pp>(const alt_bn128_G1 &P)
{
    return alt_bn128_precompute_G1(P);
}

template<>
alt_bn128_G2_precomp precompute_G2<alt_bn128_pp>(const alt_bn128_G2 &Q)
{
    return alt_bn128_precompute_G2(Q);
}


template<>
alt_bn128_Fq12 miller_loop<alt_bn128_pp>(const alt_bn128_G1_precomp &prec_P,
                                         const alt_bn128_G2_precomp &prec_Q)
{
    return alt_bn128_miller_loop(prec_P, prec_Q);
}

template<>
alt_bn128_Fq12 double_miller_loop<alt_bn128_pp>(const alt_bn128_G1_precomp &prec_P1,
                                                const alt_bn128_G2_precomp &prec_Q1,
                                                const alt_bn128_G1_precomp &prec_P2,
                                                const alt_bn128_G2_precomp &prec_Q2)
{
    return alt_bn128_double_miller_loop(prec_P1, prec_Q1, prec_P2, prec_Q2);
}

template<>
alt_bn128_Fq12 pairing<alt_bn128_pp>(const alt_bn128_G1 &P,
                                     const alt_bn128_G2 &Q)
{
    return alt_bn128_pairing(P, Q);
}

template<>
alt_bn128_Fq12 reduced_pairing<alt_bn128_pp>(const alt_bn128_G1 &P,
                                             const alt_bn128_G2 &Q)
{
    return alt_bn128_reduced_pairing(P, Q);
}

} // libsnark
