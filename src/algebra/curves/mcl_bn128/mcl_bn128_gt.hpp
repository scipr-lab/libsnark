/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MCL_BN128_GT_HPP_
#define MCL_BN128_GT_HPP_
#include "algebra/fields/fp.hpp"
#include "algebra/fields/field_utils.hpp"
#include <iostream>
#include "mcl/bn256.hpp"

namespace libsnark {

class mcl_bn128_GT;
std::ostream& operator<<(std::ostream &, const mcl_bn128_GT&);
std::istream& operator>>(std::istream &, mcl_bn128_GT&);

class mcl_bn128_GT {
public:
    static mcl_bn128_GT GT_one;
    mcl::bn256::Fp12 elem;

    mcl_bn128_GT();
    bool operator==(const mcl_bn128_GT &other) const;
    bool operator!=(const mcl_bn128_GT &other) const;

    mcl_bn128_GT operator*(const mcl_bn128_GT &other) const;
    mcl_bn128_GT unitary_inverse() const;

    static mcl_bn128_GT one();

    void print() { std::cout << this->elem << "\n"; };

    friend std::ostream& operator<<(std::ostream &out, const mcl_bn128_GT &g);
    friend std::istream& operator>>(std::istream &in, mcl_bn128_GT &g);
};

template<mp_size_t m>
mcl_bn128_GT operator^(const mcl_bn128_GT &rhs, const bigint<m> &lhs)
{
    return power<mcl_bn128_GT, m>(rhs, lhs);
}


template<mp_size_t m, const bigint<m>& modulus_p>
mcl_bn128_GT operator^(const mcl_bn128_GT &rhs, const Fp_model<m,modulus_p> &lhs)
{
    return power<mcl_bn128_GT, m>(rhs, lhs.as_bigint());
}

} // libsnark
#endif // MCL_BN128_GT_HPP_
