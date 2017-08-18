/** @file
 ********************************************************************************
 Declares functions for computing Ate pairings over the mcl_bn128 curves, split into a
 offline and online stages.
 ********************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef MCL_BN128_PAIRING_HPP_
#define MCL_BN128_PAIRING_HPP_
#include "algebra/curves/mcl_bn128/mcl_bn128_g1.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_g2.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_gt.hpp"
#include "mcl/bn256.hpp"

namespace libsnark {

struct mcl_bn128_ate_G1_precomp {
    mcl::bn256::G1 P;

    bool operator==(const mcl_bn128_ate_G1_precomp &other) const;
    friend std::ostream& operator<<(std::ostream &out, const mcl_bn128_ate_G1_precomp &prec_P);
    friend std::istream& operator>>(std::istream &in, mcl_bn128_ate_G1_precomp &prec_P);
};

typedef mcl::bn256::Fp6 mcl_bn128_ate_ell_coeffs;

struct mcl_bn128_ate_G2_precomp {
    mcl::bn256::G2 Q;
    std::vector<mcl_bn128_ate_ell_coeffs> coeffs;

    bool operator==(const mcl_bn128_ate_G2_precomp &other) const;
    friend std::ostream& operator<<(std::ostream &out, const mcl_bn128_ate_G2_precomp &prec_Q);
    friend std::istream& operator>>(std::istream &in, mcl_bn128_ate_G2_precomp &prec_Q);
};

mcl_bn128_ate_G1_precomp mcl_bn128_ate_precompute_G1(const mcl_bn128_G1& P);
mcl_bn128_ate_G2_precomp mcl_bn128_ate_precompute_G2(const mcl_bn128_G2& Q);

mcl_bn128_Fq12 mcl_bn128_double_ate_miller_loop(const mcl_bn128_ate_G1_precomp &prec_P1,
                                        const mcl_bn128_ate_G2_precomp &prec_Q1,
                                        const mcl_bn128_ate_G1_precomp &prec_P2,
                                        const mcl_bn128_ate_G2_precomp &prec_Q2);
mcl_bn128_Fq12 mcl_bn128_ate_miller_loop(const mcl_bn128_ate_G1_precomp &prec_P,
                                 const mcl_bn128_ate_G2_precomp &prec_Q);

mcl_bn128_GT mcl_bn128_final_exponentiation(const mcl_bn128_Fq12 &elt);

} // libsnark
#endif // MCL_BN128_PAIRING_HPP_
