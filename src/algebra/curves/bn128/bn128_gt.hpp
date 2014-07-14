/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef BN128_GT_HPP_
#define BN128_GT_HPP_
#include "algebra/fields/fp.hpp"
#include <iostream>
#include "bn.h"

namespace libsnark {

class bn128_GT;
std::ostream& operator<<(std::ostream &, const bn128_GT&);
std::istream& operator>>(std::istream &, bn128_GT&);

class bn128_GT {
public:
    static bn128_GT GT_one;
    bn::Fp12 elem;

    bn128_GT();
    bool operator==(const bn128_GT &other) const;
    bool operator!=(const bn128_GT &other) const;

    bn128_GT operator*(const bn128_GT &other) const;
    bn128_GT unitary_inverse() const;

    static bn128_GT one();

    void print() { std::cout << this->elem << "\n"; };

    friend std::ostream& operator<<(std::ostream &out, const bn128_GT &g);
    friend std::istream& operator>>(std::istream &in, bn128_GT &g);
};

template<mp_size_t m>
bn128_GT operator^(const bn128_GT &rhs, const bigint<m> &lhs)
{
    bn128_GT res = bn128_GT::one();

    bool found_one = false;
    for (long i = m-1; i >= 0; --i)
    {
        for (long j = GMP_NUMB_BITS - 1; j >= 0; --j)
        {
            if (found_one)
            {
                res = res * res;
            }

            if (lhs.data[i] & (1ul<<j))
            {
                found_one = true;
                res = res * rhs;
            }
        }
    }

    return res;
}


template<mp_size_t m, const bigint<m>& modulus_p>
bn128_GT operator^(const bn128_GT &rhs, const Fp_model<m,modulus_p> &lhs)
{
    return rhs ^ lhs.as_bigint();
}

} // libsnark
#endif // BN128_GT_HPP_
