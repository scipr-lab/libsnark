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

#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "algebra/scalar_multiplication/multiexp.hpp"
#include "algebra/scalar_multiplication/kc_multiexp.hpp"
#include "reductions/r1cs_to_qap/r1cs_to_qap.hpp"
#include "algebra/curves/alt_bn128/alt_bn128_pp.hpp"
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
    consume_OUTPUT_NEWLINE(in);
    in >> vk.alphaB_g1;
    consume_OUTPUT_NEWLINE(in);
    in >> vk.alphaC_g2;
    consume_OUTPUT_NEWLINE(in);
    in >> vk.gamma_g2;
    consume_OUTPUT_NEWLINE(in);
    in >> vk.gamma_beta_g1;
    consume_OUTPUT_NEWLINE(in);
    in >> vk.gamma_beta_g2;
    consume_OUTPUT_NEWLINE(in);
    in >> vk.rC_Z_g2;
    consume_OUTPUT_NEWLINE(in);
    in >> vk.encoded_IC_query;
    consume_OUTPUT_NEWLINE(in);

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
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_alphaA_g2_precomp;
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_alphaB_g1_precomp;
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_alphaC_g2_precomp;
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_rC_Z_g2_precomp;
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_gamma_g2_precomp;
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_gamma_beta_g1_precomp;
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_gamma_beta_g2_precomp;
    consume_OUTPUT_NEWLINE(in);
    in >> pvk.encoded_IC_query;
    consume_OUTPUT_NEWLINE(in);

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
    consume_OUTPUT_NEWLINE(in);
    in >> proof.g_B;
    consume_OUTPUT_NEWLINE(in);
    in >> proof.g_C;
    consume_OUTPUT_NEWLINE(in);
    in >> proof.g_H;
    consume_OUTPUT_NEWLINE(in);
    in >> proof.g_K;
    consume_OUTPUT_NEWLINE(in);

    return in;
}

template<typename ppT>
r1cs_ppzksnark_verification_key<ppT> r1cs_ppzksnark_verification_key<ppT>::dummy_verification_key(const size_t input_size)
{
    r1cs_ppzksnark_verification_key<ppT> result;
    result.alphaA_g2 = Fr<ppT>::random_element() * G2<ppT>::one();
    result.alphaB_g1 = Fr<ppT>::random_element() * G1<ppT>::one();
    result.alphaC_g2 = Fr<ppT>::random_element() * G2<ppT>::one();
    result.gamma_g2 = Fr<ppT>::random_element() * G2<ppT>::one();
    result.gamma_beta_g1 = Fr<ppT>::random_element() * G1<ppT>::one();
    result.gamma_beta_g2 = Fr<ppT>::random_element() * G2<ppT>::one();
    result.rC_Z_g2 = Fr<ppT>::random_element() * G2<ppT>::one();

    G1<ppT> base = Fr<ppT>::random_element() * G1<ppT>::one();
    G1_vector<ppT> v;
    for (size_t i = 0; i < input_size; ++i)
    {
        v.emplace_back(Fr<ppT>::random_element() * G1<ppT>::one());
    }

    result.encoded_IC_query = accumulation_vector<G1<ppT> >(std::move(base), std::move(v));

    return result;
}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator(const r1cs_ppzksnark_constraint_system<ppT> &cs)
{
    enter_block("Call to r1cs_ppzksnark_generator");

    /* make the B_query "lighter" if possible */
    r1cs_ppzksnark_constraint_system<ppT> cs_copy(cs);
    cs_copy.swap_AB_if_beneficial();

    /* draw random element at which the QAP is evaluated */
    const  Fr<ppT> t = Fr<ppT>::random_element();

    qap_instance_evaluation<Fr<ppT> > qap_inst = r1cs_to_qap_instance_map_with_evaluation(cs_copy, t);

    print_indent(); printf("* QAP number of variables: %zu\n", qap_inst.num_variables());
    print_indent(); printf("* QAP pre degree: %zu\n", cs_copy.constraints.size());
    print_indent(); printf("* QAP degree: %zu\n", qap_inst.degree());
    print_indent(); printf("* QAP number of input variables: %zu\n", qap_inst.num_inputs());

    enter_block("Compute query densities");
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
    leave_block("Compute query densities");

    Fr_vector<ppT> At = std::move(qap_inst.At); // qap_inst.At is now in unspecified state, but we do not use it later
    Fr_vector<ppT> Bt = std::move(qap_inst.Bt); // qap_inst.Bt is now in unspecified state, but we do not use it later
    Fr_vector<ppT> Ct = std::move(qap_inst.Ct); // qap_inst.Ct is now in unspecified state, but we do not use it later
    Fr_vector<ppT> Ht = std::move(qap_inst.Ht); // qap_inst.Ht is now in unspecified state, but we do not use it later

    /* append Zt to At,Bt,Ct with */
    At.emplace_back(qap_inst.Zt);
    Bt.emplace_back(qap_inst.Zt);
    Ct.emplace_back(qap_inst.Zt);

    const  Fr<ppT> alphaA = Fr<ppT>::random_element(),
        alphaB = Fr<ppT>::random_element(),
        alphaC = Fr<ppT>::random_element(),
        rA = Fr<ppT>::random_element(),
        rB = Fr<ppT>::random_element(),
        beta = Fr<ppT>::random_element(),
        gamma = Fr<ppT>::random_element();
    const Fr<ppT>      rC = rA * rB;

    // consrtuct the same-coefficient-check query (must happen before zeroing out the prefix of At)
    Fr_vector<ppT> Kt;
    Kt.reserve(qap_inst.num_variables()+4);
    for (size_t i = 0; i < qap_inst.num_variables()+1; ++i)
    {
        Kt.emplace_back( beta * (rA * At[i] + rB * Bt[i] + rC * Ct[i] ) );
    }
    Kt.emplace_back(beta * rA * qap_inst.Zt);
    Kt.emplace_back(beta * rB * qap_inst.Zt);
    Kt.emplace_back(beta * rC * qap_inst.Zt);

    /* zero out prefix of At and stick it into IC coefficients */
    Fr_vector<ppT> IC_coefficients;
    IC_coefficients.reserve(qap_inst.num_inputs() + 1);
    for (size_t i = 0; i < qap_inst.num_inputs() + 1; ++i)
    {
        IC_coefficients.emplace_back(At[i]);
        assert(!IC_coefficients[i].is_zero());
        At[i] = Fr<ppT>::zero();
    }

    const size_t g1_exp_count = 2*(non_zero_At - qap_inst.num_inputs() + non_zero_Ct) + non_zero_Bt + non_zero_Ht + Kt.size();
    const size_t g2_exp_count = non_zero_Bt;

    size_t g1_window = get_exp_window_size<G1<ppT> >(g1_exp_count);
    size_t g2_window = get_exp_window_size<G2<ppT> >(g2_exp_count);
    print_indent(); printf("* G1 window: %zu\n", g1_window);
    print_indent(); printf("* G2 window: %zu\n", g2_window);

#ifdef MULTICORE
    const size_t chunks = omp_get_max_threads(); // to override, set OMP_NUM_THREADS env var or call omp_set_num_threads()
#else
    const size_t chunks = 1;
#endif

    enter_block("Generating G1 multiexp table");
    window_table<G1<ppT> > g1_table = get_window_table(Fr<ppT>::size_in_bits(), g1_window, G1<ppT>::one());
    leave_block("Generating G1 multiexp table");

    enter_block("Generating G2 multiexp table");
    window_table<G2<ppT> > g2_table = get_window_table(Fr<ppT>::size_in_bits(), g2_window, G2<ppT>::one());
    leave_block("Generating G2 multiexp table");

    enter_block("Generate R1CS proving key");

    enter_block("Generate knowledge commitments");
    enter_block("Compute the A-query", false);
    knowledge_commitment_vector<G1<ppT>, G1<ppT> > A_query = kc_batch_exp(Fr<ppT>::size_in_bits(), g1_window, g1_window, g1_table, g1_table, rA, rA*alphaA, At, chunks);
    leave_block("Compute the A-query", false);

    enter_block("Compute the B-query", false);
    knowledge_commitment_vector<G2<ppT>, G1<ppT> > B_query = kc_batch_exp(Fr<ppT>::size_in_bits(), g2_window, g1_window, g2_table, g1_table, rB, rB*alphaB, Bt, chunks);
    leave_block("Compute the B-query", false);

    enter_block("Compute the C-query", false);
    knowledge_commitment_vector<G1<ppT>, G1<ppT> > C_query = kc_batch_exp(Fr<ppT>::size_in_bits(), g1_window, g1_window, g1_table, g1_table, rC, rC*alphaC, Ct, chunks);
    leave_block("Compute the C-query", false);

    enter_block("Compute the H-query", false);
    G1_vector<ppT> H_query = batch_exp(Fr<ppT>::size_in_bits(), g1_window, g1_table, Ht);
    leave_block("Compute the H-query", false);

    enter_block("Compute the K-query", false);
    G1_vector<ppT> K_query = batch_exp(Fr<ppT>::size_in_bits(), g1_window, g1_table, Kt);
#ifdef USE_MIXED_ADDITION
    batch_to_special<G1<ppT> >(K_query);
#endif
    leave_block("Compute the K-query", false);

    leave_block("Generate knowledge commitments");

    leave_block("Generate R1CS proving key");

    enter_block("Generate R1CS verification key");
    G2<ppT> alphaA_g2 = alphaA * G2<ppT>::one();
    G1<ppT> alphaB_g1 = alphaB * G1<ppT>::one();
    G2<ppT> alphaC_g2 = alphaC * G2<ppT>::one();
    G2<ppT> gamma_g2 = gamma * G2<ppT>::one();
    G1<ppT> gamma_beta_g1 = (gamma * beta) * G1<ppT>::one();
    G2<ppT> gamma_beta_g2 = (gamma * beta) * G2<ppT>::one();
    G2<ppT> rC_Z_g2 = (rC * qap_inst.Zt) * G2<ppT>::one();

    enter_block("Encode IC query for R1CS verification key");
    G1<ppT> encoded_IC_base = (rA * IC_coefficients[0]) * G1<ppT>::one();
    Fr_vector<ppT> multiplied_IC_coefficients;
    multiplied_IC_coefficients.reserve(qap_inst.num_inputs());
    for (size_t i = 1; i < qap_inst.num_inputs() + 1; ++i)
    {
        multiplied_IC_coefficients.emplace_back(rA * IC_coefficients[i]);
    }
    G1_vector<ppT> encoded_IC_values = batch_exp(Fr<ppT>::size_in_bits(), g1_window, g1_table, multiplied_IC_coefficients);

    leave_block("Encode IC query for R1CS verification key");
    leave_block("Generate R1CS verification key");

    leave_block("Call to r1cs_ppzksnark_generator");

    accumulation_vector<G1<ppT> > encoded_IC_query(std::move(encoded_IC_base), std::move(encoded_IC_values));

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
    enter_block("Call to r1cs_ppzksnark_prover");

#ifdef DEBUG
    assert(pk.constraint_system.is_satisfied(primary_input, auxiliary_input));
#endif

    const Fr<ppT> d1 = Fr<ppT>::random_element(),
        d2 = Fr<ppT>::random_element(),
        d3 = Fr<ppT>::random_element();

    enter_block("Compute the polynomial H");
    const qap_witness<Fr<ppT> > qap_wit = r1cs_to_qap_witness_map(pk.constraint_system, primary_input, auxiliary_input, d1, d2, d3);
    leave_block("Compute the polynomial H");

#ifdef DEBUG
    const Fr<ppT> t = Fr<ppT>::random_element();
    qap_instance_evaluation<Fr<ppT> > qap_inst = r1cs_to_qap_instance_map_with_evaluation(pk.constraint_system, t);
    assert(qap_inst.is_satisfied(qap_wit));
#endif

    knowledge_commitment<G1<ppT>, G1<ppT> > g_A = pk.A_query[0] + qap_wit.d1*pk.A_query[qap_wit.num_variables()+1];
    knowledge_commitment<G2<ppT>, G1<ppT> > g_B = pk.B_query[0] + qap_wit.d2*pk.B_query[qap_wit.num_variables()+1];
    knowledge_commitment<G1<ppT>, G1<ppT> > g_C = pk.C_query[0] + qap_wit.d3*pk.C_query[qap_wit.num_variables()+1];

    G1<ppT> g_H = G1<ppT>::zero();
    G1<ppT> g_K = (pk.K_query[0] +
                   qap_wit.d1*pk.K_query[qap_wit.num_variables()+1] +
                   qap_wit.d2*pk.K_query[qap_wit.num_variables()+2] +
                   qap_wit.d3*pk.K_query[qap_wit.num_variables()+3]);

#ifdef DEBUG
    for (size_t i = 0; i < qap_wit.num_inputs() + 1; ++i)
    {
        assert(pk.A_query[i].g == G1<ppT>::zero());
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

    enter_block("Compute the proof");

    enter_block("Compute answer to A-query", false);
    g_A = g_A + kc_multi_exp_with_mixed_addition<G1<ppT>, G1<ppT>, Fr<ppT> >(pk.A_query,
                                                                             1, 1+qap_wit.num_variables(),
                                                                             qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                             chunks, true);
    leave_block("Compute answer to A-query", false);

    enter_block("Compute answer to B-query", false);
    g_B = g_B + kc_multi_exp_with_mixed_addition<G2<ppT>, G1<ppT>, Fr<ppT> >(pk.B_query,
                                                                             1, 1+qap_wit.num_variables(),
                                                                             qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                             chunks, true);
    leave_block("Compute answer to B-query", false);

    enter_block("Compute answer to C-query", false);
    g_C = g_C + kc_multi_exp_with_mixed_addition<G1<ppT>, G1<ppT>, Fr<ppT> >(pk.C_query,
                                                                             1, 1+qap_wit.num_variables(),
                                                                             qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                             chunks, true);
    leave_block("Compute answer to C-query", false);

    enter_block("Compute answer to H-query", false);
    g_H = g_H + multi_exp<G1<ppT>, Fr<ppT> >(pk.H_query.begin(), pk.H_query.begin()+qap_wit.degree()+1,
                                             qap_wit.coefficients_for_H.begin(), qap_wit.coefficients_for_H.begin()+qap_wit.degree()+1,
                                             chunks, true);
    leave_block("Compute answer to H-query", false);

    enter_block("Compute answer to K-query", false);
    g_K = g_K + multi_exp_with_mixed_addition<G1<ppT>, Fr<ppT> >(pk.K_query.begin()+1, pk.K_query.begin()+1+qap_wit.num_variables(),
                                                                 qap_wit.coefficients_for_ABCs.begin(), qap_wit.coefficients_for_ABCs.begin()+qap_wit.num_variables(),
                                                                 chunks, true);
    leave_block("Compute answer to K-query", false);

    leave_block("Compute the proof");

    leave_block("Call to r1cs_ppzksnark_prover");

    r1cs_ppzksnark_proof<ppT> proof = r1cs_ppzksnark_proof<ppT>(std::move(g_A), std::move(g_B), std::move(g_C), std::move(g_H), std::move(g_K));
    proof.print_size();

    return proof;
}

template <typename ppT>
r1cs_ppzksnark_processed_verification_key<ppT> r1cs_ppzksnark_verifier_process_vk(const r1cs_ppzksnark_verification_key<ppT> &vk)
{
    enter_block("Call to r1cs_ppzksnark_verifier_process_vk");

    r1cs_ppzksnark_processed_verification_key<ppT> pvk;
    pvk.pp_G2_one_precomp        = ppT::precompute_G2(G2<ppT>::one());
    pvk.vk_alphaA_g2_precomp     = ppT::precompute_G2(vk.alphaA_g2);
    pvk.vk_alphaB_g1_precomp     = ppT::precompute_G1(vk.alphaB_g1);
    pvk.vk_alphaC_g2_precomp     = ppT::precompute_G2(vk.alphaC_g2);
    pvk.vk_rC_Z_g2_precomp       = ppT::precompute_G2(vk.rC_Z_g2);
    pvk.vk_gamma_g2_precomp      = ppT::precompute_G2(vk.gamma_g2);
    pvk.vk_gamma_beta_g1_precomp = ppT::precompute_G1(vk.gamma_beta_g1);
    pvk.vk_gamma_beta_g2_precomp = ppT::precompute_G2(vk.gamma_beta_g2);

    pvk.encoded_IC_query = vk.encoded_IC_query;

    leave_block("Call to r1cs_ppzksnark_verifier_process_vk");

    return pvk;
}

template <typename ppT>
bool r1cs_ppzksnark_online_verifier_weak_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                            const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_online_verifier_weak_IC");
    assert(pvk.encoded_IC_query.domain_size() >= primary_input.size());

    enter_block("Compute input-dependent part of A");
    const accumulation_vector<G1<ppT> > accumulated_IC = pvk.encoded_IC_query.template accumulate_chunk<Fr<ppT> >(primary_input.begin(), primary_input.end(), 0);
    const G1<ppT> &acc = accumulated_IC.first;
    leave_block("Compute input-dependent part of A");

    bool result = true;

    enter_block("Check if the proof is well-formed");
    if (!proof.is_well_formed())
    {
        if (!inhibit_profiling_info)
        {
            print_indent(); printf("At least one of the proof elements does not lie on the curve.\n");
        }
        result = false;
    }
    leave_block("Check if the proof is well-formed");

    enter_block("Online pairing computations");
    enter_block("Check knowledge commitment for A is valid");
    G1_precomp<ppT> proof_g_A_g_precomp      = ppT::precompute_G1(proof.g_A.g);
    G1_precomp<ppT> proof_g_A_h_precomp = ppT::precompute_G1(proof.g_A.h);
    Fqk<ppT> kc_A_1 = ppT::miller_loop(proof_g_A_g_precomp,      pvk.vk_alphaA_g2_precomp);
    Fqk<ppT> kc_A_2 = ppT::miller_loop(proof_g_A_h_precomp, pvk.pp_G2_one_precomp);
    GT<ppT> kc_A = ppT::final_exponentiation(kc_A_1 * kc_A_2.unitary_inverse());
    if (kc_A != GT<ppT>::one())
    {
        if (!inhibit_profiling_info)
        {
            print_indent(); printf("Knowledge commitment for A query incorrect.\n");
        }
        result = false;
    }
    leave_block("Check knowledge commitment for A is valid");

    enter_block("Check knowledge commitment for B is valid");
    G2_precomp<ppT> proof_g_B_g_precomp      = ppT::precompute_G2(proof.g_B.g);
    G1_precomp<ppT> proof_g_B_h_precomp = ppT::precompute_G1(proof.g_B.h);
    Fqk<ppT> kc_B_1 = ppT::miller_loop(pvk.vk_alphaB_g1_precomp, proof_g_B_g_precomp);
    Fqk<ppT> kc_B_2 = ppT::miller_loop(proof_g_B_h_precomp,    pvk.pp_G2_one_precomp);
    GT<ppT> kc_B = ppT::final_exponentiation(kc_B_1 * kc_B_2.unitary_inverse());
    if (kc_B != GT<ppT>::one())
    {
        if (!inhibit_profiling_info)
        {
            print_indent(); printf("Knowledge commitment for B query incorrect.\n");
        }
        result = false;
    }
    leave_block("Check knowledge commitment for B is valid");

    enter_block("Check knowledge commitment for C is valid");
    G1_precomp<ppT> proof_g_C_g_precomp      = ppT::precompute_G1(proof.g_C.g);
    G1_precomp<ppT> proof_g_C_h_precomp = ppT::precompute_G1(proof.g_C.h);
    Fqk<ppT> kc_C_1 = ppT::miller_loop(proof_g_C_g_precomp,      pvk.vk_alphaC_g2_precomp);
    Fqk<ppT> kc_C_2 = ppT::miller_loop(proof_g_C_h_precomp, pvk.pp_G2_one_precomp);
    GT<ppT> kc_C = ppT::final_exponentiation(kc_C_1 * kc_C_2.unitary_inverse());
    if (kc_C != GT<ppT>::one())
    {
        if (!inhibit_profiling_info)
        {
            print_indent(); printf("Knowledge commitment for C query incorrect.\n");
        }
        result = false;
    }
    leave_block("Check knowledge commitment for C is valid");

    enter_block("Check QAP divisibility");
    // check that g^((A+acc)*B)=g^(H*\Prod(t-\sigma)+C)
    // equivalently, via pairings, that e(g^(A+acc), g^B) = e(g^H, g^Z) + e(g^C, g^1)
    G1_precomp<ppT> proof_g_A_g_acc_precomp = ppT::precompute_G1(proof.g_A.g + acc);
    G1_precomp<ppT> proof_g_H_precomp       = ppT::precompute_G1(proof.g_H);
    Fqk<ppT> QAP_1  = ppT::miller_loop(proof_g_A_g_acc_precomp,  proof_g_B_g_precomp);
    Fqk<ppT> QAP_23  = ppT::double_miller_loop(proof_g_H_precomp, pvk.vk_rC_Z_g2_precomp, proof_g_C_g_precomp, pvk.pp_G2_one_precomp);
    GT<ppT> QAP = ppT::final_exponentiation(QAP_1 * QAP_23.unitary_inverse());
    if (QAP != GT<ppT>::one())
    {
        if (!inhibit_profiling_info)
        {
            print_indent(); printf("QAP divisibility check failed.\n");
        }
        result = false;
    }
    leave_block("Check QAP divisibility");

    enter_block("Check same coefficients were used");
    G1_precomp<ppT> proof_g_K_precomp = ppT::precompute_G1(proof.g_K);
    G1_precomp<ppT> proof_g_A_g_acc_C_precomp = ppT::precompute_G1((proof.g_A.g + acc) + proof.g_C.g);
    Fqk<ppT> K_1 = ppT::miller_loop(proof_g_K_precomp, pvk.vk_gamma_g2_precomp);
    Fqk<ppT> K_23 = ppT::double_miller_loop(proof_g_A_g_acc_C_precomp, pvk.vk_gamma_beta_g2_precomp, pvk.vk_gamma_beta_g1_precomp, proof_g_B_g_precomp);
    GT<ppT> K = ppT::final_exponentiation(K_1 * K_23.unitary_inverse());
    if (K != GT<ppT>::one())
    {
        if (!inhibit_profiling_info)
        {
            print_indent(); printf("Same-coefficient check failed.\n");
        }
        result = false;
    }
    leave_block("Check same coefficients were used");
    leave_block("Online pairing computations");
    leave_block("Call to r1cs_ppzksnark_online_verifier_weak_IC");

    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_verifier_weak_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                     const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                     const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_verifier_weak_IC");
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = r1cs_ppzksnark_online_verifier_weak_IC<ppT>(pvk, primary_input, proof);
    leave_block("Call to r1cs_ppzksnark_verifier_weak_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_online_verifier_strong_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                              const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                              const r1cs_ppzksnark_proof<ppT> &proof)
{
    bool result = true;
    enter_block("Call to r1cs_ppzksnark_online_verifier_strong_IC");

    if (pvk.encoded_IC_query.domain_size() != primary_input.size())
    {
        print_indent(); printf("Input length differs from expected (got %zu, expected %zu).\n", primary_input.size(), pvk.encoded_IC_query.domain_size());
        result = false;
    }
    else
    {
        result = r1cs_ppzksnark_online_verifier_weak_IC(pvk, primary_input, proof);
    }

    leave_block("Call to r1cs_ppzksnark_online_verifier_strong_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_verifier_strong_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                       const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                       const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_verifier_strong_IC");
    //r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(vk);
    //bool result = r1cs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, primary_input, proof);
    bool result = r1cs_ppzksnark_probabilistic_verifier<ppT>(vk,primary_input,proof);
    leave_block("Call to r1cs_ppzksnark_verifier_strong_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_affine_verifier_weak_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                            const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_affine_verifier_weak_IC");
    assert(vk.encoded_IC_query.domain_size() >= primary_input.size());

    affine_ate_G2_precomp<ppT> pvk_pp_G2_one_precomp        = ppT::affine_ate_precompute_G2(G2<ppT>::one());
    affine_ate_G2_precomp<ppT> pvk_vk_alphaA_g2_precomp     = ppT::affine_ate_precompute_G2(vk.alphaA_g2);
    affine_ate_G1_precomp<ppT> pvk_vk_alphaB_g1_precomp     = ppT::affine_ate_precompute_G1(vk.alphaB_g1);
    affine_ate_G2_precomp<ppT> pvk_vk_alphaC_g2_precomp     = ppT::affine_ate_precompute_G2(vk.alphaC_g2);
    affine_ate_G2_precomp<ppT> pvk_vk_rC_Z_g2_precomp       = ppT::affine_ate_precompute_G2(vk.rC_Z_g2);
    affine_ate_G2_precomp<ppT> pvk_vk_gamma_g2_precomp      = ppT::affine_ate_precompute_G2(vk.gamma_g2);
    affine_ate_G1_precomp<ppT> pvk_vk_gamma_beta_g1_precomp = ppT::affine_ate_precompute_G1(vk.gamma_beta_g1);
    affine_ate_G2_precomp<ppT> pvk_vk_gamma_beta_g2_precomp = ppT::affine_ate_precompute_G2(vk.gamma_beta_g2);

    enter_block("Compute input-dependent part of A");
    const accumulation_vector<G1<ppT> > accumulated_IC = vk.encoded_IC_query.template accumulate_chunk<Fr<ppT> >(primary_input.begin(), primary_input.end(), 0);
    assert(accumulated_IC.is_fully_accumulated());
    const G1<ppT> &acc = accumulated_IC.first;
    leave_block("Compute input-dependent part of A");

    bool result = true;
    enter_block("Check knowledge commitment for A is valid");
    affine_ate_G1_precomp<ppT> proof_g_A_g_precomp = ppT::affine_ate_precompute_G1(proof.g_A.g);
    affine_ate_G1_precomp<ppT> proof_g_A_h_precomp = ppT::affine_ate_precompute_G1(proof.g_A.h);
    Fqk<ppT> kc_A_miller = ppT::affine_ate_e_over_e_miller_loop(proof_g_A_g_precomp, pvk_vk_alphaA_g2_precomp, proof_g_A_h_precomp, pvk_pp_G2_one_precomp);
    GT<ppT> kc_A = ppT::final_exponentiation(kc_A_miller);

    if (kc_A != GT<ppT>::one())
    {
        print_indent(); printf("Knowledge commitment for A query incorrect.\n");
        result = false;
    }
    leave_block("Check knowledge commitment for A is valid");

    enter_block("Check knowledge commitment for B is valid");
    affine_ate_G2_precomp<ppT> proof_g_B_g_precomp = ppT::affine_ate_precompute_G2(proof.g_B.g);
    affine_ate_G1_precomp<ppT> proof_g_B_h_precomp = ppT::affine_ate_precompute_G1(proof.g_B.h);
    Fqk<ppT> kc_B_miller = ppT::affine_ate_e_over_e_miller_loop(pvk_vk_alphaB_g1_precomp, proof_g_B_g_precomp, proof_g_B_h_precomp,    pvk_pp_G2_one_precomp);
    GT<ppT> kc_B = ppT::final_exponentiation(kc_B_miller);
    if (kc_B != GT<ppT>::one())
    {
        print_indent(); printf("Knowledge commitment for B query incorrect.\n");
        result = false;
    }
    leave_block("Check knowledge commitment for B is valid");

    enter_block("Check knowledge commitment for C is valid");
    affine_ate_G1_precomp<ppT> proof_g_C_g_precomp = ppT::affine_ate_precompute_G1(proof.g_C.g);
    affine_ate_G1_precomp<ppT> proof_g_C_h_precomp = ppT::affine_ate_precompute_G1(proof.g_C.h);
    Fqk<ppT> kc_C_miller = ppT::affine_ate_e_over_e_miller_loop(proof_g_C_g_precomp, pvk_vk_alphaC_g2_precomp, proof_g_C_h_precomp, pvk_pp_G2_one_precomp);
    GT<ppT> kc_C = ppT::final_exponentiation(kc_C_miller);
    if (kc_C != GT<ppT>::one())
    {
        print_indent(); printf("Knowledge commitment for C query incorrect.\n");
        result = false;
    }
    leave_block("Check knowledge commitment for C is valid");

    enter_block("Check QAP divisibility");
    affine_ate_G1_precomp<ppT> proof_g_A_g_acc_precomp = ppT::affine_ate_precompute_G1(proof.g_A.g + acc);
    affine_ate_G1_precomp<ppT> proof_g_H_precomp       = ppT::affine_ate_precompute_G1(proof.g_H);
    Fqk<ppT> QAP_miller  = ppT::affine_ate_e_times_e_over_e_miller_loop(proof_g_H_precomp, pvk_vk_rC_Z_g2_precomp, proof_g_C_g_precomp, pvk_pp_G2_one_precomp, proof_g_A_g_acc_precomp,  proof_g_B_g_precomp);
    GT<ppT> QAP = ppT::final_exponentiation(QAP_miller);
    if (QAP != GT<ppT>::one())
    {
        print_indent(); printf("QAP divisibility check failed.\n");
        result = false;
    }
    leave_block("Check QAP divisibility");

    enter_block("Check same coefficients were used");
    affine_ate_G1_precomp<ppT> proof_g_K_precomp = ppT::affine_ate_precompute_G1(proof.g_K);
    affine_ate_G1_precomp<ppT> proof_g_A_g_acc_C_precomp = ppT::affine_ate_precompute_G1((proof.g_A.g + acc) + proof.g_C.g);
    Fqk<ppT> K_miller = ppT::affine_ate_e_times_e_over_e_miller_loop(proof_g_A_g_acc_C_precomp, pvk_vk_gamma_beta_g2_precomp, pvk_vk_gamma_beta_g1_precomp, proof_g_B_g_precomp, proof_g_K_precomp, pvk_vk_gamma_g2_precomp);
    GT<ppT> K = ppT::final_exponentiation(K_miller);
    if (K != GT<ppT>::one())
    {
        print_indent(); printf("Same-coefficient check failed.\n");
        result = false;
    }
    leave_block("Check same coefficients were used");

    leave_block("Call to r1cs_ppzksnark_affine_verifier_weak_IC");

    return result;
}





/*****Ariel stuff from here ****/

template<typename ppT>
r1cs_ppzksnark_processed_batch_verification_key<ppT> r1cs_ppzksnark_batch_verifier_process_vk(const r1cs_ppzksnark_verification_key<alt_bn128_pp> &vk)
{
    enter_block("Call to r1cs_ppzksnark_batch_verifier_process_vk");
    r1cs_ppzksnark_processed_batch_verification_key<ppT> pvk;
    pvk.pair1 = ppT::precompute_G2(vk.alphaA_g2);
    
    //computing the second input for the second ML factor
    // r1Pi'_a + R2Pi'_B+r3Pi'_C + r5Pi_C and -g2
    pvk.pair2 = alt_bn128_pp::precompute_G2(-G2<alt_bn128_pp>::one());
    //computing the second input for the third ML factor
    // r3Pi_c and vk_C
    pvk.pair3 = alt_bn128_pp::precompute_G2(vk.alphaC_g2) ;
    
    //computing the second input for the fourth ML factor
    // r4Pi_K and vk_gamma
    pvk.pair4 = alt_bn128_pp::precompute_G2(vk.gamma_g2);
    
    //computing the second input for the fifth ML factor
    //−r 4(vk x + πA + πC) and vk^2_betagamma
    pvk.pair5 = alt_bn128_pp::precompute_G2(vk.gamma_beta_g2);

    //computing the second input for the six ML factor
    //r5Pi_H and -vk_Z
    pvk.pair6 = alt_bn128_pp::precompute_G2(-vk.rC_Z_g2);
    
    leave_block("Call to r1cs_ppzksnark_batch_verifier_process_vk");
    return pvk;
}




/*
 * accumulate another proof inside acc for the final batch check
 */
template<typename ppT>
void r1cs_ppzksnark_batcher(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                            batch_verification_accumulator<ppT> &acc,
                                            const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_batcher");
    const accumulation_vector<G1<ppT> > accumulated_IC = vk.encoded_IC_query.template accumulate_chunk<Fr<ppT> >(primary_input.begin(), primary_input.end(), 0);
    const G1<ppT> &accu = accumulated_IC.first;
    //computing the random coefficients that will be used
    auto r_1 = Fr<ppT>::random_element();
    auto r_2 = Fr<ppT>::random_element();
    auto r_3 = Fr<ppT>::random_element();
    auto r_4 = Fr<ppT>::random_element();
    auto r_5 = Fr<ppT>::random_element();


    //computing left input for the first ML factor
    // r1Pi_a and vk_A
    acc.pair1 = acc.pair1 + r_1*proof.g_A.g;
    
    //computing left input for the second ML factor
    // r1Pi'_a + R2Pi'_B+r3Pi'_C + r5Pi_C and -g2
    acc.pair2 = acc.pair2 + r_1*proof.g_A.h+r_2*proof.g_B.h + r_3*proof.g_C.h + r_5*proof.g_C.g;
    //computing left input for the third ML factor
    // r3Pi_c and vk_C
    acc.pair3 = acc.pair3 + r_3*proof.g_C.g;

    //computing left input for the fourth ML factor
    // r4Pi_K and vk_gamma
    acc.pair4 = acc.pair4 + r_4*proof.g_K;
        //computing left input for the fifth ML factor
    //−r 4(vk x + πA + πC) and vk^2_betagamma
    acc.pair5 = acc.pair5 + -r_4*(accu + proof.g_A.g + proof.g_C.g);
    
    //computing left input for the six ML factor
    //r5Pi_H and -vk_Z
    acc.pair6 = acc.pair6 + r_5*proof.g_H;
    
    //computing left input for the seventh ML factor
    //r_2 vk_B-r_4 vk^1_{\beta\gamma}+r_5(vk_x + \pi_A) and pi_B
    auto left_7 = ppT::precompute_G1(r_2*vk.alphaB_g1-r_4*vk.gamma_beta_g1+r_5*(accu+proof.g_A.g));
    auto right_7 = ppT::precompute_G2(proof.g_B.g);
    acc.pair7 = acc.pair7*ppT::miller_loop(left_7,right_7);

    leave_block("Call to r1cs_ppzksnark_batcher");
}




/*A verifier that batches all pairing checks into zero using random coefficients
  requires only zero FE (final expzerontiation).
  Also requires only zero call to multiple_miller_loop, as only needs to compute zero product of 
  Miller loops (MLs).
  Also uses rule e(a,c)*e(b,c)=e(a+b,c) to reduce number of factors in the ML product
  If proof is wrong has at most 1/|F| probability of accepting it
  Always accepts correct proofs
*/
template<>
bool r1cs_ppzksnark_probabilistic_verifier(const r1cs_ppzksnark_verification_key<alt_bn128_pp> &vk,
                                            const r1cs_ppzksnark_primary_input<alt_bn128_pp> &primary_input,
                                            const r1cs_ppzksnark_proof<alt_bn128_pp> &proof)
{
    enter_block("Call to r1cs_ppzksnark_probabilistic_verifier");
    const accumulation_vector<G1<alt_bn128_pp> > accumulated_IC = vk.encoded_IC_query.template accumulate_chunk<Fr<alt_bn128_pp> >(primary_input.begin(), primary_input.end(), 0);
    const G1<alt_bn128_pp> &acc = accumulated_IC.first;
    //computing the random coefficients that will be used
    auto r_1 = Fr<alt_bn128_pp>::random_element();
    auto r_2 = Fr<alt_bn128_pp>::random_element();
    auto r_3 = Fr<alt_bn128_pp>::random_element();
    auto r_4 = Fr<alt_bn128_pp>::random_element();
    auto r_5 = Fr<alt_bn128_pp>::random_element();

    enter_block("Preparing 7 ML factors");

    //computing left input for the first ML factor
    // r1Pi_a and vk_A
    auto left_1 = alt_bn128_pp::precompute_G1(r_1*proof.g_A.g);
    auto right_1 = alt_bn128_pp::precompute_G2(vk.alphaA_g2);
    auto pair_1 = std::make_pair(left_1,right_1);
    
    //computing left input for the second ML factor
    // r1Pi'_a + R2Pi'_B+r3Pi'_C + r5Pi_C and -g2
    auto left_2 = alt_bn128_pp::precompute_G1(r_1*proof.g_A.h+r_2*proof.g_B.h + r_3*proof.g_C.h + r_5*proof.g_C.g);
    auto right_2 = alt_bn128_pp::precompute_G2(-G2<alt_bn128_pp>::one());
    auto pair_2 = std::make_pair(left_2,right_2);
    //computing left input for the third ML factor
    // r3Pi_c and vk_C
    auto left_3 = alt_bn128_pp::precompute_G1(r_3*proof.g_C.g);
    auto right_3 = alt_bn128_pp::precompute_G2(vk.alphaC_g2) ;
    auto pair_3 = std::make_pair(left_3,right_3);

    //computing left input for the fourth ML factor
    // r4Pi_K and vk_gamma
    auto left_4 = alt_bn128_pp::precompute_G1(r_4*proof.g_K);
    auto right_4 = alt_bn128_pp::precompute_G2(vk.gamma_g2);
    auto pair_4 = std::make_pair(left_4,right_4);

    //computing left input for the fifth ML factor
    //−r 4(vk x + πA + πC) and vk^2_betagamma
    auto left_5 = alt_bn128_pp::precompute_G1(-r_4*(acc + proof.g_A.g + proof.g_C.g));
    auto right_5 = alt_bn128_pp::precompute_G2(vk.gamma_beta_g2);
    auto pair_5 = std::make_pair(left_5,right_5);

    //computing left input for the six ML factor
    //r5Pi_H and -vk_Z
    auto left_6 = alt_bn128_pp::precompute_G1(r_5*proof.g_H);
    auto right_6 = alt_bn128_pp::precompute_G2(-vk.rC_Z_g2);
    auto pair_6 = std::make_pair(left_6,right_6);

    //computing left input for the seventh ML factor
    //r_2 vk_B-r_4 vk^1_{\beta\gamma}+r_5(vk_x + \pi_A) and pi_B
    auto left_7 = alt_bn128_pp::precompute_G1(r_2*vk.alphaB_g1-r_4*vk.gamma_beta_g1+r_5*(acc+proof.g_A.g));
    auto right_7 = alt_bn128_pp::precompute_G2(proof.g_B.g);
    auto pair_7 = std::make_pair(left_7,right_7);
    leave_block("Preparing 7 ML factors");

   /* auto ML1 = alt_bn128_pp::double_miller_loop(left_1,right_1,left_2,right_2);
    auto ML2 = alt_bn128_pp::double_miller_loop(left_3,right_3,left_4,right_4);
    auto ML3 = alt_bn128_pp::double_miller_loop(left_5,right_5,left_6,right_6);
    auto ML4 = alt_bn128_pp::miller_loop(left_7,right_7);*/
    alt_bn128_Fq12 ML  = alt_bn128_pp::multiple_miller_loop({
       pair_1,pair_2,pair_3,pair_4,pair_5,pair_6,pair_7
    });
    auto FE= alt_bn128_pp::final_exponentiation(ML);
    leave_block("Call to r1cs_ppzksnark_probabilistic_verifier");
    return (FE==GT<alt_bn128_pp>::one());
}

template<typename ppT>
bool r1cs_ppzksnark_probabilistic_verifier(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                            const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = r1cs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, primary_input, proof); 
    return result;  
}

template<>
bool r1cs_ppzksnark_batch_verifier(const r1cs_ppzksnark_processed_batch_verification_key <alt_bn128_pp> &pvk,
                                            const batch_verification_accumulator <alt_bn128_pp> &acc,
                                            const r1cs_ppzksnark_primary_input<alt_bn128_pp> &primary_input,
                                            const r1cs_ppzksnark_proof<alt_bn128_pp> &proof)
{
     enter_block("Call to r1cs_ppzksnark_batch_verifier");

    enter_block("Preparing 7 ML factors");

    //computing left input for the first ML factor
    // r3Pi_a and vk_A
    auto pair_1 = std::make_pair(alt_bn128_pp::precompute_G1(acc.pair1),pvk.pair1);
    
    //computing left input for the second ML factor
    // r3Pi'_a + R2Pi'_B+r3Pi'_C + r5Pi_C and -g2
     auto pair_2 = std::make_pair(alt_bn128_pp::precompute_G1(acc.pair2),pvk.pair2);
    //computing left input for the third ML factor
    // r3Pi_c and vk_C
    auto pair_3 = std::make_pair(alt_bn128_pp::precompute_G1(acc.pair3),pvk.pair3);

    //computing left input for the fourth ML factor
    // r4Pi_K and vk_gamma
    auto pair_4 = std::make_pair(alt_bn128_pp::precompute_G1(acc.pair4),pvk.pair4);

    //computing left input for the fifth ML factor
    //−r 4(vk x + πA + πC) and vk^2_betagamma
    auto pair_5 = std::make_pair(alt_bn128_pp::precompute_G1(acc.pair5),pvk.pair5);

    //computing left input for the six ML factor
    //r5Pi_H and -vk_Z
    auto pair_6 = std::make_pair(alt_bn128_pp::precompute_G1(acc.pair6),pvk.pair6);

    //computing left input for the seventh ML factor
    //r_2 vk_B-r_4 vk^3_{\beta\gamma}+r_5(vk_x + \pi_A) and pi_B
    leave_block("Preparing 7 ML factors");

    alt_bn128_Fq12 ML  = alt_bn128_pp::multiple_miller_loop({
       pair_1,pair_2,pair_3,pair_4,pair_5,pair_6
    })*acc.pair7;
    auto FE= alt_bn128_pp::final_exponentiation(ML);
    leave_block("Call to r1cs_ppzksnark_batch_verifier");
    return (FE==GT<alt_bn128_pp>::one());
}

template<typename ppT>
bool r1cs_ppzksnark_batch_verifier(const r1cs_ppzksnark_processed_batch_verification_key <ppT> &pvk,
                                            const batch_verification_accumulator <ppT> &acc,
                                            const r1cs_ppzksnark_primary_input<ppT> &primary_input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    assert(false);
    return false;  
}
} // libsnark
#endif // R1CS_PPZKSNARK_TCC_
