/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef ALT_BN128_PP_HPP_
#define ALT_BN128_PP_HPP_
#include "algebra/curves/public_params.hpp"
#include "algebra/curves/alt_bn128/alt_bn128_init.hpp"
#include "algebra/curves/alt_bn128/alt_bn128_g1.hpp"
#include "algebra/curves/alt_bn128/alt_bn128_g2.hpp"
#include "algebra/curves/alt_bn128/alt_bn128_pairing.hpp"

namespace libsnark {

class alt_bn128_pp {
};

template<>
class pp_selector<alt_bn128_pp> {
public:
    typedef alt_bn128_Fr Fp_type;
    typedef alt_bn128_G1 G1_type;
    typedef alt_bn128_G2 G2_type;
    typedef alt_bn128_G1_precomp G1_precomp_type;
    typedef alt_bn128_G2_precomp G2_precomp_type;
    typedef alt_bn128_Fq Fq_type;
    typedef alt_bn128_Fq2 Fqe_type;
    typedef alt_bn128_Fq12 Fqk_type;
    typedef alt_bn128_GT GT_type;
};

template<>
void init_public_params<alt_bn128_pp>();

template<>
alt_bn128_GT final_exponentiation<alt_bn128_pp>(const alt_bn128_Fq12 &elt);

template<>
alt_bn128_G1_precomp precompute_G1<alt_bn128_pp>(const alt_bn128_G1 &P);

template<>
alt_bn128_G2_precomp precompute_G2<alt_bn128_pp>(const alt_bn128_G2 &Q);

template<>
alt_bn128_Fq12 miller_loop<alt_bn128_pp>(const alt_bn128_G1_precomp &prec_P,
                              const alt_bn128_G2_precomp &prec_Q);

template<>
alt_bn128_Fq12 double_miller_loop<alt_bn128_pp>(const alt_bn128_G1_precomp &prec_P1,
                                     const alt_bn128_G2_precomp &prec_Q1,
                                     const alt_bn128_G1_precomp &prec_P2,
                                     const alt_bn128_G2_precomp &prec_Q2);

/* the following are used in test files */

template<>
alt_bn128_Fq12 pairing<alt_bn128_pp>(const alt_bn128_G1 &P,
                          const alt_bn128_G2 &Q);

template<>
alt_bn128_Fq12 reduced_pairing<alt_bn128_pp>(const alt_bn128_G1 &P,
                                  const alt_bn128_G2 &Q);

} // libsnark
#endif // ALT_BN128_PP_HPP_
