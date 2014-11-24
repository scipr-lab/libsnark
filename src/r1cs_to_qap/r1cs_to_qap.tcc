/** @file
 *****************************************************************************

 Implementation of interfaces for a R1CS-to-QAP reduction.

 See r1cs_to_qap.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_TO_QAP_TCC_
#define R1CS_TO_QAP_TCC_

#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "evaluation_domain/evaluation_domain.hpp"

#define R1CS_TO_QAP_ADDITIONAL_CONSTRAINTS 1 // +1 is for an additional constraint needed for the soundness of input consistency

namespace libsnark {

/**
 * Instance map for the R1CS-to-QAP reduction.
 *
 * Namely, given a R1CS constraint system cs, construct a QAP instance for which:
 *   A := (A_0(z),A_1(z),...,A_m(z))
 *   B := (B_0(z),B_1(z),...,B_m(z))
 *   C := (C_0(z),C_1(z),...,C_m(z))
 * where
 *   m = number of variables of the QAP
 * and
 *   each A_i,B_i,C_i is expressed in the Lagrange basis.
 */
template<typename FieldT>
qap_instance<FieldT> r1cs_to_qap_instance_map(const r1cs_constraint_system<FieldT> &cs)
{
    enter_block("Call to r1cs_to_qap_instance_map");

    qap_instance<FieldT> res;

    res.domain = get_evaluation_domain<FieldT>(cs.constraints.size() + R1CS_TO_QAP_ADDITIONAL_CONSTRAINTS);

    res.num_vars = cs.num_vars;
    res.degree = res.domain->m;
    res.num_inputs = cs.num_inputs;

    res.A_in_Lagrange_basis.resize(res.num_vars+1);
    res.B_in_Lagrange_basis.resize(res.num_vars+1);
    res.C_in_Lagrange_basis.resize(res.num_vars+1);

    enter_block("Compute polynomials A, B, C in Lagrange basis");
    /**
     * add and process the constraint
     *     (1 + \sum_{i=1}^{num_inputs} (i+1) * input_i) * 0 = 0
     * to ensure soundness of input consistency
     */
    for (size_t i = 0; i <= res.num_inputs; ++i)
    {
        res.A_in_Lagrange_basis[i][0] += FieldT(i+1);
    }
    /* process all other constraints */
    for (size_t i = 0; i < cs.constraints.size(); ++i)
    {
        for (size_t j = 0; j < cs.constraints[i].a.terms.size(); ++j)
        {
            res.A_in_Lagrange_basis[cs.constraints[i].a.terms[j].index][i+1] +=
            cs.constraints[i].a.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].b.terms.size(); ++j)
        {
            res.B_in_Lagrange_basis[cs.constraints[i].b.terms[j].index][i+1] +=
            cs.constraints[i].b.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].c.terms.size(); ++j)
        {
            res.C_in_Lagrange_basis[cs.constraints[i].c.terms[j].index][i+1] +=
            cs.constraints[i].c.terms[j].coeff;
        }
    }
    leave_block("Compute polynomials A, B, C in Lagrange basis");

    leave_block("Call to r1cs_to_qap_instance_map");

    return res;
}

/**
 * Instance map for the R1CS-to-QAP reduction followed by evaluation of the resulting QAP instance.
 *
 * Namely, given a R1CS constraint system cs and a field element t, construct
 * a QAP instance (evaluated at t) for which:
 *   At := (A_0(t),A_1(t),...,A_m(t))
 *   Bt := (B_0(t),B_1(t),...,B_m(t))
 *   Ct := (C_0(t),C_1(t),...,C_m(t))
 *   Ht := (1,t,t^2,...,t^n)
 *   Zt := Z(t) = "vanishing polynomial of a certain set S, evaluated at t"
 * where
 *   m = number of variables of the QAP
 *   n = degree of the QAP
 */
template<typename FieldT>
qap_instance_evaluation<FieldT> r1cs_to_qap_instance_map_with_evaluation(const r1cs_constraint_system<FieldT> &cs,
                                                                         const FieldT &t)
{
    enter_block("Call to r1cs_to_qap_instance_map_with_evaluation");

    qap_instance_evaluation<FieldT> res;

    res.domain = get_evaluation_domain<FieldT>(cs.constraints.size() + R1CS_TO_QAP_ADDITIONAL_CONSTRAINTS);

    res.num_vars = cs.num_vars;
    res.degree = res.domain->m;
    res.num_inputs = cs.num_inputs;

    res.t = t;

    res.At.resize(res.num_vars+1, FieldT::zero());
    res.Bt.resize(res.num_vars+1, FieldT::zero());
    res.Ct.resize(res.num_vars+1, FieldT::zero());
    res.Ht.reserve(res.degree+1);

    res.Zt = res.domain->compute_Z(res.t);

    enter_block("Compute evaluations of A, B, C, H at t");
    const std::vector<FieldT> u = res.domain->lagrange_coeffs(res.t);
    /**
     * add and process the constraint
     *     (1 + \sum_{i=1}^{num_inputs} (i+1) * input_i) * 0 = 0
     * to ensure soundness of input consistency
     */
    for (size_t i = 0; i <= res.num_inputs; ++i)
    {
        res.At[i] += u[0] * FieldT(i+1);
    }
    /* process all other constraints */
    for (size_t i = 0; i < cs.constraints.size(); ++i)
    {
        for (size_t j = 0; j < cs.constraints[i].a.terms.size(); ++j)
        {
            res.At[cs.constraints[i].a.terms[j].index] +=
                u[i+1]*cs.constraints[i].a.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].b.terms.size(); ++j)
        {
            res.Bt[cs.constraints[i].b.terms[j].index] +=
                u[i+1]*cs.constraints[i].b.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].c.terms.size(); ++j)
        {
            res.Ct[cs.constraints[i].c.terms[j].index] +=
                u[i+1]*cs.constraints[i].c.terms[j].coeff;
        }
    }
    FieldT ti = FieldT::one();
    for (size_t i = 0; i < res.degree+1; ++i)
    {
        res.Ht.emplace_back(ti);
        ti *= res.t;
    }
    leave_block("Compute evaluations of A, B, C, H at t");

    leave_block("Call to r1cs_to_qap_instance_map_with_evaluation");

    return res;
}

/**
 * Witness map for the R1CS-to-QAP reduction.
 *
 * The witness map takes zero knowledge into account when d1,d2,d3 are random.
 *
 * More precisely, compute the coefficients
 *     h_0,h_1,...,h_n
 * of the polynomial
 *     H(z) := (A(z)*B(z)-C(z))/Z(z)
 * where
 *   A(z) := A_0(z) + \sum_{k=1}^{m} w_k A_k(z) + d1 * Z(z)
 *   B(z) := B_0(z) + \sum_{k=1}^{m} w_k B_k(z) + d2 * Z(z)
 *   C(z) := C_0(z) + \sum_{k=1}^{m} w_k C_k(z) + d3 * Z(z)
 *   Z(z) := "vanishing polynomial of set S"
 * and
 *   m = number of variables of the QAP
 *   n = degree of the QAP
 *
 * This is done as follows:
 *  (1) compute evaluations of A,B,C on S = {sigma_1,...,sigma_n}
 *  (2) compute coefficients of A,B,C
 *  (3) compute evaluations of A,B,C on T = "coset of S"
 *  (4) compute evaluation of H on T
 *  (5) compute coefficients of H
 *  (6) patch H to account for d1,d2,d3 (i.e., add coefficients of the polynomial (A d2 + B d1 - d3) + d1*d2*Z )
 *
 * The code below is not as simple as the above high-level description due to
 * some reshuffling to save space.
 */
template<typename FieldT>
qap_witness<FieldT> r1cs_to_qap_witness_map(const r1cs_constraint_system<FieldT> &cs,
                                            const r1cs_variable_assignment<FieldT> &w,
                                            const FieldT &d1,
                                            const FieldT &d2,
                                            const FieldT &d3)
{
    enter_block("Call to r1cs_to_qap_witness_map");

    /* sanity check */
    assert(cs.is_satisfied(w));

    std::shared_ptr<evaluation_domain<FieldT> > domain = get_evaluation_domain<FieldT>(cs.constraints.size() + R1CS_TO_QAP_ADDITIONAL_CONSTRAINTS);

    qap_witness<FieldT> res;

    res.d1 = d1;
    res.d2 = d2;
    res.d3 = d3;

    res.coefficients_for_ABCs = w;

    res.num_vars = cs.num_vars;
    res.degree = domain->m;
    res.num_inputs = cs.num_inputs;

    enter_block("Compute evaluation of polynomials A, B on set S");
    std::vector<FieldT> aA(res.degree, FieldT::zero()), aB(res.degree, FieldT::zero());

    /* account for the additional constraint (1 + \sum_{i=1}^{num_inputs} (i+1) * input_i) * 0 = 0 */
    aA[0] = FieldT::one();
    for (size_t i = 0; i < res.num_inputs; ++i)
    {
        aA[0] += w[i] * FieldT(i+2);
    }
    /* account for all other constraints */
    for (size_t i = 0; i < cs.constraints.size(); ++i)
    {
        aA[i+1] += cs.constraints[i].a.evaluate(w);
        aB[i+1] += cs.constraints[i].b.evaluate(w);
    }
    leave_block("Compute evaluation of polynomials A, B on set S");

    enter_block("Compute coefficients of polynomial A");
    domain->iFFT(aA);
    leave_block("Compute coefficients of polynomial A");

    enter_block("Compute coefficients of polynomial B");
    domain->iFFT(aB);
    leave_block("Compute coefficients of polynomial B");

    enter_block("Compute ZK-patch");
    res.coefficients_for_H = std::vector<FieldT>(res.degree+1, FieldT::zero());
#ifdef MULTICORE
#pragma omp parallel for
#endif
    /* add coefficients of the polynomial (d2*A + d1*B - d3) + d1*d2*Z */
    for (size_t i = 0; i < res.degree; ++i)
    {
        res.coefficients_for_H[i] = res.d2*aA[i] + res.d1*aB[i];
    }
    res.coefficients_for_H[0] -= res.d3;
    domain->add_poly_Z(res.d1*res.d2, res.coefficients_for_H);
    leave_block("Compute ZK-patch");

    enter_block("Compute evaluation of polynomial A on set T");
    domain->cosetFFT(aA, FieldT::multiplicative_generator);
    leave_block("Compute evaluation of polynomial A on set T");

    enter_block("Compute evaluation of polynomial B on set T");
    domain->cosetFFT(aB, FieldT::multiplicative_generator);
    leave_block("Compute evaluation of polynomial B on set T");

    enter_block("Compute evaluation of polynomial H on set T");
    std::vector<FieldT> &H_tmp = aA; // can overwrite aA because it is not used later
#ifdef MULTICORE
#pragma omp parallel for
#endif
    for (size_t i = 0; i < res.degree; ++i)
    {
        H_tmp[i] = aA[i]*aB[i];
    }
    std::vector<FieldT>().swap(aB); // destroy aB

    enter_block("Compute evaluation of polynomial C on set S");
    std::vector<FieldT> aC(res.degree, FieldT::zero());
    for (size_t i = 0; i < cs.constraints.size(); ++i)
    {
        aC[i+1] += cs.constraints[i].c.evaluate(w);
    }
    leave_block("Compute evaluation of polynomial C on set S");

    enter_block("Compute coefficients of polynomial C");
    domain->iFFT(aC);
    leave_block("Compute coefficients of polynomial C");

    enter_block("Compute evaluation of polynomial C on set T");
    domain->cosetFFT(aC, FieldT::multiplicative_generator);
    leave_block("Compute evaluation of polynomial C on set T");

#ifdef MULTICORE
#pragma omp parallel for
#endif
    for (size_t i = 0; i < res.degree; ++i)
    {
        H_tmp[i] = (H_tmp[i]-aC[i]);
    }

    enter_block("Divide by Z on set T");
    domain->divide_by_Z_on_coset(H_tmp);
    leave_block("Divide by Z on set T");

    leave_block("Compute evaluation of polynomial H on set T");

    enter_block("Compute coefficients of polynomial H");
    domain->icosetFFT(H_tmp, FieldT::multiplicative_generator);
    leave_block("Compute coefficients of polynomial H");

    enter_block("Compute sum of H and ZK-patch");
#ifdef MULTICORE
#pragma omp parallel for
#endif
    for (size_t i = 0; i < res.degree; ++i)
    {
        res.coefficients_for_H[i] += H_tmp[i];
    }
    leave_block("Compute sum of H and ZK-patch");

    leave_block("Call to r1cs_to_qap_witness_map");

    return res;
}

} // libsnark

#endif // R1CS_TO_QAP_TCC_
