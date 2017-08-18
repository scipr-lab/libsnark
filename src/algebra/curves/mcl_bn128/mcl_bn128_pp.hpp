/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MCL_BN128_PP_HPP_
#define MCL_BN128_PP_HPP_
#include "algebra/curves/public_params.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_init.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_g1.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_g2.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_gt.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_pairing.hpp"

namespace libsnark {

class mcl_bn128_pp {
public:
    typedef mcl_bn128_Fr Fp_type;
    typedef mcl_bn128_G1 G1_type;
    typedef mcl_bn128_G2 G2_type;
    typedef mcl_bn128_ate_G1_precomp G1_precomp_type;
    typedef mcl_bn128_ate_G2_precomp G2_precomp_type;
    typedef mcl_bn128_Fq Fq_type;
    typedef mcl_bn128_Fq12 Fqk_type;
    typedef mcl_bn128_GT GT_type;

    static const bool has_affine_pairing = false;

    static void init_public_params();
    static mcl_bn128_GT final_exponentiation(const mcl_bn128_Fq12 &elt);
    static mcl_bn128_ate_G1_precomp precompute_G1(const mcl_bn128_G1 &P);
    static mcl_bn128_ate_G2_precomp precompute_G2(const mcl_bn128_G2 &Q);
    static mcl_bn128_Fq12 miller_loop(const mcl_bn128_ate_G1_precomp &prec_P,
                                  const mcl_bn128_ate_G2_precomp &prec_Q);
    static mcl_bn128_Fq12 double_miller_loop(const mcl_bn128_ate_G1_precomp &prec_P1,
                                         const mcl_bn128_ate_G2_precomp &prec_Q1,
                                         const mcl_bn128_ate_G1_precomp &prec_P2,
                                         const mcl_bn128_ate_G2_precomp &prec_Q2);

    /* the following are used in test files */
    static mcl_bn128_GT pairing(const mcl_bn128_G1 &P,
                            const mcl_bn128_G2 &Q);
    static mcl_bn128_GT reduced_pairing(const mcl_bn128_G1 &P,
                                    const mcl_bn128_G2 &Q);
};

} // libsnark
#endif // MCL_BN128_PP_HPP_
