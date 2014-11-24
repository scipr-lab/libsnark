/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/edwards/edwards_pp.hpp"

namespace libsnark {

template<>
void init_public_params<edwards_pp>()
{
    init_edwards_params();
}

template<>
edwards_GT final_exponentiation<edwards_pp>(const edwards_Fq6 &elt)
{
    return edwards_final_exponentiation(elt);
}

template<>
edwards_G1_precomp precompute_G1<edwards_pp>(const edwards_G1 &P)
{
    return edwards_precompute_G1(P);
}

template<>
edwards_G2_precomp precompute_G2<edwards_pp>(const edwards_G2 &Q)
{
    return edwards_precompute_G2(Q);
}


template<>
edwards_Fq6 miller_loop<edwards_pp>(const edwards_G1_precomp &prec_P,
                                    const edwards_G2_precomp &prec_Q)
{
    return edwards_miller_loop(prec_P, prec_Q);
}

template<>
edwards_Fq6 double_miller_loop<edwards_pp>(const edwards_G1_precomp &prec_P1,
                                           const edwards_G2_precomp &prec_Q1,
                                           const edwards_G1_precomp &prec_P2,
                                           const edwards_G2_precomp &prec_Q2)
{
    return edwards_double_miller_loop(prec_P1, prec_Q1, prec_P2, prec_Q2);
}

template<>
edwards_Fq6 pairing<edwards_pp>(const edwards_G1 &P,
                                const edwards_G2 &Q)
{
    return edwards_pairing(P, Q);
}

template<>
edwards_Fq6 reduced_pairing<edwards_pp>(const edwards_G1 &P,
                                        const edwards_G2 &Q)
{
    return edwards_reduced_pairing(P, Q);
}

} // libsnark
