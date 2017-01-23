/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/mcl_bn128/mcl_bn128_g2.hpp"
#include "algebra/curves/mcl_bn128/bn_utils.hpp"

using namespace mcl::bn256;

namespace libsnark {

#ifdef PROFILE_OP_COUNTS
long long mcl_bn128_G2::add_cnt = 0;
long long mcl_bn128_G2::dbl_cnt = 0;
#endif

std::vector<size_t> mcl_bn128_G2::wnaf_window_table;
std::vector<size_t> mcl_bn128_G2::fixed_base_exp_window_table;
mcl_bn128_G2 mcl_bn128_G2::G2_zero;
mcl_bn128_G2 mcl_bn128_G2::G2_one;

Fp2 mcl_bn128_G2::sqrt(const Fp2 &el)
{
    Fp2 x;
    bool ok = Fp2::squareRoot(x, el);
    assert(ok);
    (void)ok;
    return x;
}

mcl_bn128_G2::mcl_bn128_G2()
{
    this->pt.clear();
}

void mcl_bn128_G2::print() const
{
    if (this->is_zero())
    {
        printf("O\n");
    }
    else
    {
        pt.normalize();
        std::cout << "(" << pt.x << " : " << pt.y << " : " << pt.z << ")\n";
    }
}

void mcl_bn128_G2::print_coordinates() const
{
    if (this->is_zero())
    {
        printf("O\n");
    }
    else
    {
        std::cout << "(" << pt.x << " : " << pt.y << " : " << pt.z << ")\n";
    }
}

void mcl_bn128_G2::to_affine_coordinates()
{
    pt.normalize();
}

void mcl_bn128_G2::to_special()
{
    this->to_affine_coordinates();
}

bool mcl_bn128_G2::is_special() const
{
    return (this->is_zero() || this->pt.z == 1);
}

bool mcl_bn128_G2::is_zero() const
{
    return pt.isZero();
}

bool mcl_bn128_G2::operator==(const mcl_bn128_G2 &other) const
{
    return pt == other.pt;
}

bool mcl_bn128_G2::operator!=(const mcl_bn128_G2& other) const
{
    return !(operator==(other));
}

mcl_bn128_G2 mcl_bn128_G2::operator+(const mcl_bn128_G2 &other) const
{
    return this->add(other);
}

mcl_bn128_G2 mcl_bn128_G2::operator-() const
{
    mcl_bn128_G2 result;
    G2::neg(result.pt, pt);
    return result;
}

mcl_bn128_G2 mcl_bn128_G2::operator-(const mcl_bn128_G2 &other) const
{
#ifdef PROFILE_OP_COUNTS
    this->add_cnt++;
#endif
    mcl_bn128_G2 result;
    G2::sub(result.pt, pt, other.pt);
    return result;
}

mcl_bn128_G2 mcl_bn128_G2::add(const mcl_bn128_G2 &other) const
{
#ifdef PROFILE_OP_COUNTS
    this->add_cnt++;
#endif
    mcl_bn128_G2 result;
    mcl::bn256::G2::add(result.pt, pt, other.pt);
    return result;
}

mcl_bn128_G2 mcl_bn128_G2::mixed_add(const mcl_bn128_G2 &other) const
{
#ifdef PROFILE_OP_COUNTS
    this->add_cnt++;
#endif
    mcl_bn128_G2 result;
    G2::add(result.pt, pt, other.pt);
    return result;
}

mcl_bn128_G2 mcl_bn128_G2::dbl() const
{
#ifdef PROFILE_OP_COUNTS
    this->dbl_cnt++;
#endif
    mcl_bn128_G2 result;
    G2::dbl(result.pt, pt);
    return result;
}

bool mcl_bn128_G2::is_well_formed() const
{
    return pt.isValid();
}

mcl_bn128_G2 mcl_bn128_G2::zero()
{
    return G2_zero;
}

mcl_bn128_G2 mcl_bn128_G2::one()
{
    return G2_one;
}

mcl_bn128_G2 mcl_bn128_G2::random_element()
{
    return mcl_bn128_Fr::random_element().as_bigint() * G2_one;
}

std::ostream& operator<<(std::ostream &out, const mcl_bn128_G2 &g)
{
    g.pt.normalize();

    out << (g.is_zero() ? '1' : '0') << OUTPUT_SEPARATOR;

#ifdef NO_PT_COMPRESSION
    /* no point compression case */
#ifndef BINARY_OUTPUT
    out << g.pt.x.a << OUTPUT_SEPARATOR << g.pt.x.b << OUTPUT_SEPARATOR;
    out << g.pt.y.a << OUTPUT_SEPARATOR << g.pt.y.b;
#else
    out.write((char*) &g.pt, sizeof(g.pt));
#endif

#else
    /* point compression case */
#ifndef BINARY_OUTPUT
    out << g.pt.x.a << OUTPUT_SEPARATOR << g.pt.x.b;
#else
    out.write((char*) &g.pt.x, sizeof(g.pt.x));
#endif
    out << OUTPUT_SEPARATOR << (g.pt.y.a.getUnit()[0] & 1 ? '1' : '0');
#endif

    return out;
}

std::istream& operator>>(std::istream &in, mcl_bn128_G2 &g)
{
    char is_zero;
    in.read((char*)&is_zero, 1); // this reads is_zero;
    is_zero -= '0';
    consume_OUTPUT_SEPARATOR(in);

#ifdef NO_PT_COMPRESSION
    /* no point compression case */
#ifndef BINARY_OUTPUT
    in >> g.pt.x.a;
    consume_OUTPUT_SEPARATOR(in);
    in >> g.pt.x.b;
    consume_OUTPUT_SEPARATOR(in);
    in >> g.pt.y.a;
    consume_OUTPUT_SEPARATOR(in);
    in >> g.pt.y.b;
#else
    in.read((char*) &g.pt, sizeof(g.pt));
#endif

#else
    /* point compression case */
#ifndef BINARY_OUTPUT
    in >> g.pt.x.a;
    consume_OUTPUT_SEPARATOR(in);
    in >> g.pt.x.b;
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
        mcl::bn256::G2::getWeierstrass(g.pt.y, g.pt.x);
        Fp2::squareRoot(g.pt.y, g.pt.y);
        if ((g.pt.y.a.getUnit()[0] & 1) != Y_lsb)
        {
            Fp2::neg(g.pt.y, g.pt.y);
        }
    }
#endif

    /* finalize */
    if (!is_zero)
    {
        g.pt.z.a = Fp::one();
		g.pt.z.b.clear();
    }
    else
    {
        g.pt.clear();
    }

    return in;
}

template<>
void batch_to_special_all_non_zeros<mcl_bn128_G2>(std::vector<mcl_bn128_G2> &vec)
{
    std::vector<Fp2> Z_vec;
    Z_vec.reserve(vec.size());

    for (auto &el: vec)
    {
        Z_vec.emplace_back(el.pt.z);
    }
    bn_batch_invert<Fp2>(Z_vec);

    const Fp2 one = 1;

    for (size_t i = 0; i < vec.size(); ++i)
    {
        Fp2 Z2, Z3;
        Fp2::sqr(Z2, Z_vec[i]);
        Fp2::mul(Z3, Z2, Z_vec[i]);

        Fp2::mul(vec[i].pt.x, vec[i].pt.x, Z2);
        Fp2::mul(vec[i].pt.y, vec[i].pt.y, Z3);
        vec[i].pt.z = one;
    }
}

} // libsnark
