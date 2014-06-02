/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#include "algebra/curves/bn128/bn128_gt.hpp"

namespace libsnark {

bn128_GT bn128_GT::GT_one;
bn128_GT::bn128_GT()
{
    this->elem.clear();
}

bool bn128_GT::operator==(const bn128_GT &other) const
{
    return (this->elem == other.elem);
}

bool bn128_GT::operator!=(const bn128_GT& other) const
{
    return !(operator==(other));
}

bn128_GT bn128_GT::operator*(const bn128_GT &other) const
{
    bn128_GT result;
    bn::Fp12::mul(result.elem, this->elem, other.elem);
    return result;
}

bn128_GT bn128_GT::unitary_inverse() const
{
    bn128_GT result(*this);
    bn::Fp6::neg(result.elem.b_, result.elem.b_);
    return result;
}

bn128_GT bn128_GT::one()
{
    return GT_one;
}

std::ostream& operator<<(std::ostream &out, const bn128_GT &g)
{
    return out;
}

std::istream& operator>>(std::istream &in, bn128_GT &g)
{
    return in;
}
} // libsnark
