/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/mcl_bn128/mcl_bn128_g1.hpp"
#include "algebra/curves/mcl_bn128/bn_utils.hpp"

using namespace mcl::bn256;

namespace libsnark {

static struct LibInit {
    LibInit()
    {
        mcl::bn256::bn256init(mcl::bn::CurveSNARK1); // init mcl library
    }
} s_libInit;

#ifdef PROFILE_OP_COUNTS
long long mcl_bn128_G1::add_cnt = 0;
long long mcl_bn128_G1::dbl_cnt = 0;
#endif

std::vector<size_t> mcl_bn128_G1::wnaf_window_table;
std::vector<size_t> mcl_bn128_G1::fixed_base_exp_window_table;
mcl_bn128_G1 mcl_bn128_G1::G1_zero;
mcl_bn128_G1 mcl_bn128_G1::G1_one;

Fp mcl_bn128_G1::sqrt(const Fp &el)
{
    size_t v = mcl_bn128_Fq_s;
    Fp z = mcl_bn128_Fq_nqr_to_t;
    Fp w; Fp::pow(w, el, mcl_bn128_Fq_t_minus_1_over_2);
    Fp x = el * w;
    Fp b = x * w;

#if DEBUG
    // check if square with Euler's criterion
    Fp check = b;
    for (size_t i = 0; i < v-1; ++i)
    {
        Fp::sqr(check, check);
    }

    assert(check == Fp(1));
#endif

    // compute square root with Tonelli--Shanks
    // (does not terminate if not a square!)

    while (b != Fp(1))
    {
        size_t m = 0;
        Fp b2m = b;
        while (b2m != Fp(1))
        {
            // invariant: b2m = b^(2^m) after entering this loop
            Fp::sqr(b2m, b2m);
            m += 1;
        }

        int j = v-m-1;
        w = z;
        while (j > 0)
        {
            Fp::sqr(w, w);
            --j;
        } // w = z^2^(v-m-1)

        z = w * w;
        b = b * z;
        x = x * w;
        v = m;
    }

    return x;
}

mcl_bn128_G1::mcl_bn128_G1()
{
    this->pt.clear();
}

void mcl_bn128_G1::print() const
{
    if (this->is_zero())
    {
        printf("O\n");
    }
    else
    {
        mcl_bn128_G1 copy(*this);
        copy.to_affine_coordinates();
        std::cout << "(" << copy.pt.x.getStr(10) << " : " << copy.pt.y.getStr(10) << " : " << copy.pt.z.getStr(10) << ")\n";
    }
}

void mcl_bn128_G1::print_coordinates() const
{
    if (this->is_zero())
    {
        printf("O\n");
    }
    else
    {
        std::cout << "(" << pt.x.getStr(10) << " : " << pt.y.getStr(10) << " : " << pt.z.getStr(10) << ")\n";
    }
}

void mcl_bn128_G1::to_affine_coordinates()
{
    pt.normalize();
}

void mcl_bn128_G1::to_special()
{
    this->to_affine_coordinates();
}

bool mcl_bn128_G1::is_special() const
{
    return (this->is_zero() || this->pt.z == 1);
}

bool mcl_bn128_G1::is_zero() const
{
    return pt.isZero();
}

bool mcl_bn128_G1::operator==(const mcl_bn128_G1 &other) const
{
    return this->pt == other.pt;
}

bool mcl_bn128_G1::operator!=(const mcl_bn128_G1& other) const
{
    return !(operator==(other));
}

mcl_bn128_G1 mcl_bn128_G1::operator+(const mcl_bn128_G1 &other) const
{
    // handle special cases having to do with O
    if (this->is_zero())
    {
        return other;
    }

    if (other.is_zero())
    {
        return *this;
    }

    // no need to handle points of order 2,4
    // (they cannot exist in a prime-order subgroup)

    // handle double case, and then all the rest
    if (this->operator==(other))
    {
        return this->dbl();
    }
    else
    {
        return this->add(other);
    }
}

mcl_bn128_G1 mcl_bn128_G1::operator-() const
{
    mcl_bn128_G1 result;
    G1::neg(result.pt, pt);
    return result;
}

mcl_bn128_G1 mcl_bn128_G1::operator-(const mcl_bn128_G1 &other) const
{
    return (*this) + (-other);
}

mcl_bn128_G1 mcl_bn128_G1::add(const mcl_bn128_G1 &other) const
{
#ifdef PROFILE_OP_COUNTS
    this->add_cnt++;
#endif
    mcl_bn128_G1 result;
    G1::add(result.pt, pt, other.pt);
    return result;
}

mcl_bn128_G1 mcl_bn128_G1::mixed_add(const mcl_bn128_G1 &other) const
{
    mcl_bn128_G1 result;
    G1::add(result.pt, pt, other.pt);
#ifdef PROFILE_OP_COUNTS
    this->add_cnt++;
#endif
    return result;
}

mcl_bn128_G1 mcl_bn128_G1::dbl() const
{
#ifdef PROFILE_OP_COUNTS
    this->dbl_cnt++;
#endif

    mcl_bn128_G1 result;
    G1::dbl(result.pt, pt);
    return result;
}

mcl_bn128_G1 mcl_bn128_G1::zero()
{
    return G1_zero;
}

mcl_bn128_G1 mcl_bn128_G1::one()
{
    return G1_one;
}

mcl_bn128_G1 mcl_bn128_G1::random_element()
{
    return mcl_bn128_Fr::random_element().as_bigint() * G1_one;
}

std::ostream& operator<<(std::ostream &out, const mcl_bn128_G1 &g)
{
    g.pt.normalize();

    out << (g.is_zero() ? '1' : '0') << OUTPUT_SEPARATOR;

#ifdef NO_PT_COMPRESSION
    /* no point compression case */
#ifndef BINARY_OUTPUT
    out << g.pt.x << OUTPUT_SEPARATOR << g.pt.y;
#else
    out.write((char*) &g.pt.x, sizeof(g.pt.x));
    out.write((char*) &g.pt.y, sizeof(g.pt.y));
#endif

#else
    /* point compression case */
#ifndef BINARY_OUTPUT
    out << g.pt.x;
#else
    out.write((char*) &g.pt.x, sizeof(g.pt.x));
#endif
    out << OUTPUT_SEPARATOR << (g.pt.y.getUnit()[0] & 1 ? '1' : '0');
#endif

    return out;
}

bool mcl_bn128_G1::is_well_formed() const
{
    return pt.isValid();
}

std::istream& operator>>(std::istream &in, mcl_bn128_G1 &g)
{
    char is_zero;
    in.read((char*)&is_zero, 1); // this reads is_zero;
    is_zero -= '0';
    consume_OUTPUT_SEPARATOR(in);

#ifdef NO_PT_COMPRESSION
    /* no point compression case */
#ifndef BINARY_OUTPUT
    in >> g.pt.x;
    consume_OUTPUT_SEPARATOR(in);
    in >> g.pt.y;
#else
    in.read((char*) &g.pt.x, sizeof(g.pt.x));
    in.read((char*) &g.pt.y, sizeof(g.pt.y));
#endif

#else
    /* point compression case */
#ifndef BINARY_OUTPUT
    in >> g.pt.x;
#else
    in.read((char*)&g.pt.x, sizeof(g.pt.x));
#endif
    consume_OUTPUT_SEPARATOR(in);
    unsigned char Y_lsb;
    in.read((char*)&Y_lsb, 1);
    Y_lsb -= '0';

    // y = +/- sqrt(x^3 + b)
    if (!is_zero)
    {
        mcl::bn256::G1::getWeierstrass(g.pt.y, g.pt.x);
        Fp::squareRoot(g.pt.y, g.pt.y);
        if ((g.pt.y.getUnit()[0] & 1) != Y_lsb)
        {
            Fp::neg(g.pt.y, g.pt.y);
        }
    }
#endif

    /* finalize */
    if (!is_zero)
    {
        g.pt.z = Fp::one();
    }
    else
    {
        g.pt.clear();
    }

    return in;
}

std::ostream& operator<<(std::ostream& out, const std::vector<mcl_bn128_G1> &v)
{
    out << v.size() << "\n";
    for (const mcl_bn128_G1& t : v)
    {
        out << t << OUTPUT_NEWLINE;
    }
    return out;
}

std::istream& operator>>(std::istream& in, std::vector<mcl_bn128_G1> &v)
{
    v.clear();

    size_t s;
    in >> s;
    consume_newline(in);
    v.reserve(s);

    for (size_t i = 0; i < s; ++i)
    {
        mcl_bn128_G1 g;
        in >> g;
        consume_OUTPUT_NEWLINE(in);
        v.emplace_back(g);
    }
    return in;
}

template<>
void batch_to_special_all_non_zeros<mcl_bn128_G1>(std::vector<mcl_bn128_G1> &vec)
{
    std::vector<Fp> Z_vec;
    Z_vec.reserve(vec.size());

    for (auto &el: vec)
    {
        Z_vec.emplace_back(el.pt.z);
    }
    bn_batch_invert<Fp>(Z_vec);

    const Fp one = 1;

    for (size_t i = 0; i < vec.size(); ++i)
    {
        Fp Z2, Z3;
        Fp::sqr(Z2, Z_vec[i]);
        Fp::mul(Z3, Z2, Z_vec[i]);

        Fp::mul(vec[i].pt.x, vec[i].pt.x, Z2);
        Fp::mul(vec[i].pt.y, vec[i].pt.y, Z3);
        vec[i].pt.z = one;
    }
}

} // libsnark
