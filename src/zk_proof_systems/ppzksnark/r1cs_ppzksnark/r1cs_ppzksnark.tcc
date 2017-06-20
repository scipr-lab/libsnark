/** @file
*****************************************************************************

Implementation of interfaces for a ppzkSNARK for R1CS.

See r1cs_ppzksnark.hpp .

*****************************************************************************
* @author     This file is part of libsnark, developed by SCIPR Lab
*             and contributors (see AUTHORS).
* @copyright  MIT license (see LICENSE file)
*****************************************************************************/

#ifndef R1CS_PPZKSNARK_TCC_
#define R1CS_PPZKSNARK_TCC_

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>

#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>
#include <libff/algebra/scalar_multiplication/multiexp.hpp>
#include "../../../knowledge_commitment/kc_multiexp.hpp"
#include "../../../reductions/r1cs_to_qap/r1cs_to_qap.hpp"

namespace libsnark {

template<typename ppT>
bool r1cs_ppzksnark_proving_key<ppT>::operator==(const r1cs_ppzksnark_proving_key<ppT> &other) const
{
    return (this->A_query == other.A_query &&
            this->B_query == other.B_query &&
            this->C_query == other.C_query &&
            this->H_query == other.H_query &&
            this->K_query == other.K_query &&
            this->constraint_system == other.constraint_system);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_proving_key<ppT> &pk)
{
    out << pk.A_query;
    out << pk.B_query;
    out << pk.C_query;
    out << pk.H_query;
    out << pk.K_query;
    out << pk.constraint_system;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_proving_key<ppT> &pk)
{
    in >> pk.A_query;
    in >> pk.B_query;
    in >> pk.C_query;
    in >> pk.H_query;
    in >> pk.K_query;
    in >> pk.constraint_system;

    return in;
}

template<typename ppT>
bool r1cs_ppzksnark_verification_key<ppT>::operator==(const r1cs_ppzksnark_verification_key<ppT> &other) const
{
    return (this->alphaA_g2 == other.alphaA_g2 &&
            this->alphaB_g1 == other.alphaB_g1 &&
            this->alphaC_g2 == other.alphaC_g2 &&
            this->gamma_g2 == other.gamma_g2 &&
            this->gamma_beta_g1 == other.gamma_beta_g1 &&
            this->gamma_beta_g2 == other.gamma_beta_g2 &&
            this->rC_Z_g2 == other.rC_Z_g2 &&
            this->encoded_IC_query == other.encoded_IC_query);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_verification_key<ppT> &vk)
{
    out << vk.alphaA_g2 << OUTPUT_NEWLINE;
    out << vk.alphaB_g1 << OUTPUT_NEWLINE;
    out << vk.alphaC_g2 << OUTPUT_NEWLINE;
    out << vk.gamma_g2 << OUTPUT_NEWLINE;
    out << vk.gamma_beta_g1 << OUTPUT_NEWLINE;
    out << vk.gamma_beta_g2 << OUTPUT_NEWLINE;
    out << vk.rC_Z_g2 << OUTPUT_NEWLINE;
    out << vk.encoded_IC_query << OUTPUT_NEWLINE;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_verification_key<ppT> &vk)
{
    in >> vk.alphaA_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.alphaB_g1;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.alphaC_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.gamma_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.gamma_beta_g1;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.gamma_beta_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.rC_Z_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.encoded_IC_query;
    libff::consume_OUTPUT_NEWLINE(in);

    return in;
}

template<typename ppT>
bool r1cs_ppzksnark_processed_verification_key<ppT>::operator==(const r1cs_ppzksnark_processed_verification_key<ppT> &other) const
{
    return (this->pp_G2_one_precomp == other.pp_G2_one_precomp &&
            this->vk_alphaA_g2_precomp == other.vk_alphaA_g2_precomp &&
            this->vk_alphaB_g1_precomp == other.vk_alphaB_g1_precomp &&
            this->vk_alphaC_g2_precomp == other.vk_alphaC_g2_precomp &&
            this->vk_rC_Z_g2_precomp == other.vk_rC_Z_g2_precomp &&
            this->vk_gamma_g2_precomp == other.vk_gamma_g2_precomp &&
            this->vk_gamma_beta_g1_precomp == other.vk_gamma_beta_g1_precomp &&
            this->vk_gamma_beta_g2_precomp == other.vk_gamma_beta_g2_precomp &&
            this->encoded_IC_query == other.encoded_IC_query);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_processed_verification_key<ppT> &pvk)
{
    out << pvk.pp_G2_one_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_alphaA_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_alphaB_g1_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_alphaC_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_rC_Z_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_gamma_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_gamma_beta_g1_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_gamma_beta_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.encoded_IC_query << OUTPUT_NEWLINE;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_processed_verification_key<ppT> &pvk)
{
    in >> pvk.pp_G2_one_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_alphaA_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_alphaB_g1_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_alphaC_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_rC_Z_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_gamma_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_gamma_beta_g1_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_gamma_beta_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.encoded_IC_query;
    libff::consume_OUTPUT_NEWLINE(in);

    return in;
}

template<typename ppT>
bool r1cs_ppzksnark_proof<ppT>::operator==(const r1cs_ppzksnark_proof<ppT> &other) const
{
    return (this->g_A == other.g_A &&
            this->g_B == other.g_B &&
            this->g_C == other.g_C &&
            this->g_H == other.g_H &&
            this->g_K == other.g_K);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_proof<ppT> &proof)
{
    out << proof.g_A << OUTPUT_NEWLINE;
    out << proof.g_B << OUTPUT_NEWLINE;
    out << proof.g_C << OUTPUT_NEWLINE;
    out << proof.g_H << OUTPUT_NEWLINE;
    out << proof.g_K << OUTPUT_NEWLINE;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_proof<ppT> &proof)
{
    in >> proof.g_A;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> proof.g_B;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> proof.g_C;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> proof.g_H;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> proof.g_K;
    libff::consume_OUTPUT_NEWLINE(in);

    return in;
}

template<typename ppT>
r1cs_ppzksnark_verification_key<ppT> r1cs_ppzksnark_verification_key<ppT>::dummy_verification_key(const size_t input_size)
{
    r1cs_ppzksnark_verification_key<ppT> result;
    result.alphaA_g2 = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();
    result.alphaB_g1 = libff::Fr<ppT>::random_element() * libff::G1<ppT>::one();
    result.alphaC_g2 = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();
    result.gamma_g2 = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();
    result.gamma_beta_g1 = libff::Fr<ppT>::random_element() * libff::G1<ppT>::one();
    result.gamma_beta_g2 = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();
    result.rC_Z_g2 = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();

    libff::G1<ppT> base = libff::Fr<ppT>::random_element() * libff::G1<ppT>::one();
    libff::G1_vector<ppT> v;
    for (size_t i = 0; i < input_size; ++i)
    {
        v.emplace_back(libff::Fr<ppT>::random_element() * libff::G1<ppT>::one());
    }

    result.encoded_IC_query = accumulation_vector<libff::G1<ppT> >(std::move(base), std::move(v));

    return result;
}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator(const r1cs_ppzksnark_constraint_system<ppT> &cs)
{
    libff::enter_block("Call to r1cs_ppzksnark_generator");

    /* make the B_query "lighter" if possible */
    r1cs_ppzksnark_constraint_system<ppT> cs_copy(cs);
    cs_copy.swap_AB_if_beneficial();

    /* draw random element at which the QAP is evaluated */
    const  libff::Fr<ppT> t = libff::Fr<ppT>::random_element();

    qap_instance_evaluation<libff::Fr<ppT> > qap_inst = r1cs_to_qap_instance_map_with_evaluation(cs_copy, t);

    libff::print_indent(); printf("* QAP number of variables: %zu\n", qap_inst.num_variables());
    libff::print_indent(); printf("* QAP pre degree: %zu\n", cs_copy.constraints.size());
    libff::print_indent(); printf("* QAP degree: %zu\n", qap_inst.degree());
    libff::print_indent(); printf("* QAP number of input variables: %zu\n", qap_inst.num_inputs());

    libff::enter_block("Compute query densities");
    size_t non_zero_At = 0, non_zero_Bt = 0, non_zero_Ct = 0, non_zero_Ht = 0;
    for (size_t i = 0; i < qap_inst.num_variables()+1; ++i)
    {
        if (!qap_inst.At[i].is_zero())
        {
            ++non_zero_At;
        }
        if (!qap_inst.Bt[i].is_zero())
        {
            ++non_zero_Bt;
        }
        if (!qap_inst.Ct[i].is_zero())
        {
            ++non_zero_Ct;
        }
    }
    for (size_t i = 0; i < qap_inst.degree()+1; ++i)
    {
        if (!qap_inst.Ht[i].is_zero())
        {
            ++non_zero_Ht;
        }
    }
    libff::leave_block("Compute query densities");

    libff::Fr_vector<ppT> At = std::move(qap_inst.At); // qap_inst.At is now in unspecified state, but we do not use it later
    libff::Fr_vector<ppT> Bt = std::move(qap_inst.Bt); // qap_inst.Bt is now in unspecified state, but we do not use it later
    libff::Fr_vector<ppT> Ct = std::move(qap_inst.Ct); // qap_inst.Ct is now in unspecified state, but we do not use it later
    libff::Fr_vector<ppT> Ht = std::move(qap_inst.Ht); // qap_inst.Ht is now in unspecified state, but we do not use it later

    /* append Zt to At,Bt,Ct with */
    At.emplace_back(qap_inst.Zt);
    Bt.emplace_back(qap_inst.Zt);
    Ct.emplace_back(qap_inst.Zt);

    const  libff::Fr<ppT> alphaA = libff::Fr<ppT>::random_element(),
        alphaB = libff::Fr<ppT>::random_element(),
        alphaC = libff::Fr<ppT>::random_element(),
        rA = libff::Fr<ppT>::random_element(),
        rB = libff::Fr<ppT>::random_element(),
        beta = libff::Fr<ppT>::random_element(),
        gamma = libff::Fr<ppT>::random_element();
    const libff::Fr<ppT>      rC = rA * rB;

    // consrtuct the same-coefficient-check query (must happen before zeroing out the prefix of At)
    libff::Fr_vector<ppT> Kt;
    Kt.reserve(qap_inst.num_variables()+4);
    for (size_t i = 0; i < qap_inst.num_variables()+1; ++i)
    {
        Kt.emplace_back( beta * (rA * At[i] + rB * Bt[i] + rC * Ct[i] ) );
    }
    Kt.emplace_back(beta * rA * qap_inst.Zt);
    Kt.emplace_back(beta * rB * qap_inst.Zt);
    Kt.emplace_back(beta * rC * qap_inst.Zt);

    /* zero out prefix of At and stick it into IC coefficients */
    libff::Fr_vector<ppT> IC_coefficients;
    IC_coefficients.reserve(qap_inst.num_inputs() + 1);
    for (size_t i = 0; i < qap_inst.num_inputs() + 1; ++i)
    {
        IC_coefficients.emplace_back(At[i]);
        assert(!IC_coefficients[i].is_zero());
        At[i] = libff::Fr<ppT>::zero();
    }

    const size_t g1_exp_count = 2*(non_zero_At - qap_inst.num_inputs() + non_zero_Ct) + non_zero_Bt + non_zero_Ht + Kt.size();
    const size_t g2_exp_count = non_zero_Bt;

    size_t g1_window = libff::get_exp_window_size<libff::G1<ppT> >(g1_exp_count);
    size_t g2_window = libff::get_exp_window_size<libff::G2<ppT> >(g2_exp_count);
    libff::print_indent(); printf("* G1 window: %zu\n", g1_window);
    libff::print_indent(); printf("* G2 window: %zu\n", g2_window);

#ifdef MULTICORE
    const size_t chunks = omp_get_max_threads(); // to override, set OMP_NUM_THREADS env var or call omp_set_num_threads()
#else
    const size_t chunks = 1;
#endif

    libff::enter_block("Generating G1 multiexp table");
    libff::window_table<libff::G1<ppT> > g1_table = get_window_table(libff::Fr<ppT>::size_in_bits(), g1_window, libff::G1<ppT>::one());
    libff::leave_block("Generating G1 multiexp table");

    libff::enter_block("Generating G2 multiexp table");
    libff::window_table<libff::G2<ppT> > g2_table = get_window_table(libff::Fr<ppT>::size_in_bits(), g2_window, libff::G2<ppT>::one());
    libff::leave_block("Generating G2 multiexp table");

    libff::enter_block("Generate R1CS proving key");

    libff::enter_block("Generate knowledge commitments");
    libff::enter_block("Compute the A-query", false);
    knowledge_commitment_vector<libff::G1<ppT>, libff::G1<ppT> > A_query = kc_batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_window, g1_table, g1_table, rA, rA*alphaA, At, chunks);
    libff::leave_block("Compute the A-query", false);

    libff::enter_block("Compute the B-query", false);
    knowledge_commitment_vector<libff::G2<ppT>, libff::G1<ppT> > B_query = kc_batch_exp(libff::Fr<ppT>::size_in_bits(), g2_window, g1_window, g2_table, g1_table, rB, rB*alphaB, Bt, chunks);
    libff::leave_block("Compute the B-query", false);

    libff::enter_block("Compute the C-query", false);
    knowledge_commitment_vector<libff::G1<ppT>, libff::G1<ppT> > C_query = kc_batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_window, g1_table, g1_table, rC, rC*alphaC, Ct, chunks);
    libff::leave_block("Compute the C-query", false);

    libff::enter_block("Compute the H-query", false);
    libff::G1_vector<ppT> H_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Ht);
    libff::leave_block("Compute the H-query", false);

    libff::enter_block("Compute the K-query", false);
    libff::G1_vector<ppT> K_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Kt);
#ifdef USE_MIXED_ADDITION
    libff::batch_to_special<libff::G1<ppT> >(K_query);
#endif
    libff::leave_block("Compute the K-query", false);

    libff::leave_block("Generate knowledge commitments");

    libff::leave_block("Generate R1CS proving key");

    libff::enter_block("Generate R1CS verification key");
    libff::G2<ppT> alphaA_g2 = alphaA * libff::G2<ppT>::one();
    libff::G1<ppT> alphaB_g1 = alphaB * libff::G1<ppT>::one();
    libff::G2<ppT> alphaC_g2 = alphaC * libff::G2<ppT>::one();
    libff::G2<ppT> gamma_g2 = gamma * libff::G2<ppT>::one();
    libff::G1<ppT> gamma_beta_g1 = (gamma * beta) * libff::G1<ppT>::one();
    libff::G2<ppT> gamma_beta_g2 = (gamma * beta) * libff::G2<ppT>::one();
    libff::G2<ppT> rC_Z_g2 = (rC * qap_inst.Zt) * libff::G2<ppT>::one();

    libff::enter_block("Encode IC query for R1CS verification key");
    libff::G1<ppT> encoded_IC_base = (rA * IC_coefficients[0]) * libff::G1<ppT>::one();
    libff::Fr_vector<ppT> multiplied_IC_coefficients;
    multiplied_IC_coefficients.reserve(qap_inst.num_inputs());
    for (size_t i = 1; i < qap_inst.num_inputs() + 1; ++i)
    {
        multiplied_IC_coefficients.emplace_back(rA * IC_coefficients[i]);
    }
    libff::G1_vector<ppT> encoded_IC_values = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, multiplied_IC_coefficients);

    libff::leave_block("Encode IC query for R1CS verification key");
    libff::leave_block("Generate R1CS verification key");

    libff::leave_block("Call to r1cs_ppzksnark_generator");

    accumulation_vector<libff::G1<ppT> > encoded_IC_query(std::move(encoded_IC_base), std::move(encoded_IC_values));

    r1cs_ppzksnark_verification_key<ppT> vk = r1cs_ppzksnark_verification_key<ppT>(alphaA_g2,
                                                                                   alphaB_g1,
                                                                                   alphaC_g2,
                                                                                   gamma_g2,
                                                                                   gamma_beta_g1,
                                                                                   gamma_beta_g2,
                                                                                   rC_Z_g2,
                                                                                   encoded_IC_query);
    r1cs_ppzksnark_proving_key<ppT> pk = r1cs_ppzksnark_proving_key<ppT>(std::move(A_query),
                                                                         std::move(B_query),
                                                                         std::move(C_query),
                                                                         std::move(H_query),
                                                                         std::move(K_query),
                                                                         std::move(cs_copy));

    pk.print_size();
    vk.print_size();

    return r1cs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk));
}

template <typename ppT>
r1cs_ppzksnark_proof<ppT> r1cs_ppzksnark_prover(const r1cs_ppzksnark_proving_key<ppT> &pk,
                                                const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                                const r1cs_ppzksnark_auxiliary_input<ppT> &auxiliary_input)
{
    libff::enter_block("Call to r1cs_ppzksnark_prover");

#ifdef DEBUG
    assert(pk.constraint_system.is_satisfied(primary_input, auxiliary_input));
#endif

    const libff::Fr<ppT> d1 = libff::Fr<ppT>::random_element(),
        d2 = libff::Fr<ppT>::random_element(),
        d3 = libff::Fr<ppT>::random_element();

    libff::enter_block("Compute the polynomial H");
    const qap_witness<libff::Fr<ppT> > qap_wit = r1cs_to_qap_witness_map(pk.constraint_system, primary_input, auxiliary_input, d1, d2, d3);
    libff::leave_block("Compute the polynomial H");

#ifdef DEBUG
    const libff::Fr<ppT> t = libff::Fr<ppT>::random_element();
    qap_instance_evaluation<libff::Fr<ppT> > qap_inst = r1cs_to_qap_instance_map_with_evaluation(pk.constraint_system, t);
    assert(qap_inst.is_satisfied(qap_wit));
#endif

    knowledge_commitment<libff::G1<ppT>, libff::G1<ppT> > g_A = pk.A_query[0] + qap_wit.d1*pk.A_query[qap_wit.num_variables()+1];
    knowledge_commitment<libff::G2<ppT>, libff::G1<ppT> > g_B = pk.B_query[0] + qap_wit.d2*pk.B_query[qap_wit.num_variables()+1];
    knowledge_commitment<libff::G1<ppT>, libff::G1<ppT> > g_C = pk.C_query[0] + qap_wit.d3*pk.C_query[qap_wit.num_variables()+1];

    libff::G1<ppT> g_H = libff::G1<ppT>::zero();
    libff::G1<ppT> g_K = (pk.K_query[0] +
                   qap_wit.d1*pk.K_query[qap_wit.num_variables()+1] +
                   qap_wit.d2*pk.K_query[qap_wit.num_variables()+2] +
                   qap_wit.d3*pk.K_query[qap_wit.num_variables()+3]);

#ifdef DEBUG
    for (size_t i = 0; i < qap_wit.num_inputs() + 1; ++i)
    {
        assert(pk.A_query[i].g == libff::G1<ppT>::zero());
    }
    assert(pk.A_query.domain_size() == qap_wit.num_variables()+2);
    assert(pk.B_query.domain_size() == qap_wit.num_variables()+2);
    assert(pk.C_query.domain_size() == qap_wit.num_variables()+2);
    assert(pk.H_query.size() == qap_wit.degree()+1);
    assert(pk.K_query.size() == qap_wit.num_variables()+4);
#endif

#ifdef MULTICORE
    const size_t chunks = omp_get_max_threads(); // to override, set OMP_NUM_THREADS env var or call omp_set_num_threads()
#else
    const size_t chunks = 1;
#endif

    libff::enter_block("Compute the proof");

    libff::enter_block("Compute answer to A-query", false);
    g_A = g_A + kc_multi_exp_with_mixed_addition<libff::G1<ppT>, libff::G1<ppT>, libff::Fr<ppT> >(pk.A_query,
                                                                             1, 1+qap_wit.num_variables(),
                                                                             qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                             chunks, true);
    libff::leave_block("Compute answer to A-query", false);

    libff::enter_block("Compute answer to B-query", false);
    g_B = g_B + kc_multi_exp_with_mixed_addition<libff::G2<ppT>, libff::G1<ppT>, libff::Fr<ppT> >(pk.B_query,
                                                                             1, 1+qap_wit.num_variables(),
                                                                             qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                             chunks, true);
    libff::leave_block("Compute answer to B-query", false);

    libff::enter_block("Compute answer to C-query", false);
    g_C = g_C + kc_multi_exp_with_mixed_addition<libff::G1<ppT>, libff::G1<ppT>, libff::Fr<ppT> >(pk.C_query,
                                                                             1, 1+qap_wit.num_variables(),
                                                                             qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                             chunks, true);
    libff::leave_block("Compute answer to C-query", false);

    libff::enter_block("Compute answer to H-query", false);
    g_H = g_H + libff::multi_exp<libff::G1<ppT>, libff::Fr<ppT> >(pk.H_query.begin(), pk.H_query.begin()+qap_wit.degree()+1,
                                             qap_wit.coefficients_for_H.begin(), qap_wit.coefficients_for_H.begin()+qap_wit.degree()+1,
                                             chunks, true);
    libff::leave_block("Compute answer to H-query", false);

    libff::enter_block("Compute answer to K-query", false);
    g_K = g_K + libff::multi_exp_with_mixed_addition<libff::G1<ppT>, libff::Fr<ppT> >(pk.K_query.begin()+1, pk.K_query.begin()+1+qap_wit.num_variables(),
                                                                 qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                 chunks, true);
    libff::leave_block("Compute answer to K-query", false);

    libff::leave_block("Compute the proof");

    libff::leave_block("Call to r1cs_ppzksnark_prover");

    r1cs_ppzksnark_proof<ppT> proof = r1cs_ppzksnark_proof<ppT>(std::move(g_A), std::move(g_B), std::move(g_C), std::move(g_H), std::move(g_K));
    proof.print_size();

    return proof;
}

template <typename ppT>
r1cs_ppzksnark_processed_verification_key<ppT> r1cs_ppzksnark_verifier_process_vk(const r1cs_ppzksnark_verification_key<ppT> &vk)
{
    libff::enter_block("Call to r1cs_ppzksnark_verifier_process_vk");

    r1cs_ppzksnark_processed_verification_key<ppT> pvk;
    pvk.pp_G2_one_precomp        = ppT::precompute_G2(libff::G2<ppT>::one());
    pvk.vk_alphaA_g2_precomp     = ppT::precompute_G2(vk.alphaA_g2);
    pvk.vk_alphaB_g1_precomp     = ppT::precompute_G1(vk.alphaB_g1);
    pvk.vk_alphaC_g2_precomp     = ppT::precompute_G2(vk.alphaC_g2);
    pvk.vk_rC_Z_g2_precomp       = ppT::precompute_G2(vk.rC_Z_g2);
    pvk.vk_gamma_g2_precomp      = ppT::precompute_G2(vk.gamma_g2);
    pvk.vk_gamma_beta_g1_precomp = ppT::precompute_G1(vk.gamma_beta_g1);
    pvk.vk_gamma_beta_g2_precomp = ppT::precompute_G2(vk.gamma_beta_g2);

    pvk.encoded_IC_query = vk.encoded_IC_query;

    libff::leave_block("Call to r1cs_ppzksnark_verifier_process_vk");

    return pvk;
}

template <typename ppT>
bool r1cs_ppzksnark_online_verifier_weak_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                            const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    libff::enter_block("Call to r1cs_ppzksnark_online_verifier_weak_IC");
    assert(pvk.encoded_IC_query.domain_size() >= primary_input.size());

    libff::enter_block("Compute input-dependent part of A");
    const accumulation_vector<libff::G1<ppT> > accumulated_IC = pvk.encoded_IC_query.template accumulate_chunk<libff::Fr<ppT> >(primary_input.begin(), primary_input.end(), 0);
    const libff::G1<ppT> &acc = accumulated_IC.first;
    libff::leave_block("Compute input-dependent part of A");

    bool result = true;

    libff::enter_block("Check if the proof is well-formed");
    if (!proof.is_well_formed())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("At least one of the proof elements does not lie on the curve.\n");
        }
        result = false;
    }
    libff::leave_block("Check if the proof is well-formed");

    libff::enter_block("Online pairing computations");
    libff::enter_block("Check knowledge commitment for A is valid");
    libff::G1_precomp<ppT> proof_g_A_g_precomp      = ppT::precompute_G1(proof.g_A.g);
    libff::G1_precomp<ppT> proof_g_A_h_precomp = ppT::precompute_G1(proof.g_A.h);
    libff::Fqk<ppT> kc_A_1 = ppT::miller_loop(proof_g_A_g_precomp,      pvk.vk_alphaA_g2_precomp);
    libff::Fqk<ppT> kc_A_2 = ppT::miller_loop(proof_g_A_h_precomp, pvk.pp_G2_one_precomp);
    libff::GT<ppT> kc_A = ppT::final_exponentiation(kc_A_1 * kc_A_2.unitary_inverse());
    if (kc_A != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("Knowledge commitment for A query incorrect.\n");
        }
        result = false;
    }
    libff::leave_block("Check knowledge commitment for A is valid");

    libff::enter_block("Check knowledge commitment for B is valid");
    libff::G2_precomp<ppT> proof_g_B_g_precomp      = ppT::precompute_G2(proof.g_B.g);
    libff::G1_precomp<ppT> proof_g_B_h_precomp = ppT::precompute_G1(proof.g_B.h);
    libff::Fqk<ppT> kc_B_1 = ppT::miller_loop(pvk.vk_alphaB_g1_precomp, proof_g_B_g_precomp);
    libff::Fqk<ppT> kc_B_2 = ppT::miller_loop(proof_g_B_h_precomp,    pvk.pp_G2_one_precomp);
    libff::GT<ppT> kc_B = ppT::final_exponentiation(kc_B_1 * kc_B_2.unitary_inverse());
    if (kc_B != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("Knowledge commitment for B query incorrect.\n");
        }
        result = false;
    }
    libff::leave_block("Check knowledge commitment for B is valid");

    libff::enter_block("Check knowledge commitment for C is valid");
    libff::G1_precomp<ppT> proof_g_C_g_precomp      = ppT::precompute_G1(proof.g_C.g);
    libff::G1_precomp<ppT> proof_g_C_h_precomp = ppT::precompute_G1(proof.g_C.h);
    libff::Fqk<ppT> kc_C_1 = ppT::miller_loop(proof_g_C_g_precomp,      pvk.vk_alphaC_g2_precomp);
    libff::Fqk<ppT> kc_C_2 = ppT::miller_loop(proof_g_C_h_precomp, pvk.pp_G2_one_precomp);
    libff::GT<ppT> kc_C = ppT::final_exponentiation(kc_C_1 * kc_C_2.unitary_inverse());
    if (kc_C != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("Knowledge commitment for C query incorrect.\n");
        }
        result = false;
    }
    libff::leave_block("Check knowledge commitment for C is valid");

    libff::enter_block("Check QAP divisibility");
    // check that g^((A+acc)*B)=g^(H*\Prod(t-\sigma)+C)
    // equivalently, via pairings, that e(g^(A+acc), g^B) = e(g^H, g^Z) + e(g^C, g^1)
    libff::G1_precomp<ppT> proof_g_A_g_acc_precomp = ppT::precompute_G1(proof.g_A.g + acc);
    libff::G1_precomp<ppT> proof_g_H_precomp       = ppT::precompute_G1(proof.g_H);
    libff::Fqk<ppT> QAP_1  = ppT::miller_loop(proof_g_A_g_acc_precomp,  proof_g_B_g_precomp);
    libff::Fqk<ppT> QAP_23  = ppT::double_miller_loop(proof_g_H_precomp, pvk.vk_rC_Z_g2_precomp, proof_g_C_g_precomp, pvk.pp_G2_one_precomp);
    libff::GT<ppT> QAP = ppT::final_exponentiation(QAP_1 * QAP_23.unitary_inverse());
    if (QAP != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("QAP divisibility check failed.\n");
        }
        result = false;
    }
    libff::leave_block("Check QAP divisibility");

    libff::enter_block("Check same coefficients were used");
    libff::G1_precomp<ppT> proof_g_K_precomp = ppT::precompute_G1(proof.g_K);
    libff::G1_precomp<ppT> proof_g_A_g_acc_C_precomp = ppT::precompute_G1((proof.g_A.g + acc) + proof.g_C.g);
    libff::Fqk<ppT> K_1 = ppT::miller_loop(proof_g_K_precomp, pvk.vk_gamma_g2_precomp);
    libff::Fqk<ppT> K_23 = ppT::double_miller_loop(proof_g_A_g_acc_C_precomp, pvk.vk_gamma_beta_g2_precomp, pvk.vk_gamma_beta_g1_precomp, proof_g_B_g_precomp);
    libff::GT<ppT> K = ppT::final_exponentiation(K_1 * K_23.unitary_inverse());
    if (K != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("Same-coefficient check failed.\n");
        }
        result = false;
    }
    libff::leave_block("Check same coefficients were used");
    libff::leave_block("Online pairing computations");
    libff::leave_block("Call to r1cs_ppzksnark_online_verifier_weak_IC");

    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_verifier_weak_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                     const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                     const r1cs_ppzksnark_proof<ppT> &proof)
{
    libff::enter_block("Call to r1cs_ppzksnark_verifier_weak_IC");
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = r1cs_ppzksnark_online_verifier_weak_IC<ppT>(pvk, primary_input, proof);
    libff::leave_block("Call to r1cs_ppzksnark_verifier_weak_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_online_verifier_strong_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                              const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                              const r1cs_ppzksnark_proof<ppT> &proof)
{
    bool result = true;
    libff::enter_block("Call to r1cs_ppzksnark_online_verifier_strong_IC");

    if (pvk.encoded_IC_query.domain_size() != primary_input.size())
    {
        libff::print_indent(); printf("Input length differs from expected (got %zu, expected %zu).\n", primary_input.size(), pvk.encoded_IC_query.domain_size());
        result = false;
    }
    else
    {
        result = r1cs_ppzksnark_online_verifier_weak_IC(pvk, primary_input, proof);
    }

    libff::leave_block("Call to r1cs_ppzksnark_online_verifier_strong_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_verifier_strong_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                       const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                       const r1cs_ppzksnark_proof<ppT> &proof)
{
    libff::enter_block("Call to r1cs_ppzksnark_verifier_strong_IC");
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = r1cs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, primary_input, proof);
    libff::leave_block("Call to r1cs_ppzksnark_verifier_strong_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_affine_verifier_weak_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                            const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    libff::enter_block("Call to r1cs_ppzksnark_affine_verifier_weak_IC");
    assert(vk.encoded_IC_query.domain_size() >= primary_input.size());

    libff::affine_ate_G2_precomp<ppT> pvk_pp_G2_one_precomp        = ppT::affine_ate_precompute_G2(libff::G2<ppT>::one());
    libff::affine_ate_G2_precomp<ppT> pvk_vk_alphaA_g2_precomp     = ppT::affine_ate_precompute_G2(vk.alphaA_g2);
    libff::affine_ate_G1_precomp<ppT> pvk_vk_alphaB_g1_precomp     = ppT::affine_ate_precompute_G1(vk.alphaB_g1);
    libff::affine_ate_G2_precomp<ppT> pvk_vk_alphaC_g2_precomp     = ppT::affine_ate_precompute_G2(vk.alphaC_g2);
    libff::affine_ate_G2_precomp<ppT> pvk_vk_rC_Z_g2_precomp       = ppT::affine_ate_precompute_G2(vk.rC_Z_g2);
    libff::affine_ate_G2_precomp<ppT> pvk_vk_gamma_g2_precomp      = ppT::affine_ate_precompute_G2(vk.gamma_g2);
    libff::affine_ate_G1_precomp<ppT> pvk_vk_gamma_beta_g1_precomp = ppT::affine_ate_precompute_G1(vk.gamma_beta_g1);
    libff::affine_ate_G2_precomp<ppT> pvk_vk_gamma_beta_g2_precomp = ppT::affine_ate_precompute_G2(vk.gamma_beta_g2);

    libff::enter_block("Compute input-dependent part of A");
    const accumulation_vector<libff::G1<ppT> > accumulated_IC = vk.encoded_IC_query.template accumulate_chunk<libff::Fr<ppT> >(primary_input.begin(), primary_input.end(), 0);
    assert(accumulated_IC.is_fully_accumulated());
    const libff::G1<ppT> &acc = accumulated_IC.first;
    libff::leave_block("Compute input-dependent part of A");

    bool result = true;
    libff::enter_block("Check knowledge commitment for A is valid");
    libff::affine_ate_G1_precomp<ppT> proof_g_A_g_precomp = ppT::affine_ate_precompute_G1(proof.g_A.g);
    libff::affine_ate_G1_precomp<ppT> proof_g_A_h_precomp = ppT::affine_ate_precompute_G1(proof.g_A.h);
    libff::Fqk<ppT> kc_A_miller = ppT::affine_ate_e_over_e_miller_loop(proof_g_A_g_precomp, pvk_vk_alphaA_g2_precomp, proof_g_A_h_precomp, pvk_pp_G2_one_precomp);
    libff::GT<ppT> kc_A = ppT::final_exponentiation(kc_A_miller);

    if (kc_A != libff::GT<ppT>::one())
    {
        libff::print_indent(); printf("Knowledge commitment for A query incorrect.\n");
        result = false;
    }
    libff::leave_block("Check knowledge commitment for A is valid");

    libff::enter_block("Check knowledge commitment for B is valid");
    libff::affine_ate_G2_precomp<ppT> proof_g_B_g_precomp = ppT::affine_ate_precompute_G2(proof.g_B.g);
    libff::affine_ate_G1_precomp<ppT> proof_g_B_h_precomp = ppT::affine_ate_precompute_G1(proof.g_B.h);
    libff::Fqk<ppT> kc_B_miller = ppT::affine_ate_e_over_e_miller_loop(pvk_vk_alphaB_g1_precomp, proof_g_B_g_precomp, proof_g_B_h_precomp,    pvk_pp_G2_one_precomp);
    libff::GT<ppT> kc_B = ppT::final_exponentiation(kc_B_miller);
    if (kc_B != libff::GT<ppT>::one())
    {
        libff::print_indent(); printf("Knowledge commitment for B query incorrect.\n");
        result = false;
    }
    libff::leave_block("Check knowledge commitment for B is valid");

    libff::enter_block("Check knowledge commitment for C is valid");
    libff::affine_ate_G1_precomp<ppT> proof_g_C_g_precomp = ppT::affine_ate_precompute_G1(proof.g_C.g);
    libff::affine_ate_G1_precomp<ppT> proof_g_C_h_precomp = ppT::affine_ate_precompute_G1(proof.g_C.h);
    libff::Fqk<ppT> kc_C_miller = ppT::affine_ate_e_over_e_miller_loop(proof_g_C_g_precomp, pvk_vk_alphaC_g2_precomp, proof_g_C_h_precomp, pvk_pp_G2_one_precomp);
    libff::GT<ppT> kc_C = ppT::final_exponentiation(kc_C_miller);
    if (kc_C != libff::GT<ppT>::one())
    {
        libff::print_indent(); printf("Knowledge commitment for C query incorrect.\n");
        result = false;
    }
    libff::leave_block("Check knowledge commitment for C is valid");

    libff::enter_block("Check QAP divisibility");
    libff::affine_ate_G1_precomp<ppT> proof_g_A_g_acc_precomp = ppT::affine_ate_precompute_G1(proof.g_A.g + acc);
    libff::affine_ate_G1_precomp<ppT> proof_g_H_precomp       = ppT::affine_ate_precompute_G1(proof.g_H);
    libff::Fqk<ppT> QAP_miller  = ppT::affine_ate_e_times_e_over_e_miller_loop(proof_g_H_precomp, pvk_vk_rC_Z_g2_precomp, proof_g_C_g_precomp, pvk_pp_G2_one_precomp, proof_g_A_g_acc_precomp,  proof_g_B_g_precomp);
    libff::GT<ppT> QAP = ppT::final_exponentiation(QAP_miller);
    if (QAP != libff::GT<ppT>::one())
    {
        libff::print_indent(); printf("QAP divisibility check failed.\n");
        result = false;
    }
    libff::leave_block("Check QAP divisibility");

    libff::enter_block("Check same coefficients were used");
    libff::affine_ate_G1_precomp<ppT> proof_g_K_precomp = ppT::affine_ate_precompute_G1(proof.g_K);
    libff::affine_ate_G1_precomp<ppT> proof_g_A_g_acc_C_precomp = ppT::affine_ate_precompute_G1((proof.g_A.g + acc) + proof.g_C.g);
    libff::Fqk<ppT> K_miller = ppT::affine_ate_e_times_e_over_e_miller_loop(proof_g_A_g_acc_C_precomp, pvk_vk_gamma_beta_g2_precomp, pvk_vk_gamma_beta_g1_precomp, proof_g_B_g_precomp, proof_g_K_precomp, pvk_vk_gamma_g2_precomp);
    libff::GT<ppT> K = ppT::final_exponentiation(K_miller);
    if (K != libff::GT<ppT>::one())
    {
        libff::print_indent(); printf("Same-coefficient check failed.\n");
        result = false;
    }
    libff::leave_block("Check same coefficients were used");

    libff::leave_block("Call to r1cs_ppzksnark_affine_verifier_weak_IC");

    return result;
}

} // libsnark
#endif // R1CS_PPZKSNARK_TCC_
