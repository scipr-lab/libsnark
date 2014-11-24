/** @file
 *****************************************************************************
 Implementation of misc. math and serialization utility functions
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef EXPONENTIATION_TCC_
#define EXPONENTIATION_TCC_

#include "common/utils.hpp"

namespace libsnark {

template<typename T, typename FieldT>
T naive_plain_exp(typename std::vector<T>::const_iterator vec_start,
                  typename std::vector<T>::const_iterator vec_end,
                  typename std::vector<FieldT>::const_iterator scalar_start,
                  typename std::vector<FieldT>::const_iterator scalar_end)
{
    T result(T::zero());

    typename std::vector<T>::const_iterator vec_it;
    typename std::vector<FieldT>::const_iterator scalar_it;

    for (vec_it = vec_start, scalar_it = scalar_start; vec_it != vec_end; ++vec_it, ++scalar_it)
    {
        result = result + (*vec_it) * (*scalar_it);
    }
    assert(scalar_it == scalar_end);

    return result;
}

template<typename FieldT, mp_size_t m>
FieldT power(const FieldT &base, const bigint<m> &exponent)
{
    FieldT result = FieldT::one();

    bool found_one = false;
    for (long i = exponent.max_bits() - 1; i >= 0; --i)
    {
        if (found_one)
        {
            result = result * result;
        }

        if (exponent.test_bit(i))
        {
            found_one = true;
            result = result * base;
        }
    }

    return result;
}

template<typename FieldT>
FieldT power(const FieldT &base, const unsigned long exponent)
{
    return power<FieldT>(base, bigint<1>(exponent));
}

} // libsnark
#endif // EXPONENTIATION_TCC_
