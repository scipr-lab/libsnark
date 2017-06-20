/** @file
 *****************************************************************************
 Implementation of interfaces for a ppzkSNARK for USCS.

 See uscs_ppzksnark.hpp .
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef USCS_PPZKSNARK_TCC_
#define USCS_PPZKSNARK_TCC_

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>

#include "../../../reductions/uscs_to_ssp/uscs_to_ssp.hpp"
#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>
#include <libff/algebra/scalar_multiplication/multiexp.hpp>
#include "../../../relations/arithmetic_programs/ssp/ssp.hpp"

namespace libsnark {

template<typename ppT>
bool uscs_ppzksnark_proving_key<ppT>::operator==(const uscs_ppzksnark_proving_key<ppT> &other) const
{
    return (this->V_g1_query == other.V_g1_query &&
            this->alpha_V_g1_query == other.alpha_V_g1_query &&
            this->H_g1_query == other.H_g1_query &&
            this->V_g2_query == other.V_g2_query &&
            this->constraint_system == other.constraint_system);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const uscs_ppzksnark_proving_key<ppT> &pk)
{
    out << pk.V_g1_query;
    out << pk.alpha_V_g1_query;
    out << pk.H_g1_query;
    out << pk.V_g2_query;
    out << pk.constraint_system;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, uscs_ppzksnark_proving_key<ppT> &pk)
{
    in >> pk.V_g1_query;
    in >> pk.alpha_V_g1_query;
    in >> pk.H_g1_query;
    in >> pk.V_g2_query;
    in >> pk.constraint_system;

    return in;
}

template<typename ppT>
bool uscs_ppzksnark_verification_key<ppT>::operator==(const uscs_ppzksnark_verification_key<ppT> &other) const
{
    return (this->tilde_g2 == other.tilde_g2 &&
            this->alpha_tilde_g2 == other.alpha_tilde_g2 &&
            this->Z_g2 == other.Z_g2 &&
            this->encoded_IC_query == other.encoded_IC_query);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const uscs_ppzksnark_verification_key<ppT> &vk)
{
    out << vk.tilde_g2 << OUTPUT_NEWLINE;
    out << vk.alpha_tilde_g2 << OUTPUT_NEWLINE;
    out << vk.Z_g2 << OUTPUT_NEWLINE;
    out << vk.encoded_IC_query << OUTPUT_NEWLINE;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, uscs_ppzksnark_verification_key<ppT> &vk)
{
    in >> vk.tilde_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.alpha_tilde_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.Z_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> vk.encoded_IC_query;
    libff::consume_OUTPUT_NEWLINE(in);

    return in;
}

template<typename ppT>
bool uscs_ppzksnark_processed_verification_key<ppT>::operator==(const uscs_ppzksnark_processed_verification_key<ppT> &other) const
{
    return (this->pp_G1_one_precomp == other.pp_G1_one_precomp &&
            this->pp_G2_one_precomp == other.pp_G2_one_precomp &&
            this->vk_tilde_g2_precomp == other.vk_tilde_g2_precomp &&
            this->vk_alpha_tilde_g2_precomp == other.vk_alpha_tilde_g2_precomp &&
            this->vk_Z_g2_precomp == other.vk_Z_g2_precomp &&
            this->pairing_of_g1_and_g2 == other.pairing_of_g1_and_g2 &&
            this->encoded_IC_query == other.encoded_IC_query);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const uscs_ppzksnark_processed_verification_key<ppT> &pvk)
{
    out << pvk.pp_G1_one_precomp << OUTPUT_NEWLINE;
    out << pvk.pp_G2_one_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_tilde_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_alpha_tilde_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.vk_Z_g2_precomp << OUTPUT_NEWLINE;
    out << pvk.pairing_of_g1_and_g2 << OUTPUT_NEWLINE;
    out << pvk.encoded_IC_query << OUTPUT_NEWLINE;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, uscs_ppzksnark_processed_verification_key<ppT> &pvk)
{
    in >> pvk.pp_G1_one_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.pp_G2_one_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_tilde_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_alpha_tilde_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.vk_Z_g2_precomp;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.pairing_of_g1_and_g2;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> pvk.encoded_IC_query;
    libff::consume_OUTPUT_NEWLINE(in);

    return in;
}

template<typename ppT>
bool uscs_ppzksnark_proof<ppT>::operator==(const uscs_ppzksnark_proof<ppT> &other) const
{
    return (this->V_g1 == other.V_g1 &&
            this->alpha_V_g1 == other.alpha_V_g1 &&
            this->H_g1 == other.H_g1 &&
            this->V_g2 == other.V_g2);
}

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const uscs_ppzksnark_proof<ppT> &proof)
{
    out << proof.V_g1 << OUTPUT_NEWLINE;
    out << proof.alpha_V_g1 << OUTPUT_NEWLINE;
    out << proof.H_g1 << OUTPUT_NEWLINE;
    out << proof.V_g2 << OUTPUT_NEWLINE;

    return out;
}

template<typename ppT>
std::istream& operator>>(std::istream &in, uscs_ppzksnark_proof<ppT> &proof)
{
    in >> proof.V_g1;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> proof.alpha_V_g1;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> proof.H_g1;
    libff::consume_OUTPUT_NEWLINE(in);
    in >> proof.V_g2;
    libff::consume_OUTPUT_NEWLINE(in);

    return in;
}

template<typename ppT>
uscs_ppzksnark_verification_key<ppT> uscs_ppzksnark_verification_key<ppT>::dummy_verification_key(const size_t input_size)
{
    uscs_ppzksnark_verification_key<ppT> result;
    result.tilde_g2       = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();
    result.alpha_tilde_g2 = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();
    result.Z_g2           = libff::Fr<ppT>::random_element() * libff::G2<ppT>::one();

    libff::G1<ppT> base = libff::Fr<ppT>::random_element() * libff::G1<ppT>::one();
    libff::G1_vector<ppT> v;
    for (size_t i = 0; i < input_size; ++i)
    {
        v.emplace_back(libff::Fr<ppT>::random_element() * libff::G1<ppT>::one());
    }

    result.encoded_IC_query = accumulation_vector<libff::G1<ppT> >(v);

    return result;
}

template <typename ppT>
uscs_ppzksnark_keypair<ppT> uscs_ppzksnark_generator(const uscs_ppzksnark_constraint_system<ppT> &cs)
{
    libff::enter_block("Call to uscs_ppzksnark_generator");

    /* draw random element at which the SSP is evaluated */

    const  libff::Fr<ppT> t = libff::Fr<ppT>::random_element();

    /* perform USCS-to-SSP reduction */

    ssp_instance_evaluation<libff::Fr<ppT> > ssp_inst = uscs_to_ssp_instance_map_with_evaluation(cs, t);

    libff::print_indent(); printf("* SSP number of variables: %zu\n", ssp_inst.num_variables());
    libff::print_indent(); printf("* SSP pre degree: %zu\n", cs.num_constraints());
    libff::print_indent(); printf("* SSP degree: %zu\n", ssp_inst.degree());
    libff::print_indent(); printf("* SSP number of input variables: %zu\n", ssp_inst.num_inputs());

    /* construct various tables of FieldT elements */

    libff::Fr_vector<ppT> Vt_table = std::move(ssp_inst.Vt); // ssp_inst.Vt is now in unspecified state, but we do not use it later
    libff::Fr_vector<ppT> Ht_table = std::move(ssp_inst.Ht); // ssp_inst.Ht is now in unspecified state, but we do not use it later

    Vt_table.emplace_back(ssp_inst.Zt);

    libff::Fr_vector<ppT> Xt_table = libff::Fr_vector<ppT>(Vt_table.begin(), Vt_table.begin() + ssp_inst.num_inputs() + 1);
    libff::Fr_vector<ppT> Vt_table_minus_Xt_table = libff::Fr_vector<ppT>(Vt_table.begin() + ssp_inst.num_inputs() + 1, Vt_table.end());

    /* sanity checks */

    assert(Vt_table.size() == ssp_inst.num_variables() + 2);
    printf("Ht_table.size() = %zu, ssp_inst.degree() + 1 = %zu\n", Ht_table.size(), ssp_inst.degree() + 1);
    assert(Ht_table.size() == ssp_inst.degree() + 1);
    assert(Xt_table.size() == ssp_inst.num_inputs() + 1);
    assert(Vt_table_minus_Xt_table.size() == ssp_inst.num_variables() + 2 - ssp_inst.num_inputs() - 1);
    for (size_t i = 0; i < ssp_inst.num_inputs()+1; ++i)
    {
        assert(!Xt_table[i].is_zero());
    }

    const libff::Fr<ppT> alpha = libff::Fr<ppT>::random_element();

    libff::enter_block("Generate USCS proving key");

    const size_t g1_exp_count = Vt_table.size() + Vt_table_minus_Xt_table.size() + Ht_table.size();
    const size_t g2_exp_count = Vt_table_minus_Xt_table.size();

    size_t g1_window = libff::get_exp_window_size<libff::G1<ppT> >(g1_exp_count);
    size_t g2_window = libff::get_exp_window_size<libff::G2<ppT> >(g2_exp_count);

    libff::print_indent(); printf("* G1 window: %zu\n", g1_window);
    libff::print_indent(); printf("* G2 window: %zu\n", g2_window);

    libff::enter_block("Generating G1 multiexp table");
    libff::window_table<libff::G1<ppT> > g1_table = get_window_table(libff::Fr<ppT>::size_in_bits(), g1_window, libff::G1<ppT>::one());
    libff::leave_block("Generating G1 multiexp table");

    libff::enter_block("Generating G2 multiexp table");
    libff::window_table<libff::G2<ppT> > g2_table = get_window_table(libff::Fr<ppT>::size_in_bits(), g2_window, libff::G2<ppT>::one());
    libff::leave_block("Generating G2 multiexp table");

    libff::enter_block("Generate proof components");

    libff::enter_block("Compute the query for V_g1", false);
    libff::G1_vector<ppT> V_g1_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Vt_table_minus_Xt_table);
    libff::leave_block("Compute the query for V_g1", false);

    libff::enter_block("Compute the query for alpha_V_g1", false);
    libff::G1_vector<ppT> alpha_V_g1_query = batch_exp_with_coeff(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, alpha, Vt_table_minus_Xt_table);
    libff::leave_block("Compute the query for alpha_V_g1", false);

    libff::enter_block("Compute the query for H_g1", false);
    libff::G1_vector<ppT> H_g1_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Ht_table);
    libff::leave_block("Compute the query for H_g1", false);

    libff::enter_block("Compute the query for V_g2", false);
    libff::G2_vector<ppT> V_g2_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g2_window, g2_table, Vt_table);
    libff::leave_block("Compute the query for V_g2", false);

    libff::leave_block("Generate proof components");

    libff::leave_block("Generate USCS proving key");

    libff::enter_block("Generate USCS verification key");

    const libff::Fr<ppT> tilde    = libff::Fr<ppT>::random_element();
    libff::G2<ppT> tilde_g2       = tilde * libff::G2<ppT>::one();
    libff::G2<ppT> alpha_tilde_g2 = (alpha * tilde) * libff::G2<ppT>::one();
    libff::G2<ppT> Z_g2           = ssp_inst.Zt * libff::G2<ppT>::one();

    libff::enter_block("Encode IC query for USCS verification key");
    libff::G1<ppT> encoded_IC_base = Xt_table[0] * libff::G1<ppT>::one();
    libff::G1_vector<ppT> encoded_IC_values = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, libff::Fr_vector<ppT>(Xt_table.begin() + 1, Xt_table.end()));
    libff::leave_block("Encode IC query for USCS verification key");

    libff::leave_block("Generate USCS verification key");

    libff::leave_block("Call to uscs_ppzksnark_generator");

    accumulation_vector<libff::G1<ppT> > encoded_IC_query(std::move(encoded_IC_base), std::move(encoded_IC_values));

    uscs_ppzksnark_verification_key<ppT> vk = uscs_ppzksnark_verification_key<ppT>(tilde_g2,
                                                                                   alpha_tilde_g2,
                                                                                   Z_g2,
                                                                                   encoded_IC_query);

    uscs_ppzksnark_constraint_system<ppT> cs_copy = cs;
    uscs_ppzksnark_proving_key<ppT> pk = uscs_ppzksnark_proving_key<ppT>(std::move(V_g1_query),
                                                                         std::move(alpha_V_g1_query),
                                                                         std::move(H_g1_query),
                                                                         std::move(V_g2_query),
                                                                         std::move(cs_copy));

    pk.print_size();
    vk.print_size();

    return uscs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk));
}

template <typename ppT>
uscs_ppzksnark_proof<ppT> uscs_ppzksnark_prover(const uscs_ppzksnark_proving_key<ppT> &pk,
                                                const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                const uscs_ppzksnark_auxiliary_input<ppT> &auxiliary_input)
{
    libff::enter_block("Call to uscs_ppzksnark_prover");

    const libff::Fr<ppT> d = libff::Fr<ppT>::random_element();

    libff::enter_block("Compute the polynomial H");
    const ssp_witness<libff::Fr<ppT> > ssp_wit = uscs_to_ssp_witness_map(pk.constraint_system, primary_input, auxiliary_input, d);
    libff::leave_block("Compute the polynomial H");

    /* sanity checks */
    assert(pk.constraint_system.is_satisfied(primary_input, auxiliary_input));
    assert(pk.V_g1_query.size() == ssp_wit.num_variables() + 2 - ssp_wit.num_inputs() - 1);
    assert(pk.alpha_V_g1_query.size() == ssp_wit.num_variables() + 2 - ssp_wit.num_inputs() - 1);
    assert(pk.H_g1_query.size() == ssp_wit.degree() + 1);
    assert(pk.V_g2_query.size() == ssp_wit.num_variables() + 2);

#ifdef DEBUG
    const libff::Fr<ppT> t = libff::Fr<ppT>::random_element();
    ssp_instance_evaluation<libff::Fr<ppT> > ssp_inst = uscs_to_ssp_instance_map_with_evaluation(pk.constraint_system, t);
    assert(ssp_inst.is_satisfied(ssp_wit));
#endif

    libff::G1<ppT> V_g1       = ssp_wit.d*pk.V_g1_query[pk.V_g1_query.size()-1];
    libff::G1<ppT> alpha_V_g1 = ssp_wit.d*pk.alpha_V_g1_query[pk.alpha_V_g1_query.size()-1];
    libff::G1<ppT> H_g1       = libff::G1<ppT>::zero();
    libff::G2<ppT> V_g2       = pk.V_g2_query[0]+ssp_wit.d*pk.V_g2_query[pk.V_g2_query.size()-1];

#ifdef MULTICORE
    const size_t chunks = omp_get_max_threads(); // to override, set OMP_NUM_THREADS env var or call omp_set_num_threads()
#else
    const size_t chunks = 1;
#endif

    // MAYBE LATER: do queries 1,2,4 at once for slightly better speed

    libff::enter_block("Compute the proof");

    libff::enter_block("Compute V_g1, the 1st component of the proof", false);
    V_g1 = V_g1 + libff::multi_exp_with_mixed_addition<libff::G1<ppT>, libff::Fr<ppT> >(pk.V_g1_query.begin(), pk.V_g1_query.begin()+(ssp_wit.num_variables()-ssp_wit.num_inputs()),
                                                                   ssp_wit.coefficients_for_Vs.begin()+ssp_wit.num_inputs(), ssp_wit.coefficients_for_Vs.begin()+ssp_wit.num_variables(),
                                                                   chunks,
                                                                   true);
    libff::leave_block("Compute V_g1, the 1st component of the proof", false);

    libff::enter_block("Compute alpha_V_g1, the 2nd component of the proof", false);
    alpha_V_g1 = alpha_V_g1 + libff::multi_exp_with_mixed_addition<libff::G1<ppT>, libff::Fr<ppT> >(pk.alpha_V_g1_query.begin(), pk.alpha_V_g1_query.begin()+(ssp_wit.num_variables()-ssp_wit.num_inputs()),
                                                                               ssp_wit.coefficients_for_Vs.begin()+ssp_wit.num_inputs(), ssp_wit.coefficients_for_Vs.begin()+ssp_wit.num_variables(),
                                                                               chunks,
                                                                               true);
    libff::leave_block("Compute alpha_V_g1, the 2nd component of the proof", false);

    libff::enter_block("Compute H_g1, the 3rd component of the proof", false);
    H_g1 = H_g1 + libff::multi_exp<libff::G1<ppT>, libff::Fr<ppT> >(pk.H_g1_query.begin(), pk.H_g1_query.begin()+ssp_wit.degree()+1,
                                               ssp_wit.coefficients_for_H.begin(), ssp_wit.coefficients_for_H.begin()+ssp_wit.degree()+1,
                                               chunks,
                                               true);
    libff::leave_block("Compute H_g1, the 3rd component of the proof", false);

    libff::enter_block("Compute V_g2, the 4th component of the proof", false);
    V_g2 = V_g2 + libff::multi_exp<libff::G2<ppT>, libff::Fr<ppT> >(pk.V_g2_query.begin()+1, pk.V_g2_query.begin()+ssp_wit.num_variables()+1,
                                               ssp_wit.coefficients_for_Vs.begin(), ssp_wit.coefficients_for_Vs.begin()+ssp_wit.num_variables(),
                                               chunks,
                                               true);
    libff::leave_block("Compute V_g2, the 4th component of the proof", false);

    libff::leave_block("Compute the proof");

    libff::leave_block("Call to uscs_ppzksnark_prover");

    uscs_ppzksnark_proof<ppT> proof = uscs_ppzksnark_proof<ppT>(std::move(V_g1), std::move(alpha_V_g1), std::move(H_g1), std::move(V_g2));

    proof.print_size();

    return proof;
}

template <typename ppT>
uscs_ppzksnark_processed_verification_key<ppT> uscs_ppzksnark_verifier_process_vk(const uscs_ppzksnark_verification_key<ppT> &vk)
{
    libff::enter_block("Call to uscs_ppzksnark_verifier_process_vk");

    uscs_ppzksnark_processed_verification_key<ppT> pvk;

    pvk.pp_G1_one_precomp         = ppT::precompute_G1(libff::G1<ppT>::one());
    pvk.pp_G2_one_precomp         = ppT::precompute_G2(libff::G2<ppT>::one());

    pvk.vk_tilde_g2_precomp       = ppT::precompute_G2(vk.tilde_g2);
    pvk.vk_alpha_tilde_g2_precomp = ppT::precompute_G2(vk.alpha_tilde_g2);
    pvk.vk_Z_g2_precomp           = ppT::precompute_G2(vk.Z_g2);

    pvk.pairing_of_g1_and_g2      = ppT::miller_loop(pvk.pp_G1_one_precomp,pvk.pp_G2_one_precomp);

    pvk.encoded_IC_query = vk.encoded_IC_query;

    libff::leave_block("Call to uscs_ppzksnark_verifier_process_vk");

    return pvk;
}

template <typename ppT>
bool uscs_ppzksnark_online_verifier_weak_IC(const uscs_ppzksnark_processed_verification_key<ppT> &pvk,
                                            const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                            const uscs_ppzksnark_proof<ppT> &proof)
{
    libff::enter_block("Call to uscs_ppzksnark_online_verifier_weak_IC");
    assert(pvk.encoded_IC_query.domain_size() >= primary_input.size());

    libff::enter_block("Compute input-dependent part of V");
    const accumulation_vector<libff::G1<ppT> > accumulated_IC = pvk.encoded_IC_query.template accumulate_chunk<libff::Fr<ppT> >(primary_input.begin(), primary_input.end(), 0);
    assert(accumulated_IC.is_fully_accumulated());
    const libff::G1<ppT> &acc = accumulated_IC.first;
    libff::leave_block("Compute input-dependent part of V");

    bool result = true;

    libff::enter_block("Check if the proof is well-formed");
    if (!proof.is_well_formed())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("At least one of the proof components is not well-formed.\n");
        }
        result = false;
    }
    libff::leave_block("Check if the proof is well-formed");

    libff::enter_block("Online pairing computations");

    libff::enter_block("Check knowledge commitment for V is valid");
    libff::G1_precomp<ppT> proof_V_g1_with_acc_precomp = ppT::precompute_G1(proof.V_g1 + acc);
    libff::G2_precomp<ppT> proof_V_g2_precomp = ppT::precompute_G2(proof.V_g2);
    libff::Fqk<ppT> V_1 = ppT::miller_loop(proof_V_g1_with_acc_precomp,    pvk.pp_G2_one_precomp);
    libff::Fqk<ppT> V_2 = ppT::miller_loop(pvk.pp_G1_one_precomp, proof_V_g2_precomp);
    libff::GT<ppT> V = ppT::final_exponentiation(V_1 * V_2.unitary_inverse());
    if (V != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("Knowledge commitment for V invalid.\n");
        }
        result = false;
    }
    libff::leave_block("Check knowledge commitment for V is valid");

    libff::enter_block("Check SSP divisibility"); // i.e., check that V^2=H*Z+1
    libff::G1_precomp<ppT> proof_H_g1_precomp = ppT::precompute_G1(proof.H_g1);
    libff::Fqk<ppT> SSP_1  = ppT::miller_loop(proof_V_g1_with_acc_precomp,  proof_V_g2_precomp);
    libff::Fqk<ppT> SSP_2  = ppT::miller_loop(proof_H_g1_precomp, pvk.vk_Z_g2_precomp);
    libff::GT<ppT> SSP = ppT::final_exponentiation(SSP_1.unitary_inverse() * SSP_2 * pvk.pairing_of_g1_and_g2);
    if (SSP != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("SSP divisibility check failed.\n");
        }
        result = false;
    }
    libff::leave_block("Check SSP divisibility");

    libff::enter_block("Check same coefficients were used");
    libff::G1_precomp<ppT> proof_V_g1_precomp = ppT::precompute_G1(proof.V_g1);
    libff::G1_precomp<ppT> proof_alpha_V_g1_precomp = ppT::precompute_G1(proof.alpha_V_g1);
    libff::Fqk<ppT> alpha_V_1 = ppT::miller_loop(proof_V_g1_precomp, pvk.vk_alpha_tilde_g2_precomp);
    libff::Fqk<ppT> alpha_V_2 = ppT::miller_loop(proof_alpha_V_g1_precomp, pvk.vk_tilde_g2_precomp);
    libff::GT<ppT> alpha_V = ppT::final_exponentiation(alpha_V_1 * alpha_V_2.unitary_inverse());
    if (alpha_V != libff::GT<ppT>::one())
    {
        if (!libff::inhibit_profiling_info)
        {
            libff::print_indent(); printf("Same-coefficient check failed.\n");
        }
        result = false;
    }
    libff::leave_block("Check same coefficients were used");

    libff::leave_block("Online pairing computations");

    libff::leave_block("Call to uscs_ppzksnark_online_verifier_weak_IC");

    return result;
}

template<typename ppT>
bool uscs_ppzksnark_verifier_weak_IC(const uscs_ppzksnark_verification_key<ppT> &vk,
                                     const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                     const uscs_ppzksnark_proof<ppT> &proof)
{
    libff::enter_block("Call to uscs_ppzksnark_verifier_weak_IC");
    uscs_ppzksnark_processed_verification_key<ppT> pvk = uscs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = uscs_ppzksnark_online_verifier_weak_IC<ppT>(pvk, primary_input, proof);
    libff::leave_block("Call to uscs_ppzksnark_verifier_weak_IC");
    return result;
}

template<typename ppT>
bool uscs_ppzksnark_online_verifier_strong_IC(const uscs_ppzksnark_processed_verification_key<ppT> &pvk,
                                              const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                              const uscs_ppzksnark_proof<ppT> &proof)
{
    bool result = true;
    libff::enter_block("Call to uscs_ppzksnark_online_verifier_strong_IC");

    if (pvk.encoded_IC_query.domain_size() != primary_input.size())
    {
        libff::print_indent(); printf("Input length differs from expected (got %zu, expected %zu).\n", primary_input.size(), pvk.encoded_IC_query.domain_size());
        result = false;
    }
    else
    {
        result = uscs_ppzksnark_online_verifier_weak_IC(pvk, primary_input, proof);
    }

    libff::leave_block("Call to uscs_ppzksnark_online_verifier_strong_IC");
    return result;
}

template<typename ppT>
bool uscs_ppzksnark_verifier_strong_IC(const uscs_ppzksnark_verification_key<ppT> &vk,
                                       const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                       const uscs_ppzksnark_proof<ppT> &proof)
{
    libff::enter_block("Call to uscs_ppzksnark_verifier_strong_IC");
    uscs_ppzksnark_processed_verification_key<ppT> pvk = uscs_ppzksnark_verifier_process_vk<ppT>(vk);
    bool result = uscs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, primary_input, proof);
    libff::leave_block("Call to uscs_ppzksnark_verifier_strong_IC");
    return result;
}

} // libsnark

#endif // USCS_PPZKSNARK_TCC_
