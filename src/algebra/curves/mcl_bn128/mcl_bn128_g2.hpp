/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MCL_BN128_G2_HPP_
#define MCL_BN128_G2_HPP_
#include <iostream>
#include <vector>
#include "algebra/curves/mcl_bn128/mcl_bn128_init.hpp"
#include "algebra/curves/curve_utils.hpp"
#include "mcl/bn256.hpp"

namespace libsnark {

class mcl_bn128_G2;
std::ostream& operator<<(std::ostream &, const mcl_bn128_G2&);
std::istream& operator>>(std::istream &, mcl_bn128_G2&);

class mcl_bn128_G2 {
private:
    typedef mcl::bn256::Fp2 Fp2;
    typedef mcl::bn256::G2 G2;
    static Fp2 sqrt(const Fp2 &el);
public:
#ifdef PROFILE_OP_COUNTS
    static long long add_cnt;
    static long long dbl_cnt;
#endif
    static std::vector<size_t> wnaf_window_table;
    static std::vector<size_t> fixed_base_exp_window_table;
    static mcl_bn128_G2 G2_zero;
    static mcl_bn128_G2 G2_one;

    mcl::bn256::G2 pt;
    mcl_bn128_G2();
    typedef mcl_bn128_Fq base_field;
    typedef mcl_bn128_Fr scalar_field;

    void print() const;
    void print_coordinates() const;

    void to_affine_coordinates();
    void to_special();
    bool is_special() const;

    bool is_zero() const;

    bool operator==(const mcl_bn128_G2 &other) const;
    bool operator!=(const mcl_bn128_G2 &other) const;

    mcl_bn128_G2 operator+(const mcl_bn128_G2 &other) const;
    mcl_bn128_G2 operator-() const;
    mcl_bn128_G2 operator-(const mcl_bn128_G2 &other) const;

    mcl_bn128_G2 add(const mcl_bn128_G2 &other) const;
    mcl_bn128_G2 mixed_add(const mcl_bn128_G2 &other) const;
    mcl_bn128_G2 dbl() const;

    bool is_well_formed() const;

    static mcl_bn128_G2 zero();
    static mcl_bn128_G2 one();
    static mcl_bn128_G2 random_element();

    static size_t size_in_bits() { return 2*base_field::size_in_bits() + 1; }
    static bigint<base_field::num_limbs> base_field_char() { return base_field::field_char(); }
    static bigint<scalar_field::num_limbs> order() { return scalar_field::field_char(); }

    friend std::ostream& operator<<(std::ostream &out, const mcl_bn128_G2 &g);
    friend std::istream& operator>>(std::istream &in, mcl_bn128_G2 &g);
};

template<mp_size_t m>
mcl_bn128_G2 operator*(const bigint<m> &lhs, const mcl_bn128_G2 &rhs)
{
    return scalar_mul<mcl_bn128_G2, m>(rhs, lhs);
}

template<mp_size_t m, const bigint<m>& modulus_p>
mcl_bn128_G2 operator*(const Fp_model<m, modulus_p> &lhs, const mcl_bn128_G2 &rhs)
{
    return scalar_mul<mcl_bn128_G2, m>(rhs, lhs.as_bigint());
}

template<typename T>
void batch_to_special_all_non_zeros(std::vector<T> &vec);
template<>
void batch_to_special_all_non_zeros<mcl_bn128_G2>(std::vector<mcl_bn128_G2> &vec);

} // libsnark
#endif // MCL_BN128_G2_HPP_
