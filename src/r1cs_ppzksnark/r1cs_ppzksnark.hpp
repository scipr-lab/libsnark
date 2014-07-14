/** @file
 *****************************************************************************
 Declaration of interfaces for a ppzkSNARK for R1CS.

 This includes:
 - class for proving key
 - class for verification key
 - class for processed verification key
 - class for key pair (proving key & verification key)
 - class for proof
 - generator algorithm
 - prover algorithm
 - verifier algorithm (with strong or weak input consistency)
 - online verifier algorithm (with strong or weak input consistency)

 Acronyms:
 - R1CS = "Rank-1 Constraint Systems"
 - ppzkSNARK = "PreProcessing Zero-Knowledge Succinct Non-interactive ARgument of Knowledge"

 The implementation follows, extends, and optimizes the basic approach described in \[BCTV14].

 \[BCTV14]:
 "Succinct Non-Interactive Zero Knowledge for a von Neumann Architecture",
 Eli Ben-Sasson, Alessandro Chiesa, Eran Tromer, Madars Virza,
 USENIX Security 2014,
 <http://eprint.iacr.org/2013/879>
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_PPZKSNARK_HPP_
#define R1CS_PPZKSNARK_HPP_

#include <memory>

#include "algebra/curves/public_params.hpp"
#include "encoding/knowledge_commitment.hpp"
#include "r1cs/r1cs.hpp"

namespace libsnark {

/******************************** Proving key ********************************/

template<typename ppT>
class r1cs_ppzksnark_proving_key;

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_proving_key<ppT> &pk);

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_proving_key<ppT> &pk);

/**
 * A proving key for the R1CS ppzkSNARK
 */
template<typename ppT>
class r1cs_ppzksnark_proving_key {
public:
    G1G1_knowledge_commitment_vector<ppT> A_query;
    G2G1_knowledge_commitment_vector<ppT> B_query;
    G1G1_knowledge_commitment_vector<ppT> C_query;
    G1_vector<ppT> H_query;
    G1_vector<ppT> K_query;

    r1cs_constraint_system<Fr<ppT> > constraint_system;

    r1cs_ppzksnark_proving_key() {};
    r1cs_ppzksnark_proving_key<ppT>& operator=(const r1cs_ppzksnark_proving_key<ppT> &other) = default;
    r1cs_ppzksnark_proving_key(const r1cs_ppzksnark_proving_key<ppT> &other) = default;
    r1cs_ppzksnark_proving_key(r1cs_ppzksnark_proving_key<ppT> &&other) = default;
    r1cs_ppzksnark_proving_key(G1G1_knowledge_commitment_vector<ppT> &&A_query,
                     G2G1_knowledge_commitment_vector<ppT> &&B_query,
                     G1G1_knowledge_commitment_vector<ppT> &&C_query,
                     G1_vector<ppT> &&H_query,
                     G1_vector<ppT> &&K_query,
                     r1cs_constraint_system<Fr<ppT> > &&constraint_system) :
        A_query(std::move(A_query)),
        B_query(std::move(B_query)),
        C_query(std::move(C_query)),
        H_query(std::move(H_query)),
        K_query(std::move(K_query)),
        constraint_system(std::move(constraint_system))
    {};

    size_t G1_size() const
    {
        return 2*(A_query.original_size + C_query.original_size) + B_query.original_size + H_query.size() + K_query.size();
    }

    size_t G2_size() const
    {
        return B_query.original_size;
    }

    size_t G1_sparse_size() const
    {
        return 2*(A_query.sparse_size() + C_query.sparse_size()) + B_query.sparse_size() + H_query.size() + K_query.size();
    }

    size_t G2_sparse_size() const
    {
        return B_query.sparse_size();
    }

    size_t size_in_bits() const
    {
        return A_query.size_in_bits() + B_query.size_in_bits() + C_query.size_in_bits() + G1<ppT>::size_in_bits() * (H_query.size() + K_query.size()) + 2 * 8 * sizeof(size_t);
    }

    void print_size() const
    {
        print_indent(); printf("* G1 elements in PK: %zu\n", this->G1_size());
        print_indent(); printf("* Non-zero G1 elements in PK: %zu\n", this->G1_sparse_size());
        print_indent(); printf("* G2 elements in PK: %zu\n", this->G2_size());
        print_indent(); printf("* Non-zero G2 elements in PK: %zu\n", this->G2_sparse_size());
        print_indent(); printf("* PK size in bits: %zu\n", this->size_in_bits());
    }

    bool operator==(const r1cs_ppzksnark_proving_key<ppT> &other) const;
    friend std::ostream& operator<< <ppT>(std::ostream &out, const r1cs_ppzksnark_proving_key<ppT> &pk);
    friend std::istream& operator>> <ppT>(std::istream &in, r1cs_ppzksnark_proving_key<ppT> &pk);
};

/********************** Input-consistency query  ********************************/

template<typename ppT>
class r1cs_ppzksnark_IC_query;

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_IC_query<ppT> &pk);

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_IC_query<ppT> &pk);

/**
 * An input-consistency (IC) query for R1CS ppzkSNARK (used inside r1cs_ppzksnark_verification_key)
 */
template<typename ppT>
class r1cs_ppzksnark_IC_query {
public:
    G1<ppT> base;

    std::vector<size_t> pos;
    G1_vector<ppT> encoded_terms;

    r1cs_ppzksnark_IC_query() = default;
    r1cs_ppzksnark_IC_query(const G1<ppT> &encoded_IC_base,const G1_vector<ppT> &v) :
        base(encoded_IC_base), encoded_terms(v)
    {
        pos.reserve(encoded_terms.size());

        for (size_t i = 0; i < encoded_terms.size(); ++i)
        {
            pos.emplace_back(i);
        }
    }

    size_t input_size() const { return pos.size(); }
    size_t G1_size() const { return (1 + pos.size()); }
    /* this assumes that IC_query_terms form a contiguous vector, so we only need 2 * 8 * sizeof(size_t) bits to encode them (first + size) */
    size_t sparse_size_in_bits() const { return G1<ppT>::size_in_bits() * this->G1_size() + 2 * 8 * sizeof(size_t) ; }
    size_t dense_size_in_bits() const { return G1<ppT>::size_in_bits() * this->G1_size(); }
    r1cs_ppzksnark_IC_query accumulate(const typename Fr_vector<ppT>::const_iterator w_begin,
                             const typename Fr_vector<ppT>::const_iterator w_end,
                             const size_t offset) const;

    bool operator==(const r1cs_ppzksnark_IC_query<ppT> &other) const;
    friend std::ostream& operator<< <ppT>(std::ostream &out, const r1cs_ppzksnark_IC_query<ppT> &q);
    friend std::istream& operator>> <ppT>(std::istream &in, r1cs_ppzksnark_IC_query<ppT> &q);
};

/***************************** Verification key **************************/

template<typename ppT>
class r1cs_ppzksnark_verification_key;

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_verification_key<ppT> &vk);

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_verification_key<ppT> &vk);

/**
 * A verification key for R1CS ppzkSNARK
 */
template<typename ppT>
class r1cs_ppzksnark_verification_key {
public:
    G2<ppT> alphaA_g2;
    G1<ppT> alphaB_g1;
    G2<ppT> alphaC_g2;
    G2<ppT> gamma_g2;
    G1<ppT> gamma_beta_g1;
    G2<ppT> gamma_beta_g2;
    G2<ppT> rC_Z_g2;

    std::shared_ptr<r1cs_ppzksnark_IC_query<ppT> > encoded_IC_query;

    r1cs_ppzksnark_verification_key() = default;
    r1cs_ppzksnark_verification_key(const G2<ppT> &alphaA_g2,
                          const G1<ppT> &alphaB_g1,
                          const G2<ppT> &alphaC_g2,
                          const G2<ppT> &gamma_g2,
                          const G1<ppT> &gamma_beta_g1,
                          const G2<ppT> &gamma_beta_g2,
                          const G2<ppT> &rC_Z_g2,
                          r1cs_ppzksnark_IC_query<ppT>* eIC) :
        alphaA_g2(alphaA_g2),
        alphaB_g1(alphaB_g1),
        alphaC_g2(alphaC_g2),
        gamma_g2(gamma_g2),
        gamma_beta_g1(gamma_beta_g1),
        gamma_beta_g2(gamma_beta_g2),
        rC_Z_g2(rC_Z_g2),
        encoded_IC_query(eIC)
    {};

    size_t G1_size() const
    {
        return 2 + encoded_IC_query->G1_size();
    }

    size_t G2_size() const
    {
        return 5;
    }

    size_t sparse_size_in_bits() const
    {
        return 2 * G1<ppT>::size_in_bits() + 5 * G2<ppT>::size_in_bits() + encoded_IC_query->sparse_size_in_bits();
    }

    size_t dense_size_in_bits() const
    {
        return 2 * G1<ppT>::size_in_bits() + 5 * G2<ppT>::size_in_bits() + encoded_IC_query->dense_size_in_bits();
    }

    void print_size() const
    {
        print_indent(); printf("* G1 elements in VK: %zu\n", this->G1_size());
        print_indent(); printf("* G2 elements in VK: %zu\n", this->G2_size());
        print_indent(); printf("* Dense VK size in bits: %zu\n", this->dense_size_in_bits());
        print_indent(); printf("* Sparse VK size in bits: %zu\n", this->sparse_size_in_bits());
    }

    bool operator==(const r1cs_ppzksnark_verification_key<ppT> &other) const;
    friend std::ostream& operator<< <ppT>(std::ostream &out, const r1cs_ppzksnark_verification_key<ppT> &vk);
    friend std::istream& operator>> <ppT>(std::istream &in, r1cs_ppzksnark_verification_key<ppT> &vk);

    static r1cs_ppzksnark_verification_key<ppT> dummy_verification_key(const size_t input_size);
};

/************************* Processed verification key **************************/

template<typename ppT>
class r1cs_ppzksnark_processed_verification_key;

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_processed_verification_key<ppT> &vk);

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_processed_verification_key<ppT> &vk);

/**
 * A processed verification key for R1CS ppzkSNARK.
 * This key contains preprocessed information about the verification key,
 * to speed up its use in verification (as explained in \[BCTV14]).
 */
template<typename ppT>
class r1cs_ppzksnark_processed_verification_key {
public:
    G2_precomp<ppT> pp_G2_one_precomp;
    G2_precomp<ppT> vk_alphaA_g2_precomp;
    G1_precomp<ppT> vk_alphaB_g1_precomp;
    G2_precomp<ppT> vk_alphaC_g2_precomp;
    G2_precomp<ppT> vk_rC_Z_g2_precomp;
    G2_precomp<ppT> vk_gamma_g2_precomp;
    G1_precomp<ppT> vk_gamma_beta_g1_precomp;
    G2_precomp<ppT> vk_gamma_beta_g2_precomp;

    std::shared_ptr<r1cs_ppzksnark_IC_query<ppT> > encoded_IC_query;

    bool operator==(const r1cs_ppzksnark_processed_verification_key &other) const;
    friend std::ostream& operator<< <ppT>(std::ostream &out, const r1cs_ppzksnark_processed_verification_key<ppT> &pvk);
    friend std::istream& operator>> <ppT>(std::istream &in, r1cs_ppzksnark_processed_verification_key<ppT> &pvk);
};

/********************************* Key pair ********************************/

/**
 * A pair of proving key and verification key for the R1CS ppzkSNARK
 */
template<typename ppT>
class r1cs_ppzksnark_keypair {
public:
    r1cs_ppzksnark_proving_key<ppT> pk;
    r1cs_ppzksnark_verification_key<ppT> vk;

    r1cs_ppzksnark_keypair() {};
    r1cs_ppzksnark_keypair(r1cs_ppzksnark_keypair<ppT> &&other) = default;
    r1cs_ppzksnark_keypair(r1cs_ppzksnark_verification_key<ppT> &&vk, r1cs_ppzksnark_proving_key<ppT> &&pk) :
        pk(std::move(pk)),
        vk(std::move(vk))
    {}
};


/******************************* Proof ******************************/

template<typename ppT>
class r1cs_ppzksnark_proof;

template<typename ppT>
std::ostream& operator<<(std::ostream &out, const r1cs_ppzksnark_proof<ppT> &pk);

template<typename ppT>
std::istream& operator>>(std::istream &in, r1cs_ppzksnark_proof<ppT> &pk);

/**
 * A proof in the R1CS ppZKSNARK.
 * Internally the proof has a structure, but externally you just opaquely
 * produce, de/serailize and verify it.
 * We expose some information about the size of the internal structure,
 * for statistics.
 */
template<typename ppT>
class r1cs_ppzksnark_proof {
public:
    G1G1_knowledge_commitment<ppT> g_A;
    G2G1_knowledge_commitment<ppT> g_B;
    G1G1_knowledge_commitment<ppT> g_C;
    G1<ppT> g_H;
    G1<ppT> g_K;

    r1cs_ppzksnark_proof()
    {
        // invalid proof with valid curve points
        this->g_A.g = G1<ppT> ::one();
        this->g_A.h = G1<ppT>::one();
        this->g_B.g = G2<ppT> ::one();
        this->g_B.h = G1<ppT>::one();
        this->g_C.g = G1<ppT> ::one();
        this->g_C.h = G1<ppT>::one();
        this->g_H = G1<ppT>::one();
        this->g_K = G1<ppT>::one();
    }
    r1cs_ppzksnark_proof(G1G1_knowledge_commitment<ppT> &&g_A,
               G2G1_knowledge_commitment<ppT> &&g_B,
               G1G1_knowledge_commitment<ppT> &&g_C,
               G1<ppT> &&g_H,
               G1<ppT> &&g_K) :
        g_A(std::move(g_A)),
        g_B(std::move(g_B)),
        g_C(std::move(g_C)),
        g_H(std::move(g_H)),
        g_K(std::move(g_K))
    {};

    size_t G1_size() const
    {
        return 7;
    }

    size_t G2_size() const
    {
        return 1;
    }

    size_t size_in_bits() const
    {
        return 7 * G1<ppT>::size_in_bits() + 1 * G2<ppT>::size_in_bits();
    }

    void print_size() const
    {
        print_indent(); printf("* G1 elements in proof: %zu\n", this->G1_size());
        print_indent(); printf("* G2 elements in proof: %zu\n", this->G2_size());
        print_indent(); printf("* Proof size in bits: %zu\n", this->size_in_bits());
    }

    bool is_well_formed() const
    {
        return (g_A.g.is_well_formed() && g_A.h.is_well_formed() &&
                g_B.g.is_well_formed() && g_B.h.is_well_formed() &&
                g_C.g.is_well_formed() && g_C.h.is_well_formed() &&
                g_H.is_well_formed() &&
                g_K.is_well_formed());
    }

    bool operator==(const r1cs_ppzksnark_proof<ppT> &other) const;
    friend std::ostream& operator<< <ppT>(std::ostream &out, const r1cs_ppzksnark_proof<ppT> &proof);
    friend std::istream& operator>> <ppT>(std::istream &in, r1cs_ppzksnark_proof<ppT> &proof);
};

/********************** Main algorithms of the R1CS ppzkSNARK *******************/

/**
 * R1CS ppZKSNARK generator algorithm.
 * This algorithm produces the proving and verification keys. It needs to be
 * ran just once per constraint system.
 */
template<typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator(const r1cs_constraint_system<Fr<ppT> > &cs);


/**
 * R1CS ppZKSNARK prover algorithm.
 * It proves a particular statement, expressed as an R1CS, using a witness
 * (an assignment that satisfies the R1CS). It produces a proof string,
 * to be passed to the verifier.
 */
template<typename ppT>
r1cs_ppzksnark_proof<ppT> r1cs_ppzksnark_prover(const r1cs_ppzksnark_proving_key<ppT> &pk,
                                      const r1cs_variable_assignment<Fr<ppT> > &witness);


/* Several variants of the verifier algorithm: */

/**
 * R1CS ppzkSNARK verifier algorithm (given an un-processed verification key),
 * with "weak" input consistency.
 *
 * Weak input consistency guarantees that input' := input || 0^{n-|input|}
 * can be extended to a satisfiable assignment for the R1CS from which vk
 * was created. (Here, n denotes the number of inputs to the R1CS.)
*/
template<typename ppT>
bool r1cs_ppzksnark_verifier_weak_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                     const r1cs_variable_assignment<Fr<ppT> > &input,
                                     const r1cs_ppzksnark_proof<ppT> &proof);
/**
 * R1CS ppzkSNARK verifier algorithm (given an un-processed verification key),
 * with "strong" input consistency.
 *
 * Strong input consistency requires input to contain n field elements, i.e.,
 * disallows input to be padded with 0's up to length n.
*/
template<typename ppT>
bool r1cs_ppzksnark_verifier_strong_IC(const r1cs_ppzksnark_verification_key<ppT> &vk,
                                       const r1cs_variable_assignment<Fr<ppT> > &input,
                                       const r1cs_ppzksnark_proof<ppT> &proof);

/**
 * Converts an (un-processed) verification key into a processed verification key,
 * to make subsequent "online" verification faster.
 */
template<typename ppT>
r1cs_ppzksnark_processed_verification_key<ppT> r1cs_ppzksnark_verifier_process_vk(const r1cs_ppzksnark_verification_key<ppT> &vk);

/**
 * R1CS ppzkSNARK online verifier algorithm (given a processed verification key),
 * with "weak" input consistency.
*/
template<typename ppT>
bool r1cs_ppzksnark_online_verifier_weak_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                            const r1cs_variable_assignment<Fr<ppT> > &input,
                                            const r1cs_ppzksnark_proof<ppT> &proof);
/**
 * R1CS ppzkSNARK online verifier algorithm (given a processed verification key),
 * with "strong" input consistency.
*/
template<typename ppT>
bool r1cs_ppzksnark_online_verifier_strong_IC(const r1cs_ppzksnark_processed_verification_key<ppT> &pvk,
                                              const r1cs_variable_assignment<Fr<ppT> > &input,
                                              const r1cs_ppzksnark_proof<ppT> &proof);

} // libsnark
#include "r1cs_ppzksnark/r1cs_ppzksnark.tcc"

#endif // R1CS_PPZKSNARK_HPP_
