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

#include "r1cs_ppzksnark/r1cs_ppzksnark.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
#include <omp.h>

#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "encoding/multiexp.hpp"
#include "qap/qap.hpp"

namespace libsnark {

template<typename ppT>
r1cs_ppzksnark_IC_query<ppT> r1cs_ppzksnark_IC_query<ppT>::accumulate(const typename Fr_vector<ppT>::const_iterator w_begin,
                                                  const typename Fr_vector<ppT>::const_iterator w_end,
                                                  const size_t offset) const
{
    enter_block("Call to r1cs_ppzksnark_IC_query::accumulate");
    r1cs_ppzksnark_IC_query<ppT> result;
    result.base = this->base;

    const size_t w_size = w_end - w_begin;
    bool in_block = false;
    size_t first_pos = -1, last_pos = -1; // g++ -flto emits unitialized warning, even though in_block guards for such cases.

    for (size_t i = 0; i < pos.size(); ++i)
    {
        const bool matching_pos = (offset <= pos[i] && pos[i] < offset + w_size);
        // printf("i = %zu, pos[i] = %zu, offset = %zu, w_size = %zu\n", i, pos[i], offset, w_size);
        bool copy_over;

        if (in_block)
        {
            if (matching_pos && last_pos == i-1)
            {
                // block can be extended, do it
                last_pos = i;
                copy_over = false;
            }
            else
            {
                // block ends here
                in_block = false;
                copy_over = true;

#ifdef DEBUG
                print_indent(); printf("doing multiexp for w_%zu ... w_%zu\n", pos[first_pos], pos[last_pos]);
#endif
                result.base = result.base + multi_exp<G1<ppT>, Fr<ppT> >(G1<ppT>::zero(),
                                                                         encoded_terms.begin() + first_pos,
                                                                         encoded_terms.begin() + last_pos + 1,
                                                                         w_begin + (pos[first_pos] - offset),
                                                                         w_begin + (pos[last_pos] - offset) + 1,
                                                                         1, true);
            }
        }
        else
        {
            if (matching_pos)
            {
                // block can be started
                first_pos = i;
                last_pos = i;
                in_block = true;
                copy_over = false;
            }
            else
            {
                copy_over = true;
            }
        }

        if (copy_over)
        {
            result.pos.emplace_back(pos[i]);
            result.encoded_terms.emplace_back(encoded_terms[i]);
        }
    }

    if (in_block)
    {
#ifdef DEBUG
        print_indent(); printf("doing multiexp for w_%zu ... w_%zu\n", pos[first_pos], pos[last_pos]);
#endif
        result.base = result.base + multi_exp<G1<ppT>, Fr<ppT> >(G1<ppT>::zero(),
                                                                 encoded_terms.begin() + first_pos,
                                                                 encoded_terms.begin() + last_pos + 1,
                                                                 w_begin + (pos[first_pos] - offset),
                                                                 w_begin + (pos[last_pos] - offset) + 1,
                                                                 1, true);
    }

    leave_block("Call to r1cs_ppzksnark_IC_query::accumulate");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_IC_query<ppT>::operator==(const r1cs_ppzksnark_IC_query<ppT> &other) const
{
    return (this->base == other.base &&
            this->pos == other.pos &&
            this->encoded_terms == other.encoded_terms);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_IC_query<ppT> &q)
{
    out << q.base << OUTPUT_NEWLINE;

    out << q.pos.size() << "\n";
    for (size_t i = 0; i < q.pos.size(); ++i)
    {
        out << q.pos[i] << "\n";
        out << q.encoded_terms[i] << OUTPUT_NEWLINE;
    }

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_IC_query<ppT> &q)
{
    in >> q.base;
    consume_OUTPUT_NEWLINE(in);

    size_t s;
    in >> s;

    q.pos.resize(0);
    q.encoded_terms.resize(0);
    consume_newline(in);

    for (size_t i = 0; i < s; ++i)
    {
        size_t idx;
        in >> idx;
        consume_newline(in);

        G1<ppT> g;
        in >> g;
        consume_OUTPUT_NEWLINE(in);
        q.pos.emplace_back(idx);
        q.encoded_terms.emplace_back(g);
    }

    return in;
}

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
            *(this->encoded_IC_query) == *(other.encoded_IC_query));
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
    out << *(vk.encoded_IC_query) << OUTPUT_NEWLINE;

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
    r1cs_ppzksnark_IC_query<ppT>* icq = new r1cs_ppzksnark_IC_query<ppT>;
    in >> *icq;
    vk.encoded_IC_query.reset(icq);
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
            *(this->encoded_IC_query) == *(other.encoded_IC_query));
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
    out << *(pvk.encoded_IC_query) << OUTPUT_NEWLINE;

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
    r1cs_ppzksnark_IC_query<ppT>* icq = new r1cs_ppzksnark_IC_query<ppT>;
    in >> *icq;
    pvk.encoded_IC_query.reset(icq);
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
    result.encoded_IC_query.reset(new r1cs_ppzksnark_IC_query<ppT>(base, v));

    return result;
}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator(const r1cs_constraint_system<Fr<ppT> > &cs)
{
    enter_block("Call to r1cs_ppzksnark_generator");

    r1cs_constraint_system<Fr<ppT> > cs_copy(cs);
    cs_copy.swap_AB_if_beneficial();

    size_t qap_num_vars;
    size_t qap_degree;
    size_t qap_num_inputs;
    qap_get_params(cs_copy, &qap_num_vars, &qap_degree, &qap_num_inputs);
    std::shared_ptr<evaluation_domain<Fr<ppT> > > domain = get_evaluation_domain<Fr<ppT> >(qap_degree);

    print_indent(); printf("* QAP number of variables: %zu\n", qap_num_vars);
    print_indent(); printf("* QAP pre degree: %zu\n", cs_copy.constraints.size());
    print_indent(); printf("* QAP degree: %zu\n", qap_degree);
    print_indent(); printf("* QAP number of input variables: %zu\n", qap_num_inputs);

    const  Fr<ppT> t = Fr<ppT>::random_element(),
              alphaA = Fr<ppT>::random_element(),
              alphaB = Fr<ppT>::random_element(),
              alphaC = Fr<ppT>::random_element(),
                  rA = Fr<ppT>::random_element(),
                  rB = Fr<ppT>::random_element(),
                beta = Fr<ppT>::random_element(),
               gamma = Fr<ppT>::random_element();
    const Fr<ppT> rC = rA * rB;

    const Fr<ppT> Z = domain->compute_Z(t);

    ABCH_eval_at_t<Fr<ppT> > abch = qap_instance_map(cs_copy, t);

    /* after moves abch will be left in unspecified state, but we don't use it later */
    Fr_vector<ppT> A_query = std::move(abch.At);
    Fr_vector<ppT> B_query = std::move(abch.Bt);
    Fr_vector<ppT> C_query = std::move(abch.Ct);
    Fr_vector<ppT> H_query = std::move(abch.Ht);

    // consistency check query. important that this happens *before* zeroing out
    Fr_vector<ppT> K_query;
    K_query.reserve(3+qap_num_vars+1);

    for (size_t i = 0; i < 3+qap_num_vars+1; ++i)
    {
        K_query.emplace_back(beta*((rA * A_query[i]) + (rB * B_query[i]) + (rC * C_query[i])));
    }

    /* zero out IC from A query and place it into IC coefficients */
    Fr_vector<ppT> IC_coefficients;
    IC_coefficients.reserve(qap_num_inputs + 1);
    for (size_t i = 0; i < qap_num_inputs + 1; ++i)
    {
        IC_coefficients.emplace_back(A_query[3+i]);
        assert(!IC_coefficients[i].is_zero());
        A_query[3+i] = Fr<ppT>::zero();
    }
    enter_block("Generate R1CS proving key");

    const size_t g1_exp_count = 2*(abch.non_zero_At - qap_num_inputs + abch.non_zero_Ct)
                                + abch.non_zero_Bt
                                + abch.non_zero_Ht
                                + K_query.size();
    const size_t g2_exp_count = abch.non_zero_Bt;

    size_t g1_window = get_exp_window_size<G1<ppT> >(g1_exp_count);
    size_t g2_window = get_exp_window_size<G2<ppT> >(g2_exp_count);
    print_indent(); printf("* G1 window: %zu\n", g1_window);
    print_indent(); printf("* G2 window: %zu\n", g2_window);

#ifdef MULTICORE
    const size_t chunks = 4; //omp_get_max_threads();
#else
    const size_t chunks = 1;
#endif

    enter_block("Generating G1 multiexp table");
    window_table<G1<ppT> > g1_table = get_window_table(Fr<ppT>::num_bits, G1<ppT>::zero(), g1_window, G1<ppT>::one());
    leave_block("Generating G1 multiexp table");

    enter_block("Generating G2 multiexp table");
    window_table<G2<ppT> > g2_table = get_window_table(Fr<ppT>::num_bits, G2<ppT>::zero(), g2_window, G2<ppT>::one());
    leave_block("Generating G2 multiexp table");

    enter_block("Generate knowledge commitments");
    enter_block("Compute the A-query", false);
    G1G1_knowledge_commitment_vector<ppT> encoded_A_query = kc_batch_exp(Fr<ppT>::num_bits, g1_window, g1_window, g1_table, g1_table, rA, rA*alphaA, A_query, true, chunks);
    leave_block("Compute the A-query", false);

    enter_block("Compute the B-query", false);
    G2G1_knowledge_commitment_vector<ppT> encoded_B_query = kc_batch_exp(Fr<ppT>::num_bits, g2_window, g1_window, g2_table, g1_table, rB, rB*alphaB, B_query, true, chunks);
    leave_block("Compute the B-query", false);

    enter_block("Compute the C-query", false);
    G1G1_knowledge_commitment_vector<ppT> encoded_C_query = kc_batch_exp(Fr<ppT>::num_bits, g1_window, g1_window, g1_table, g1_table, rC, rC*alphaC, C_query, true, chunks);
    leave_block("Compute the C-query", false);

    enter_block("Compute the H-query", false);
    G1_vector<ppT> encoded_H_query = batch_exp(Fr<ppT>::num_bits, g1_window, g1_table, H_query);
    leave_block("Compute the H-query", false);

    enter_block("Compute the K-query", false);
    G1_vector<ppT> encoded_K_query = batch_exp(Fr<ppT>::num_bits, g1_window, g1_table, K_query);
#ifdef USE_ADD_SPECIAL
    batch_to_special<G1<ppT> >(encoded_K_query);
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
    G2<ppT> rC_Z_g2 = (rC * Z) * G2<ppT>::one();

    enter_block("Encode IC query for R1CS verification key");
    G1<ppT> encoded_IC_base = (rA * IC_coefficients[0]) * G1<ppT>::one();
    Fr_vector<ppT> multiplied_IC_coefficients;
    multiplied_IC_coefficients.reserve(qap_num_inputs);
    for (size_t i = 1; i < qap_num_inputs + 1; ++i)
    {
        multiplied_IC_coefficients.emplace_back(IC_coefficients[i] * rA);
    }
    G1_vector<ppT> encoded_IC_query = batch_exp(Fr<ppT>::num_bits, g1_window, g1_table, multiplied_IC_coefficients);

    leave_block("Encode IC query for R1CS verification key");
    leave_block("Generate R1CS verification key");

    leave_block("Call to r1cs_ppzksnark_generator");

    r1cs_ppzksnark_IC_query<ppT>* icptr = new r1cs_ppzksnark_IC_query<ppT>;
    *icptr = r1cs_ppzksnark_IC_query<ppT>(encoded_IC_base, encoded_IC_query);

    r1cs_ppzksnark_verification_key<ppT> vk = r1cs_ppzksnark_verification_key<ppT>(alphaA_g2,
                                                               alphaB_g1,
                                                               alphaC_g2,
                                                               gamma_g2,
                                                               gamma_beta_g1,
                                                               gamma_beta_g2,
                                                               rC_Z_g2,
                                                               icptr);
    r1cs_ppzksnark_proving_key<ppT> pk = r1cs_ppzksnark_proving_key<ppT>(std::move(encoded_A_query),
                                                     std::move(encoded_B_query),
                                                     std::move(encoded_C_query),
                                                     std::move(encoded_H_query),
                                                     std::move(encoded_K_query),
                                                     std::move(cs_copy));

    pk.print_size();
    vk.print_size();

    return r1cs_ppzksnark_keypair<ppT>(std::move(vk), std::move(pk));
}

template <typename ppT>
r1cs_ppzksnark_proof<ppT> r1cs_ppzksnark_prover(const r1cs_ppzksnark_proving_key<ppT> &pk,
                                      const r1cs_variable_assignment<Fr<ppT> > &w)
{
    enter_block("Call to r1cs_ppzksnark_prover");

    size_t qap_num_vars;
    size_t qap_degree;
    size_t qap_num_inputs;
    qap_get_params(pk.constraint_system, &qap_num_vars, &qap_degree, &qap_num_inputs);

#ifdef DEBUG
    assert(pk.constraint_system.is_satisfied(w));
#endif

    const Fr<ppT> d1 = Fr<ppT>::random_element(),
                  d2 = Fr<ppT>::random_element(),
                  d3 = Fr<ppT>::random_element();

    enter_block("Compute the polynomial H");
    const Fr_vector<ppT> H = qap_witness_map(pk.constraint_system, w, d1, d2, d3);
    leave_block("Compute the polynomial H");

    G1G1_knowledge_commitment<ppT> empty_kc1(G1<ppT>::zero(), G1<ppT>::zero());
    G2G1_knowledge_commitment<ppT> empty_kc2(G2<ppT>::zero(), G1<ppT>::zero());

    G1G1_knowledge_commitment<ppT> g_A = (d1*pk.A_query.get_value(0))+pk.A_query.get_value(3);
    G2G1_knowledge_commitment<ppT> g_B = (d2*pk.B_query.get_value(1))+pk.B_query.get_value(3);
    G1G1_knowledge_commitment<ppT> g_C = (d3*pk.C_query.get_value(2))+pk.C_query.get_value(3);

    G1<ppT> g_H = G1<ppT>::zero();
    G1<ppT> g_K = (d1*pk.K_query[0]) + (d2*pk.K_query[1]) + (d3*pk.K_query[2]) + pk.K_query[3];

#ifdef DEBUG
    for (size_t i = 0; i < qap_num_inputs + 1; ++i)
    {
        assert(pk.A_query.get_value(3+i).g == G1<ppT>::zero());
    }
    assert(pk.A_query.original_size == 3+qap_num_vars+1);
    assert(pk.B_query.original_size == 3+qap_num_vars+1);
    assert(pk.C_query.original_size == 3+qap_num_vars+1);
    assert(pk.H_query.size() == qap_degree+1);
    assert(pk.K_query.size() == 3+qap_num_vars+1);
#endif

#ifdef MULTICORE
    const size_t chunks = 4; //omp_get_max_threads();
#else
    const size_t chunks = 1;
#endif

    enter_block("Compute the proof");

    enter_block("Compute answer to A-query", false);
    g_A = g_A + kc_multi_exp_with_add_special<G1<ppT>, G1<ppT>, Fr<ppT> >(empty_kc1,
                                                                          pk.A_query,
                                                                          4, 4+qap_num_vars,
                                                                          w.begin(), w.begin()+qap_num_vars,
                                                                          chunks, true);
    leave_block("Compute answer to A-query", false);

    enter_block("Compute answer to B-query", false);
    g_B = g_B + kc_multi_exp_with_add_special<G2<ppT>, G1<ppT>, Fr<ppT> >(empty_kc2,
                                                                          pk.B_query,
                                                                          4, 4+qap_num_vars,
                                                                          w.begin(), w.begin()+qap_num_vars,
                                                                          chunks, true);
    leave_block("Compute answer to B-query", false);

    enter_block("Compute answer to C-query", false);
    g_C = g_C + kc_multi_exp_with_add_special<G1<ppT>, G1<ppT>, Fr<ppT> >(empty_kc1,
                                                                          pk.C_query,
                                                                          4, 4+qap_num_vars,
                                                                          w.begin(), w.begin()+qap_num_vars,
                                                                          chunks, true);
    leave_block("Compute answer to C-query", false);

    enter_block("Compute answer to H-query", false);
    g_H = g_H + multi_exp<G1<ppT>, Fr<ppT> >(G1<ppT>::zero(),
                                             pk.H_query.begin(), pk.H_query.begin()+qap_degree+1,
                                             H.begin(), H.begin()+qap_degree+1,
                                             chunks, true);
    leave_block("Compute answer to H-query", false);

    enter_block("Compute answer to K-query", false);
    g_K = g_K + multi_exp_with_add_special<G1<ppT>, Fr<ppT> >(G1<ppT>::zero(),
                                                              pk.K_query.begin()+4, pk.K_query.begin()+4+qap_num_vars,
                                                              w.begin(), w.begin()+qap_num_vars,
                                                              chunks, true);
    leave_block("Compute answer to K-query", false);

    leave_block("Compute the proof");

    leave_block("Call to r1cs_ppzksnark_prover");

    r1cs_ppzksnark_proof<ppT> proof = r1cs_ppzksnark_proof<ppT>(std::move(g_A),
                                         std::move(g_B),
                                         std::move(g_C),
                                         std::move(g_H),
                                         std::move(g_K));
    proof.print_size();

    return proof;
}

template <typename ppT>
r1cs_ppzksnark_processed_verification_key<ppT> r1cs_ppzksnark_verifier_process_vk(const r1cs_ppzksnark_verification_key<ppT> &vk)
{
    enter_block("Call to r1cs_ppzksnark_verifier_process_vk");

    r1cs_ppzksnark_processed_verification_key<ppT> pvk;
    pvk.pp_G2_one_precomp        = precompute_G2<ppT>(G2<ppT>::one());
    pvk.vk_alphaA_g2_precomp     = precompute_G2<ppT>(vk.alphaA_g2);
    pvk.vk_alphaB_g1_precomp     = precompute_G1<ppT>(vk.alphaB_g1);
    pvk.vk_alphaC_g2_precomp     = precompute_G2<ppT>(vk.alphaC_g2);
    pvk.vk_rC_Z_g2_precomp       = precompute_G2<ppT>(vk.rC_Z_g2);
    pvk.vk_gamma_g2_precomp      = precompute_G2<ppT>(vk.gamma_g2);
    pvk.vk_gamma_beta_g1_precomp = precompute_G1<ppT>(vk.gamma_beta_g1);
    pvk.vk_gamma_beta_g2_precomp = precompute_G2<ppT>(vk.gamma_beta_g2);

    pvk.encoded_IC_query = vk.encoded_IC_query;

    leave_block("Call to r1cs_ppzksnark_verifier_process_vk");

    return pvk;
}

template <typename ppT>
bool r1cs_ppzksnark_online_verifier_weak_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                            const r1cs_variable_assignment<Fr<ppT> > &input,
                                            const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_online_verifier_weak_IC");
    assert(pvk.encoded_IC_query->input_size() >= input.size());

    enter_block("Compute input-dependent part of A");
    r1cs_ppzksnark_IC_query<ppT> accumulated_IC = pvk.encoded_IC_query->accumulate(input.begin(), input.end(), 0);
    assert(accumulated_IC.pos.empty());
    G1<ppT> acc = accumulated_IC.base;
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
    G1_precomp<ppT> proof_g_A_g_precomp      = precompute_G1<ppT>(proof.g_A.g);
    G1_precomp<ppT> proof_g_A_h_precomp = precompute_G1<ppT>(proof.g_A.h);
    Fqk<ppT> kc_A_1 = miller_loop<ppT>(proof_g_A_g_precomp,      pvk.vk_alphaA_g2_precomp);
    Fqk<ppT> kc_A_2 = miller_loop<ppT>(proof_g_A_h_precomp, pvk.pp_G2_one_precomp);
    GT<ppT> kc_A = final_exponentiation<ppT>(kc_A_1 * kc_A_2.unitary_inverse());
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
    G2_precomp<ppT> proof_g_B_g_precomp      = precompute_G2<ppT>(proof.g_B.g);
    G1_precomp<ppT> proof_g_B_h_precomp = precompute_G1<ppT>(proof.g_B.h);
    Fqk<ppT> kc_B_1 = miller_loop<ppT>(pvk.vk_alphaB_g1_precomp, proof_g_B_g_precomp);
    Fqk<ppT> kc_B_2 = miller_loop<ppT>(proof_g_B_h_precomp,    pvk.pp_G2_one_precomp);
    GT<ppT> kc_B = final_exponentiation<ppT>(kc_B_1 * kc_B_2.unitary_inverse());
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
    G1_precomp<ppT> proof_g_C_g_precomp      = precompute_G1<ppT>(proof.g_C.g);
    G1_precomp<ppT> proof_g_C_h_precomp = precompute_G1<ppT>(proof.g_C.h);
    Fqk<ppT> kc_C_1 = miller_loop<ppT>(proof_g_C_g_precomp,      pvk.vk_alphaC_g2_precomp);
    Fqk<ppT> kc_C_2 = miller_loop<ppT>(proof_g_C_h_precomp, pvk.pp_G2_one_precomp);
    GT<ppT> kc_C = final_exponentiation<ppT>(kc_C_1 * kc_C_2.unitary_inverse());
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
    G1_precomp<ppT> proof_g_A_g_acc_precomp = precompute_G1<ppT>(proof.g_A.g + acc);
    G1_precomp<ppT> proof_g_H_precomp       = precompute_G1<ppT>(proof.g_H);
    Fqk<ppT> QAP_1  = miller_loop<ppT>(proof_g_A_g_acc_precomp,  proof_g_B_g_precomp);
    Fqk<ppT> QAP_23  = double_miller_loop<ppT>(proof_g_H_precomp, pvk.vk_rC_Z_g2_precomp, proof_g_C_g_precomp, pvk.pp_G2_one_precomp);
    GT<ppT> QAP = final_exponentiation<ppT>(QAP_1 * QAP_23.unitary_inverse());
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
    G1_precomp<ppT> proof_g_K_precomp = precompute_G1<ppT>(proof.g_K);
    G1_precomp<ppT> proof_g_A_g_acc_C_precomp = precompute_G1<ppT>((proof.g_A.g + acc) + proof.g_C.g);
    Fqk<ppT> K_1 = miller_loop<ppT>(proof_g_K_precomp, pvk.vk_gamma_g2_precomp);
    Fqk<ppT> K_23 = double_miller_loop<ppT>(proof_g_A_g_acc_C_precomp, pvk.vk_gamma_beta_g2_precomp, pvk.vk_gamma_beta_g1_precomp, proof_g_B_g_precomp);
    GT<ppT> K = final_exponentiation<ppT>(K_1 * K_23.unitary_inverse());
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
                                     const r1cs_variable_assignment<Fr<ppT> > &input,
                                     const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_verifier_weak_IC");
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = r1cs_ppzksnark_online_verifier_weak_IC<ppT>(pvk, input, proof);
    leave_block("Call to r1cs_ppzksnark_verifier_weak_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_online_verifier_strong_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                              const r1cs_variable_assignment<Fr<ppT> > &input,
                                              const r1cs_ppzksnark_proof<ppT> &proof)
{
    bool result = true;
    enter_block("Call to r1cs_ppzksnark_online_verifier_strong_IC");

    if (pvk.encoded_IC_query->input_size() != input.size())
    {
        print_indent(); printf("Input length differs from expected (got %zu, expected %zu).\n", input.size(), pvk.encoded_IC_query->input_size());
        result = false;
    }
    else
    {
        result = r1cs_ppzksnark_online_verifier_weak_IC(pvk, input, proof);
    }

    leave_block("Call to r1cs_ppzksnark_online_verifier_strong_IC");
    return result;
}

template<typename ppT>
bool r1cs_ppzksnark_verifier_strong_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                       const r1cs_variable_assignment<Fr<ppT> > &input,
                                       const r1cs_ppzksnark_proof<ppT> &proof)
{
    enter_block("Call to r1cs_ppzksnark_verifier_strong_IC");
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = r1cs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, input, proof);
    leave_block("Call to r1cs_ppzksnark_verifier_strong_IC");
    return result;
}

} // libsnark
#endif // R1CS_PPZKSNARK_TCC_
