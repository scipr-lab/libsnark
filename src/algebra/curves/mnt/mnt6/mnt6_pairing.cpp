/** @file
 *****************************************************************************

 Implementation of interfaces for pairing operations on MNT6.

 See mnt6_pairing.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <cassert>

#include "algebra/curves/mnt/mnt6/mnt6_pairing.hpp"
#include "algebra/curves/mnt/mnt6/mnt6_init.hpp"
#include "algebra/curves/mnt/mnt6/mnt6_g1.hpp"
#include "algebra/curves/mnt/mnt6/mnt6_g2.hpp"
#include "algebra/scalar_multiplication/wnaf.hpp"
#include "common/profiling.hpp"

namespace libsnark {

bool mnt6_ate_G1_precomp::operator==(const mnt6_ate_G1_precomp &other) const
{
    return (this->PX == other.PX &&
            this->PY == other.PY &&
            this->PX_twist == other.PX_twist &&
            this->PY_twist == other.PY_twist);
}

std::ostream& operator<<(std::ostream &out, const mnt6_ate_G1_precomp &prec_P)
{
    out << prec_P.PX << OUTPUT_SEPARATOR << prec_P.PY << OUTPUT_SEPARATOR << prec_P.PX_twist << OUTPUT_SEPARATOR << prec_P.PY_twist;

    return out;
}

std::istream& operator>>(std::istream &in, mnt6_ate_G1_precomp &prec_P)
{
    in >> prec_P.PX;
    consume_OUTPUT_SEPARATOR(in);
    in >> prec_P.PY;
    consume_OUTPUT_SEPARATOR(in);
    in >> prec_P.PX_twist;
    consume_OUTPUT_SEPARATOR(in);
    in >> prec_P.PY_twist;

    return in;
}

bool mnt6_ate_dbl_coeffs::operator==(const mnt6_ate_dbl_coeffs &other) const
{
    return (this->c_H == other.c_H &&
            this->c_4C == other.c_4C &&
            this->c_J == other.c_J &&
            this->c_L == other.c_L);
}

std::ostream& operator<<(std::ostream &out, const mnt6_ate_dbl_coeffs &dc)
{
    out << dc.c_H << OUTPUT_SEPARATOR << dc.c_4C << OUTPUT_SEPARATOR << dc.c_J << OUTPUT_SEPARATOR << dc.c_L;
    return out;
}

std::istream& operator>>(std::istream &in, mnt6_ate_dbl_coeffs &dc)
{
    in >> dc.c_H;
    consume_OUTPUT_SEPARATOR(in);
    in >> dc.c_4C;
    consume_OUTPUT_SEPARATOR(in);
    in >> dc.c_J;
    consume_OUTPUT_SEPARATOR(in);
    in >> dc.c_L;

    return in;
}

bool mnt6_ate_add_coeffs::operator==(const mnt6_ate_add_coeffs &other) const
{
    return (this->c_L1 == other.c_L1 &&
            this->c_RZ == other.c_RZ);
}

std::ostream& operator<<(std::ostream &out, const mnt6_ate_add_coeffs &ac)
{
    out << ac.c_L1 << OUTPUT_SEPARATOR << ac.c_RZ;
    return out;
}

std::istream& operator>>(std::istream &in, mnt6_ate_add_coeffs &ac)
{
    in >> ac.c_L1;
    consume_OUTPUT_SEPARATOR(in);
    in >> ac.c_RZ;

    return in;
}


bool mnt6_ate_G2_precomp::operator==(const mnt6_ate_G2_precomp &other) const
{
    return (this->QX == other.QX &&
            this->QY == other.QY &&
            this->QY2 == other.QY2 &&
            this->QX_over_twist == other.QX_over_twist &&
            this->QY_over_twist == other.QY_over_twist &&
            this->dbl_coeffs == other.dbl_coeffs &&
            this->add_coeffs == other.add_coeffs);
}

std::ostream& operator<<(std::ostream& out, const mnt6_ate_G2_precomp &prec_Q)
{
    out << prec_Q.QX << OUTPUT_SEPARATOR
        << prec_Q.QY << OUTPUT_SEPARATOR
        << prec_Q.QY2  << OUTPUT_SEPARATOR
        << prec_Q.QX_over_twist << OUTPUT_SEPARATOR
        << prec_Q.QY_over_twist << "\n";
    out << prec_Q.dbl_coeffs.size() << "\n";
    for (const mnt6_ate_dbl_coeffs &dc : prec_Q.dbl_coeffs)
    {
        out << dc << OUTPUT_NEWLINE;
    }
    out << prec_Q.add_coeffs.size() << "\n";
    for (const mnt6_ate_add_coeffs &ac : prec_Q.add_coeffs)
    {
        out << ac << OUTPUT_NEWLINE;
    }

    return out;
}

std::istream& operator>>(std::istream& in, mnt6_ate_G2_precomp &prec_Q)
{
    in >> prec_Q.QX;
    consume_OUTPUT_SEPARATOR(in);
    in >> prec_Q.QY;
    consume_OUTPUT_SEPARATOR(in);
    in >> prec_Q.QY2;
    consume_OUTPUT_SEPARATOR(in);
    in >> prec_Q.QX_over_twist;
    consume_OUTPUT_SEPARATOR(in);
    in >> prec_Q.QY_over_twist;
    consume_newline(in);

    prec_Q.dbl_coeffs.clear();
    size_t dbl_s;
    in >> dbl_s;
    consume_newline(in);

    prec_Q.dbl_coeffs.reserve(dbl_s);

    for (size_t i = 0; i < dbl_s; ++i)
    {
        mnt6_ate_dbl_coeffs dc;
        in >> dc;
        consume_OUTPUT_NEWLINE(in);
        prec_Q.dbl_coeffs.emplace_back(dc);
    }

    prec_Q.add_coeffs.clear();
    size_t add_s;
    in >> add_s;
    consume_newline(in);

    prec_Q.add_coeffs.reserve(add_s);

    for (size_t i = 0; i < add_s; ++i)
    {
        mnt6_ate_add_coeffs ac;
        in >> ac;
        consume_OUTPUT_NEWLINE(in);
        prec_Q.add_coeffs.emplace_back(ac);
    }

    return in;
}

/* final exponentiations */

mnt6_Fq6 mnt6_final_exponentiation_last_chunk(const mnt6_Fq6 &elt, const mnt6_Fq6 &elt_inv)
{
    enter_block("Call to mnt6_final_exponentiation_last_chunk");
    const mnt6_Fq6 elt_q = elt.Frobenius_map(1);
    mnt6_Fq6 w1_part = elt_q.cyclotomic_exp(mnt6_final_exponent_last_chunk_w1);
    mnt6_Fq6 w0_part;
    if (mnt6_final_exponent_last_chunk_is_w0_neg)
    {
    	w0_part = elt_inv.cyclotomic_exp(mnt6_final_exponent_last_chunk_abs_of_w0);
    } else {
    	w0_part = elt.cyclotomic_exp(mnt6_final_exponent_last_chunk_abs_of_w0);
    }
    mnt6_Fq6 result = w1_part * w0_part;
    leave_block("Call to mnt6_final_exponentiation_last_chunk");

    return result;
}

mnt6_Fq6 mnt6_final_exponentiation_first_chunk(const mnt6_Fq6 &elt, const mnt6_Fq6 &elt_inv)
{
    enter_block("Call to mnt6_final_exponentiation_first_chunk");

    /* (q^3-1)*(q+1) */

    /* elt_q3 = elt^(q^3) */
    const mnt6_Fq6 elt_q3 = elt.Frobenius_map(3);
    /* elt_q3_over_elt = elt^(q^3-1) */
    const mnt6_Fq6 elt_q3_over_elt = elt_q3 * elt_inv;
    /* alpha = elt^((q^3-1) * q) */
    const mnt6_Fq6 alpha = elt_q3_over_elt.Frobenius_map(1);
    /* beta = elt^((q^3-1)*(q+1) */
    const mnt6_Fq6 beta = alpha * elt_q3_over_elt;
    leave_block("Call to mnt6_final_exponentiation_first_chunk");
    return beta;
}

mnt6_GT mnt6_final_exponentiation(const mnt6_Fq6 &elt)
{
    enter_block("Call to mnt6_final_exponentiation");
    const mnt6_Fq6 elt_inv = elt.inverse();
    const mnt6_Fq6 elt_to_first_chunk = mnt6_final_exponentiation_first_chunk(elt, elt_inv);
    const mnt6_Fq6 elt_inv_to_first_chunk = mnt6_final_exponentiation_first_chunk(elt_inv, elt);
    mnt6_GT result = mnt6_final_exponentiation_last_chunk(elt_to_first_chunk, elt_inv_to_first_chunk);
    leave_block("Call to mnt6_final_exponentiation");

    return result;
}

/* affine ate miller loop */

mnt6_affine_ate_G1_precomputation mnt6_affine_ate_precompute_G1(const mnt6_G1& P)
{
    enter_block("Call to mnt6_affine_ate_precompute_G1");

    mnt6_G1 Pcopy = P;
    Pcopy.to_affine_coordinates();

    mnt6_affine_ate_G1_precomputation result;
    result.PX = Pcopy.X();
    result.PY = Pcopy.Y();
    result.PY_twist_squared = Pcopy.Y() * mnt6_twist.squared();

    leave_block("Call to mnt6_affine_ate_precompute_G1");
    return result;
}

mnt6_affine_ate_G2_precomputation mnt6_affine_ate_precompute_G2(const mnt6_G2& Q)
{
    enter_block("Call to mnt6_affine_ate_precompute_G2");

    mnt6_G2 Qcopy(Q);
    Qcopy.to_affine_coordinates();

    mnt6_affine_ate_G2_precomputation result;
    result.QX = Qcopy.X();
    result.QY = Qcopy.Y();

    mnt6_Fq3 RX = Qcopy.X();
    mnt6_Fq3 RY = Qcopy.Y();

    const bigint<mnt6_Fr::num_limbs> &loop_count = mnt6_ate_loop_count;
    bool found_nonzero = false;

    std::vector<long> NAF = find_wnaf(1, loop_count);
    for (long i = NAF.size() - 1; i >= 0; --i)
    {
        if (!found_nonzero)
        {
            /* this skips the MSB itself */
            found_nonzero |= (NAF[i] != 0);
            continue;
        }

        mnt6_affine_ate_coeffs c;
        c.old_RX = RX;
        c.old_RY = RY;
        mnt6_Fq3 old_RX_2 = c.old_RX.squared();
        c.gamma = (old_RX_2 + old_RX_2 + old_RX_2 + mnt6_twist_coeff_a) * (c.old_RY + c.old_RY).inverse();
        c.gamma_twist = c.gamma * mnt6_twist;
        c.gamma_X = c.gamma * c.old_RX;
        result.coeffs.push_back(c);

        RX = c.gamma.squared() - (c.old_RX+c.old_RX);
        RY = c.gamma * (c.old_RX - RX) - c.old_RY;

        if (NAF[i] != 0)
        {
            mnt6_affine_ate_coeffs c;
            c.old_RX = RX;
            c.old_RY = RY;
            if (NAF[i] > 0)
            {
                c.gamma = (c.old_RY - result.QY) * (c.old_RX - result.QX).inverse();
            }
            else
            {
                c.gamma = (c.old_RY + result.QY) * (c.old_RX - result.QX).inverse();
            }
            c.gamma_twist = c.gamma * mnt6_twist;
            c.gamma_X = c.gamma * result.QX;
            result.coeffs.push_back(c);

            RX = c.gamma.squared() - (c.old_RX+result.QX);
            RY = c.gamma * (c.old_RX - RX) - c.old_RY;
        }
    }

    /* TODO: maybe handle neg
    if (mnt6_ate_is_loop_count_neg)
    {
    	mnt6_ate_add_coeffs ac;
		mnt6_affine_ate_dbl_coeffs c;
		c.old_RX = RX;
		c.old_RY = -RY;
		old_RX_2 = c.old_RY.squared();
		c.gamma = (old_RX_2 + old_RX_2 + old_RX_2 + mnt6_coeff_a) * (c.old_RY + c.old_RY).inverse();
		c.gamma_twist = c.gamma * mnt6_twist;
		c.gamma_X = c.gamma * c.old_RX;
		result.coeffs.push_back(c);
    }
    */

    leave_block("Call to mnt6_affine_ate_precompute_G2");
    return result;
}

mnt6_Fq6 mnt6_affine_ate_miller_loop(const mnt6_affine_ate_G1_precomputation &prec_P,
                                     const mnt6_affine_ate_G2_precomputation &prec_Q)
{
    enter_block("Call to mnt6_affine_ate_miller_loop");

    mnt6_Fq6 f = mnt6_Fq6::one();

    const bigint<mnt6_Fr::num_limbs> &loop_count = mnt6_ate_loop_count;
    bool found_nonzero = false;
    size_t idx = 0;

    std::vector<long> NAF = find_wnaf(1, loop_count);
    for (long i = NAF.size() - 1; i >= 0; --i)
    {
        if (!found_nonzero)
        {
            /* this skips the MSB itself */
            found_nonzero |= (NAF[i] != 0);
            continue;
        }

        /* code below gets executed for all bits (EXCEPT the MSB itself) of
           mnt6_param_p (skipping leading zeros) in MSB to LSB
           order */
        mnt6_affine_ate_coeffs c = prec_Q.coeffs[idx++];

        mnt6_Fq6 g_RR_at_P = mnt6_Fq6(prec_P.PY_twist_squared,
                                      - prec_P.PX * c.gamma_twist + c.gamma_X - c.old_RY);
        f = f.squared().mul_by_2345(g_RR_at_P);

        if (NAF[i] != 0)
        {
            mnt6_affine_ate_coeffs c = prec_Q.coeffs[idx++];
            mnt6_Fq6 g_RQ_at_P;
            if (NAF[i] > 0)
            {
                g_RQ_at_P = mnt6_Fq6(prec_P.PY_twist_squared,
                                     - prec_P.PX * c.gamma_twist + c.gamma_X - prec_Q.QY);
            }
            else
            {
                g_RQ_at_P = mnt6_Fq6(prec_P.PY_twist_squared,
                                     - prec_P.PX * c.gamma_twist + c.gamma_X + prec_Q.QY);
            }
            f = f.mul_by_2345(g_RQ_at_P);
        }

    }

    /* TODO: maybe handle neg
    if (mnt6_ate_is_loop_count_neg)
    {
    	// TODO:
    	mnt6_affine_ate_coeffs ac = prec_Q.coeffs[idx++];
    	mnt6_Fq6 g_RnegR_at_P = mnt6_Fq6(prec_P.PY_twist_squared,
                                          - prec_P.PX * c.gamma_twist + c.gamma_X - c.old_RY);
    	f = (f * g_RnegR_at_P).inverse();
    }
    */

    leave_block("Call to mnt6_affine_ate_miller_loop");

    return f;
}

/* ate pairing */

struct extended_mnt6_G2_projective {
    mnt6_Fq3 X;
    mnt6_Fq3 Y;
    mnt6_Fq3 Z;
    mnt6_Fq3 T;

    void print() const
    {
        printf("extended mnt6_G2 projective X/Y/Z/T:\n");
        X.print();
        Y.print();
        Z.print();
        T.print();
    }

    void test_invariant() const
    {
        assert(T == Z.squared());
    }
};

void doubling_step_for_flipped_miller_loop(extended_mnt6_G2_projective &current,
                                           mnt6_ate_dbl_coeffs &dc)
{
    const mnt6_Fq3 X = current.X, Y = current.Y, Z = current.Z, T = current.T;

    const mnt6_Fq3 A = T.squared(); // A = T1^2
    const mnt6_Fq3 B = X.squared(); // B = X1^2
    const mnt6_Fq3 C = Y.squared(); // C = Y1^2
    const mnt6_Fq3 D = C.squared(); // D = C^2
    const mnt6_Fq3 E = (X+C).squared() - B - D; // E = (X1+C)^2-B-D
    const mnt6_Fq3 F = (B+B+B) + mnt6_twist_coeff_a * A; // F = 3*B +  a  *A
    const mnt6_Fq3 G = F.squared(); // G = F^2

    current.X = -(E+E+E+E) + G; // X3 = -4*E+G
    current.Y = -mnt6_Fq("8")*D + F*(E+E-current.X); // Y3 = -8*D+F*(2*E-X3)
    current.Z = (Y+Z).squared() - C - Z.squared(); // Z3 = (Y1+Z1)^2-C-Z1^2
    current.T = current.Z.squared(); // T3 = Z3^2

    dc.c_H = (current.Z + T).squared() - current.T - A; // H = (Z3+T1)^2-T3-A
    dc.c_4C = C+C+C+C; // fourC = 4*C
    dc.c_J = (F+T).squared() - G - A; // J = (F+T1)^2-G-A
    dc.c_L = (F+X).squared() - G - B; // L = (F+X1)^2-G-B

#ifdef DEBUG
    current.test_invariant();
#endif
}

void mixed_addition_step_for_flipped_miller_loop(const mnt6_Fq3 base_X, const mnt6_Fq3 base_Y, const mnt6_Fq3 base_Y_squared,
                                                 extended_mnt6_G2_projective &current,
                                                 mnt6_ate_add_coeffs &ac)
{
    const mnt6_Fq3 X1 = current.X, Y1 = current.Y, Z1 = current.Z, T1 = current.T;
    const mnt6_Fq3 &x2 = base_X,    &y2 =  base_Y, &y2_squared = base_Y_squared;

    const mnt6_Fq3 B = x2 * T1; // B = x2 * T1
    const mnt6_Fq3 D = ((y2 + Z1).squared() - y2_squared - T1) * T1; // D = ((y2 + Z1)^2 - y2squared - T1) * T1
    const mnt6_Fq3 H = B - X1; // H = B - X1
    const mnt6_Fq3 I = H.squared(); // I = H^2
    const mnt6_Fq3 E = I + I + I + I; // E = 4*I
    const mnt6_Fq3 J = H * E; // J = H * E
    const mnt6_Fq3 V = X1 * E; // V = X1 * E
    const mnt6_Fq3 L1 = D - (Y1 + Y1); // L1 = D - 2 * Y1

    current.X = L1.squared() - J - (V+V); // X3 = L1^2 - J - 2*V
    current.Y = L1 * (V-current.X) - (Y1+Y1) * J; // Y3 = L1 * (V-X3) - 2*Y1 * J
    current.Z = (Z1+H).squared() - T1 - I; // Z3 = (Z1 + H)^2 - T1 - I
    current.T = current.Z.squared(); // T3 = Z3^2

    ac.c_L1 = L1;
    ac.c_RZ = current.Z;
#ifdef DEBUG
    current.test_invariant();
#endif
}

mnt6_ate_G1_precomp mnt6_ate_precompute_G1(const mnt6_G1& P)
{
    enter_block("Call to mnt6_ate_precompute_G1");

    mnt6_G1 Pcopy = P;
    Pcopy.to_affine_coordinates();

    mnt6_ate_G1_precomp result;
    result.PX = Pcopy.X();
    result.PY = Pcopy.Y();
    result.PX_twist = Pcopy.X() * mnt6_twist;
    result.PY_twist = Pcopy.Y() * mnt6_twist;

    leave_block("Call to mnt6_ate_precompute_G1");
    return result;
}

mnt6_ate_G2_precomp mnt6_ate_precompute_G2(const mnt6_G2& Q)
{
    enter_block("Call to mnt6_ate_precompute_G2");

    mnt6_G2 Qcopy(Q);
    Qcopy.to_affine_coordinates();

    mnt6_Fq3 mnt6_twist_inv = mnt6_twist.inverse(); // could add to global params if needed

    mnt6_ate_G2_precomp result;
    result.QX = Qcopy.X();
    result.QY = Qcopy.Y();
    result.QY2 = Qcopy.Y().squared();
    result.QX_over_twist = Qcopy.X() * mnt6_twist_inv;
    result.QY_over_twist = Qcopy.Y() * mnt6_twist_inv;

    extended_mnt6_G2_projective R;
    R.X = Qcopy.X();
    R.Y = Qcopy.Y();
    R.Z = mnt6_Fq3::one();
    R.T = mnt6_Fq3::one();

    const bigint<mnt6_Fr::num_limbs> &loop_count = mnt6_ate_loop_count;
    bool found_one = false;
    for (long i = loop_count.max_bits() - 1; i >= 0; --i)
    {
        const bool bit = loop_count.test_bit(i);

        if (!found_one)
        {
            /* this skips the MSB itself */
            found_one |= bit;
            continue;
        }

        mnt6_ate_dbl_coeffs dc;
        doubling_step_for_flipped_miller_loop(R, dc);
        result.dbl_coeffs.push_back(dc);

        if (bit)
        {
            mnt6_ate_add_coeffs ac;
            mixed_addition_step_for_flipped_miller_loop(result.QX, result.QY, result.QY2, R, ac);
            result.add_coeffs.push_back(ac);
        }
    }

    if (mnt6_ate_is_loop_count_neg)
    {
    	mnt6_Fq3 RZ_inv = R.Z.inverse();
    	mnt6_Fq3 RZ2_inv = RZ_inv.squared();
    	mnt6_Fq3 RZ3_inv = RZ2_inv * RZ_inv;
    	mnt6_Fq3 minus_R_affine_X = R.X * RZ2_inv;
    	mnt6_Fq3 minus_R_affine_Y = - R.Y * RZ3_inv;
    	mnt6_Fq3 minus_R_affine_Y2 = minus_R_affine_Y.squared();
    	mnt6_ate_add_coeffs ac;
        mixed_addition_step_for_flipped_miller_loop(minus_R_affine_X, minus_R_affine_Y, minus_R_affine_Y2, R, ac);
        result.add_coeffs.push_back(ac);
    }

    leave_block("Call to mnt6_ate_precompute_G2");
    return result;
}

mnt6_Fq6 mnt6_ate_miller_loop(const mnt6_ate_G1_precomp &prec_P,
                              const mnt6_ate_G2_precomp &prec_Q)
{
    enter_block("Call to mnt6_ate_miller_loop");

    mnt6_Fq3 L1_coeff = mnt6_Fq3(prec_P.PX, mnt6_Fq::zero(), mnt6_Fq::zero()) - prec_Q.QX_over_twist;

    mnt6_Fq6 f = mnt6_Fq6::one();

    bool found_one = false;
    size_t dbl_idx = 0;
    size_t add_idx = 0;

    const bigint<mnt6_Fr::num_limbs> &loop_count = mnt6_ate_loop_count;

    for (long i = loop_count.max_bits() - 1; i >= 0; --i)
    {
        const bool bit = loop_count.test_bit(i);

        if (!found_one)
        {
            /* this skips the MSB itself */
            found_one |= bit;
            continue;
        }

        /* code below gets executed for all bits (EXCEPT the MSB itself) of
           mnt6_param_p (skipping leading zeros) in MSB to LSB
           order */
        mnt6_ate_dbl_coeffs dc = prec_Q.dbl_coeffs[dbl_idx++];

        mnt6_Fq6 g_RR_at_P = mnt6_Fq6(- dc.c_4C - dc.c_J * prec_P.PX_twist + dc.c_L,
                                      dc.c_H * prec_P.PY_twist);
        f = f.squared() * g_RR_at_P;

        if (bit)
        {
            mnt6_ate_add_coeffs ac = prec_Q.add_coeffs[add_idx++];
            mnt6_Fq6 g_RQ_at_P = mnt6_Fq6(ac.c_RZ * prec_P.PY_twist,
                                          -(prec_Q.QY_over_twist * ac.c_RZ + L1_coeff * ac.c_L1));
            f = f * g_RQ_at_P;
        }

    }

    if (mnt6_ate_is_loop_count_neg)
    {
    	mnt6_ate_add_coeffs ac = prec_Q.add_coeffs[add_idx++];
    	mnt6_Fq6 g_RnegR_at_P = mnt6_Fq6(ac.c_RZ * prec_P.PY_twist,
                                         -(prec_Q.QY_over_twist * ac.c_RZ + L1_coeff * ac.c_L1));
    	f = (f * g_RnegR_at_P).inverse();
    }

    leave_block("Call to mnt6_ate_miller_loop");

    return f;
}

mnt6_Fq6 mnt6_ate_double_miller_loop(const mnt6_ate_G1_precomp &prec_P1,
                                     const mnt6_ate_G2_precomp &prec_Q1,
                                     const mnt6_ate_G1_precomp &prec_P2,
                                     const mnt6_ate_G2_precomp &prec_Q2)
{
    enter_block("Call to mnt6_ate_double_miller_loop");

    mnt6_Fq3 L1_coeff1 = mnt6_Fq3(prec_P1.PX, mnt6_Fq::zero(), mnt6_Fq::zero()) - prec_Q1.QX_over_twist;
    mnt6_Fq3 L1_coeff2 = mnt6_Fq3(prec_P2.PX, mnt6_Fq::zero(), mnt6_Fq::zero()) - prec_Q2.QX_over_twist;

    mnt6_Fq6 f = mnt6_Fq6::one();

    bool found_one = false;
    size_t dbl_idx = 0;
    size_t add_idx = 0;

    const bigint<mnt6_Fr::num_limbs> &loop_count = mnt6_ate_loop_count;

    for (long i = loop_count.max_bits() - 1; i >= 0; --i)
    {
        const bool bit = loop_count.test_bit(i);

        if (!found_one)
        {
            /* this skips the MSB itself */
            found_one |= bit;
            continue;
        }

        /* code below gets executed for all bits (EXCEPT the MSB itself) of
           mnt6_param_p (skipping leading zeros) in MSB to LSB
           order */
        mnt6_ate_dbl_coeffs dc1 = prec_Q1.dbl_coeffs[dbl_idx];
        mnt6_ate_dbl_coeffs dc2 = prec_Q2.dbl_coeffs[dbl_idx];
        ++dbl_idx;

        mnt6_Fq6 g_RR_at_P1 = mnt6_Fq6(- dc1.c_4C - dc1.c_J * prec_P1.PX_twist + dc1.c_L,
                                       dc1.c_H * prec_P1.PY_twist);

        mnt6_Fq6 g_RR_at_P2 = mnt6_Fq6(- dc2.c_4C - dc2.c_J * prec_P2.PX_twist + dc2.c_L,
                                       dc2.c_H * prec_P2.PY_twist);

        f = f.squared() * g_RR_at_P1 * g_RR_at_P2;

        if (bit)
        {
            mnt6_ate_add_coeffs ac1 = prec_Q1.add_coeffs[add_idx];
            mnt6_ate_add_coeffs ac2 = prec_Q2.add_coeffs[add_idx];
            ++add_idx;

            mnt6_Fq6 g_RQ_at_P1 = mnt6_Fq6(ac1.c_RZ * prec_P1.PY_twist,
                                           -(prec_Q1.QY_over_twist * ac1.c_RZ + L1_coeff1 * ac1.c_L1));
            mnt6_Fq6 g_RQ_at_P2 = mnt6_Fq6(ac2.c_RZ * prec_P2.PY_twist,
                                           -(prec_Q2.QY_over_twist * ac2.c_RZ + L1_coeff2 * ac2.c_L1));

            f = f * g_RQ_at_P1 * g_RQ_at_P2;
        }
    }

    if (mnt6_ate_is_loop_count_neg)
    {
    	mnt6_ate_add_coeffs ac1 = prec_Q1.add_coeffs[add_idx];
        mnt6_ate_add_coeffs ac2 = prec_Q2.add_coeffs[add_idx];
    	++add_idx;
    	mnt6_Fq6 g_RnegR_at_P1 = mnt6_Fq6(ac1.c_RZ * prec_P1.PY_twist,
                                          -(prec_Q1.QY_over_twist * ac1.c_RZ + L1_coeff1 * ac1.c_L1));
    	mnt6_Fq6 g_RnegR_at_P2 = mnt6_Fq6(ac2.c_RZ * prec_P2.PY_twist,
                                          -(prec_Q2.QY_over_twist * ac2.c_RZ + L1_coeff2 * ac2.c_L1));

    	f = (f * g_RnegR_at_P1 * g_RnegR_at_P2).inverse();
    }

    leave_block("Call to mnt6_ate_double_miller_loop");

    return f;
}

mnt6_Fq6 mnt6_ate_pairing(const mnt6_G1& P, const mnt6_G2 &Q)
{
    enter_block("Call to mnt6_ate_pairing");
    mnt6_ate_G1_precomp prec_P = mnt6_ate_precompute_G1(P);
    mnt6_ate_G2_precomp prec_Q = mnt6_ate_precompute_G2(Q);
    mnt6_Fq6 result = mnt6_ate_miller_loop(prec_P, prec_Q);
    leave_block("Call to mnt6_ate_pairing");
    return result;
}

mnt6_GT mnt6_ate_reduced_pairing(const mnt6_G1 &P, const mnt6_G2 &Q)
{
    enter_block("Call to mnt6_ate_reduced_pairing");
    const mnt6_Fq6 f = mnt6_ate_pairing(P, Q);
    const mnt6_GT result = mnt6_final_exponentiation(f);
    leave_block("Call to mnt6_ate_reduced_pairing");
    return result;
}

mnt6_G1_precomp mnt6_precompute_G1(const mnt6_G1& P)
{
    return mnt6_ate_precompute_G1(P);
}

mnt6_G2_precomp mnt6_precompute_G2(const mnt6_G2& Q)
{
    return mnt6_ate_precompute_G2(Q);
}

mnt6_Fq6 mnt6_miller_loop(const mnt6_G1_precomp &prec_P,
                          const mnt6_G2_precomp &prec_Q)
{
    return mnt6_ate_miller_loop(prec_P, prec_Q);
}

mnt6_Fq6 mnt6_double_miller_loop(const mnt6_G1_precomp &prec_P1,
                                 const mnt6_G2_precomp &prec_Q1,
                                 const mnt6_G1_precomp &prec_P2,
                                 const mnt6_G2_precomp &prec_Q2)
{
    return mnt6_ate_double_miller_loop(prec_P1, prec_Q1, prec_P2, prec_Q2);
}

mnt6_Fq6 mnt6_pairing(const mnt6_G1& P,
                      const mnt6_G2 &Q)
{
    return mnt6_ate_pairing(P, Q);
}

mnt6_GT mnt6_reduced_pairing(const mnt6_G1 &P,
                             const mnt6_G2 &Q)
{
    return mnt6_ate_reduced_pairing(P, Q);
}

mnt6_GT mnt6_affine_reduced_pairing(const mnt6_G1 &P,
                                    const mnt6_G2 &Q)
{
    const mnt6_affine_ate_G1_precomputation prec_P = mnt6_affine_ate_precompute_G1(P);
    const mnt6_affine_ate_G2_precomputation prec_Q = mnt6_affine_ate_precompute_G2(Q);
    const mnt6_Fq6 f = mnt6_affine_ate_miller_loop(prec_P, prec_Q);
    const mnt6_GT result = mnt6_final_exponentiation(f);
    return result;
}
//Ariel additions
mnt6_Fq6 mnt6_ate_multiple_miller_loop(const std::initializer_list<std::pair<
        const mnt6_G1_precomp&,
        const mnt6_G2_precomp&
    > >& v
)
{
    enter_block("Call to mnt6_ate_multiple_miller_loop");

    
    mnt6_Fq6 f = mnt6_Fq6::one();

    bool found_one = false;
    size_t dbl_idx = 0;
    size_t add_idx = 0;

    const bigint<mnt6_Fr::num_limbs> &loop_count = mnt6_ate_loop_count;
    for (long i = loop_count.max_bits() - 1; i >= 0; --i)
    {
        const bool bit = loop_count.test_bit(i);

        if (!found_one)
        {
            /* this skips the MSB itself */
            found_one |= bit;
            continue;
        }

        /* code below gets executed for all bits (EXCEPT the MSB itself) of
           mnt6_param_p (skipping leading zeros) in MSB to LSB
           order */
        
        f=f.squared();
        for(auto& p:v)
        {
            mnt6_ate_dbl_coeffs dc = p.second.dbl_coeffs[dbl_idx];
            mnt6_Fq6 g_RR_at_P = mnt6_Fq6(- dc.c_4C - dc.c_J * p.first.PX_twist + dc.c_L,
                                       dc.c_H * p.first.PY_twist);
            f=f*g_RR_at_P;
                
        }
        ++dbl_idx;

        if (bit)
        {
            for(auto& p:v)
            {
                //mnt6_Fq3 L1_coeff1 = mnt6_Fq3(prec_P1.PX, mnt6_Fq::zero(), mnt6_Fq::zero()) - prec_Q1.QX_over_twist;
    
                mnt6_Fq3 L1_coeff = mnt6_Fq3(p.first.PX, mnt6_Fq::zero(),mnt6_Fq::zero()) - p.second.QX_over_twist;
                auto ac = p.second.add_coeffs[add_idx];
                mnt6_Fq6 g_RQ_at_P = mnt6_Fq6(ac.c_RZ * p.first.PY_twist,
                                           -(p.second.QY_over_twist * ac.c_RZ + L1_coeff * ac.c_L1));
                f =f* g_RQ_at_P;
    
            }
            ++add_idx;
        }
        
    }

    if (mnt6_ate_is_loop_count_neg)
    {
        for(auto& p:v)
            {
                auto L1_coeff = mnt6_Fq3(p.first.PX, mnt6_Fq::zero(),mnt6_Fq::zero()) - p.second.QX_over_twist;
                auto ac = p.second.add_coeffs[add_idx];
                mnt6_Fq6 g_RnegR_at_P = mnt6_Fq6(ac.c_RZ * p.first.PY_twist,
                                          -(p.second.QY_over_twist * ac.c_RZ + L1_coeff * ac.c_L1));
                f =f* g_RnegR_at_P;
    
            }
        f=f.inverse();
        ++add_idx;
    }
    leave_block("Call to mnt6_ate_multiple_miller_loop");
    return f;

}
mnt6_Fq6 mnt6_multiple_miller_loop(const std::initializer_list<std::pair<
        const mnt6_G1_precomp&,
        const mnt6_G2_precomp&
    > >& v
)
{
    return mnt6_ate_multiple_miller_loop(v);
}

} // libsnark
