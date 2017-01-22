/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MCL_BN128_INIT_HPP_
#define MCL_BN128_INIT_HPP_
#include "algebra/curves/public_params.hpp"
#include "algebra/fields/fp.hpp"
#include "mcl/bn256.hpp"   // If you're missing this file, run libsnark's ./prepare-depends.sh

namespace libsnark {

const mp_size_t mcl_bn128_r_bitcount = 254;
const mp_size_t mcl_bn128_q_bitcount = 254;

const mp_size_t mcl_bn128_r_limbs = (mcl_bn128_r_bitcount+GMP_NUMB_BITS-1)/GMP_NUMB_BITS;
const mp_size_t mcl_bn128_q_limbs = (mcl_bn128_q_bitcount+GMP_NUMB_BITS-1)/GMP_NUMB_BITS;

extern bigint<mcl_bn128_r_limbs> mcl_bn128_modulus_r;
extern bigint<mcl_bn128_q_limbs> mcl_bn128_modulus_q;

extern mcl::bn256::Fp mcl_bn128_coeff_b;
extern size_t mcl_bn128_Fq_s;
extern mcl::bn256::Fp mcl_bn128_Fq_nqr_to_t;
extern mpz_class mcl_bn128_Fq_t_minus_1_over_2;

extern mcl::bn256::Fp2 mcl_bn128_twist_coeff_b;
extern size_t mcl_bn128_Fq2_s;
extern mcl::bn256::Fp2 mcl_bn128_Fq2_nqr_to_t;
extern mpz_class mcl_bn128_Fq2_t_minus_1_over_2;

typedef Fp_model<mcl_bn128_r_limbs, mcl_bn128_modulus_r> mcl_bn128_Fr;
typedef Fp_model<mcl_bn128_q_limbs, mcl_bn128_modulus_q> mcl_bn128_Fq;

void init_mcl_bn128_params();

class mcl_bn128_G1;
class mcl_bn128_G2;
class mcl_bn128_GT;
typedef mcl_bn128_GT mcl_bn128_Fq12;

} // libsnark
#endif // MCL_BN128_INIT_HPP_
