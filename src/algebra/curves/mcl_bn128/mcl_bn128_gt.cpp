/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/mcl_bn128/mcl_bn128_gt.hpp"

using namespace mcl::bn256;

namespace libsnark {

mcl_bn128_GT mcl_bn128_GT::GT_one;
mcl_bn128_GT::mcl_bn128_GT()
{
    this->elem.clear();
}

bool mcl_bn128_GT::operator==(const mcl_bn128_GT &other) const
{
    return (this->elem == other.elem);
}

bool mcl_bn128_GT::operator!=(const mcl_bn128_GT& other) const
{
    return !(operator==(other));
}

mcl_bn128_GT mcl_bn128_GT::operator*(const mcl_bn128_GT &other) const
{
    mcl_bn128_GT result;
    Fp12::mul(result.elem, this->elem, other.elem);
    return result;
}

mcl_bn128_GT mcl_bn128_GT::unitary_inverse() const
{
    mcl_bn128_GT result(*this);
    Fp6::neg(result.elem.b, result.elem.b);
    return result;
}

mcl_bn128_GT mcl_bn128_GT::one()
{
    return GT_one;
}

std::ostream& operator<<(std::ostream &out, const mcl_bn128_GT &g)
{
#ifndef BINARY_OUTPUT
    out << g.elem.a << OUTPUT_SEPARATOR << g.elem.b;
#else
    out.write((char*) &g.elem, sizeof(g.elem));
#endif
    return out;
}

std::istream& operator>>(std::istream &in, mcl_bn128_GT &g)
{
#ifndef BINARY_OUTPUT
    in >> g.elem.a;
    consume_OUTPUT_SEPARATOR(in);
    in >> g.elem.b;
#else
    in.read((char*) &g.elem, sizeof(g.elem));
#endif
    return in;
}
} // libsnark
