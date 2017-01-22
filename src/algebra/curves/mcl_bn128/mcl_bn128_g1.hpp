/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MCL_BN128_G1_HPP_
#define MCL_BN128_G1_HPP_
#include <vector>
#include "algebra/curves/mcl_bn128/mcl_bn128_init.hpp"
#include "algebra/curves/curve_utils.hpp"
#include "mcl/bn256.hpp"

namespace libsnark {

class mcl_bn128_G1;
std::ostream& operator<<(std::ostream &, const mcl_bn128_G1&);
std::istream& operator>>(std::istream &, mcl_bn128_G1&);

class mcl_bn128_G1 {
private:
    typedef mcl::bn256::Fp Fp;
    typedef mcl::bn256::G1 G1;
    static Fp sqrt(const Fp &el);
public:
#ifdef PROFILE_OP_COUNTS
    static long long add_cnt;
    static long long dbl_cnt;
#endif
    static std::vector<size_t> wnaf_window_table;
    static std::vector<size_t> fixed_base_exp_window_table;
    static mcl_bn128_G1 G1_zero;
    static mcl_bn128_G1 G1_one;

    mcl::bn256::G1 pt;
    mcl_bn128_G1();
    typedef mcl_bn128_Fq base_field;
    typedef mcl_bn128_Fr scalar_field;

    void print() const;
    void print_coordinates() const;

    void to_affine_coordinates();
    void to_special();
    bool is_special() const;

    bool is_zero() const;

    bool operator==(const mcl_bn128_G1 &other) const;
    bool operator!=(const mcl_bn128_G1 &other) const;

    mcl_bn128_G1 operator+(const mcl_bn128_G1 &other) const;
    mcl_bn128_G1 operator-() const;
    mcl_bn128_G1 operator-(const mcl_bn128_G1 &other) const;

    mcl_bn128_G1 add(const mcl_bn128_G1 &other) const;
    mcl_bn128_G1 mixed_add(const mcl_bn128_G1 &other) const;
    mcl_bn128_G1 dbl() const;

    bool is_well_formed() const;

    static mcl_bn128_G1 zero();
    static mcl_bn128_G1 one();
    static mcl_bn128_G1 random_element();

    static size_t size_in_bits() { return mcl_bn128_Fq::size_in_bits() + 1; }
    static bigint<base_field::num_limbs> base_field_char() { return base_field::field_char(); }
    static bigint<scalar_field::num_limbs> order() { return scalar_field::field_char(); }

    friend std::ostream& operator<<(std::ostream &out, const mcl_bn128_G1 &g);
    friend std::istream& operator>>(std::istream &in, mcl_bn128_G1 &g);
};

template<mp_size_t m>
mcl_bn128_G1 operator*(const bigint<m> &lhs, const mcl_bn128_G1 &rhs)
{
    return scalar_mul<mcl_bn128_G1, m>(rhs, lhs);
}

template<mp_size_t m, const bigint<m>& modulus_p>
mcl_bn128_G1 operator*(const Fp_model<m,modulus_p> &lhs, const mcl_bn128_G1 &rhs)
{
    return scalar_mul<mcl_bn128_G1, m>(rhs, lhs.as_bigint());
}

std::ostream& operator<<(std::ostream& out, const std::vector<mcl_bn128_G1> &v);
std::istream& operator>>(std::istream& in, std::vector<mcl_bn128_G1> &v);

template<typename T>
void batch_to_special_all_non_zeros(std::vector<T> &vec);
template<>
void batch_to_special_all_non_zeros<mcl_bn128_G1>(std::vector<mcl_bn128_G1> &vec);

} // libsnark
#endif // MCL_BN128_G1_HPP_
