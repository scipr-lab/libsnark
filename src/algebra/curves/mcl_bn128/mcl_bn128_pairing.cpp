/** @file
 ********************************************************************************
 Implements functions for computing Ate pairings over the mcl_bn128 curves, split into a
 offline and online stages.
 ********************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#include <sstream>

#include "algebra/curves/mcl_bn128/mcl_bn128_pairing.hpp"
#include "common/profiling.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_init.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_g1.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_g2.hpp"
#include "algebra/curves/mcl_bn128/mcl_bn128_gt.hpp"

using namespace mcl::bn256;

namespace libsnark {

bool mcl_bn128_ate_G1_precomp::operator==(const mcl_bn128_ate_G1_precomp &other) const
{
    return this->P == other.P;
}

std::ostream& operator<<(std::ostream &out, const mcl_bn128_ate_G1_precomp &precP)
{
#ifndef BINARY_OUTPUT
    out << precP.P << "\n";
#else
    out.write((char*) &precP.P, sizeof(precP.P));
#endif
    return out;
}

std::istream& operator>>(std::istream &in, mcl_bn128_ate_G1_precomp &precP)
{
#ifndef BINARY_OUTPUT
    in >> precP.P;
    consume_newline(in);
#else
    in.read((char*) &precP.P, sizeof(precP.P));
#endif
    return in;
}

bool mcl_bn128_ate_G2_precomp::operator==(const mcl_bn128_ate_G2_precomp &other) const
{
    if (!(this->Q == other.Q && this->coeffs.size() == other.coeffs.size()))
    {
        return false;
    }

    for (size_t i = 0; i < this->coeffs.size(); ++i)
    {
        if (this->coeffs[i] != other.coeffs[i])
        {
            return false;
        }
    }

    return true;
}

std::ostream& operator<<(std::ostream &out, const mcl_bn128_ate_G2_precomp &precQ)
{
#ifndef BINARY_OUTPUT
    out << precQ.Q << "\n";
#else
    out.write((char*) &precQ.Q, sizeof(precQ.Q));
#endif

    out << precQ.coeffs.size() << "\n";

    for (size_t i = 0; i < precQ.coeffs.size(); ++i)
    {
#ifndef BINARY_OUTPUT
        out << precQ.coeffs[i] << "\n";
#else
        out.write((char*) &precQ.coeffs[i], sizeof(precQ.coeffs[i]));
#endif
    }

    return out;
}

std::istream& operator>>(std::istream &in, mcl_bn128_ate_G2_precomp &precQ)
{
#ifndef BINARY_OUTPUT
    in >> precQ.Q;
    consume_newline(in);
#else
    in.read((char*) &precQ.Q, sizeof(precQ.Q));
#endif

    size_t count;
    in >> count;
    consume_newline(in);
    precQ.coeffs.resize(count);
    for (size_t i = 0; i < count; ++i)
    {
#ifndef BINARY_OUTPUT
        in >> precQ.coeffs[i];
        consume_newline(in);
#else
        in.read((char*) &precQ.coeffs[i], sizeof(precQ.coeffs[i]));
#endif
    }
    return in;
}

mcl_bn128_ate_G1_precomp mcl_bn128_ate_precompute_G1(const mcl_bn128_G1& P)
{
    enter_block("Call to mcl_bn128_ate_precompute_G1");

    mcl_bn128_ate_G1_precomp result;
    result.P = P.pt;
    result.P.normalize();

    leave_block("Call to mcl_bn128_ate_precompute_G1");
    return result;
}

mcl_bn128_ate_G2_precomp mcl_bn128_ate_precompute_G2(const mcl_bn128_G2& Q)
{
    enter_block("Call to mcl_bn128_ate_precompute_G2");

    mcl_bn128_ate_G2_precomp result;
    result.Q = Q.pt;
    mcl::bn256::BN::precomputeG2(result.coeffs, result.Q);

    leave_block("Call to mcl_bn128_ate_precompute_G2");
    return result;
}

mcl_bn128_Fq12 mcl_bn128_ate_miller_loop(const mcl_bn128_ate_G1_precomp &precP,
                                 const mcl_bn128_ate_G2_precomp &precQ)
{
    mcl_bn128_Fq12 f;
    mcl::bn256::BN::precomputedMillerLoop(f.elem, precP.P, precQ.coeffs);
    return f;
}

mcl_bn128_Fq12 mcl_bn128_double_ate_miller_loop(const mcl_bn128_ate_G1_precomp &precP1,
                                        const mcl_bn128_ate_G2_precomp &precQ1,
                                        const mcl_bn128_ate_G1_precomp &precP2,
                                        const mcl_bn128_ate_G2_precomp &precQ2)
{
    mcl_bn128_Fq12 f;
    mcl::bn256::BN::precomputedMillerLoop2(f.elem, precP1.P, precQ1.coeffs, precP2.P, precQ2.coeffs);
    return f;
}

mcl_bn128_GT mcl_bn128_final_exponentiation(const mcl_bn128_Fq12 &elt)
{
    enter_block("Call to mcl_bn128_final_exponentiation");
    mcl_bn128_GT eltcopy;
    mcl::bn256::BN::finalExp(eltcopy.elem, elt.elem);
    leave_block("Call to mcl_bn128_final_exponentiation");
    return eltcopy;
}
} // libsnark
