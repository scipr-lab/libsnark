/** @file
 *****************************************************************************
 Implementation of interfaces for constructing a QAP ("Quadratic Arithmetic Program")
 from a R1CS ("Rank-1 Constraint System")

 See qap.hpp .
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef QAP_TCC_
#define QAP_TCC_

#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "evaluation_domain/evaluation_domain.hpp"

namespace libsnark {

template<typename FieldT>
void qap_get_params(const r1cs_constraint_system<FieldT> &cs,
                    size_t *qap_num_vars,
                    size_t *qap_degree,
                    size_t *qap_num_inputs)
{
    // +1 is for an additional constraint needed for the soundness of input consistency
    std::shared_ptr<evaluation_domain<FieldT> > domain = get_evaluation_domain<FieldT>(cs.constraints.size() + 1);

    *qap_num_vars = cs.num_vars;
    *qap_degree = domain->m;
    *qap_num_inputs = cs.num_inputs;
}

/*
  Given a constraint system cs and field element t, compute:
   At = (Z(t),0,   0,   A_0(t),A_1(t),...,A_n(t))
   Bt = (0   ,Z(t),0,   B_0(t),B_1(t),...,B_n(t))
   Ct = (0   ,0   ,Z(t),C_0(t),C_1(t),...,C_n(t))
   Ht = (1,t,t^2,...,t^m)
  where
    n = number of variables of the QAP
    m = degree of the QAP
*/
template<typename FieldT>
ABCH_eval_at_t<FieldT> qap_instance_map(const r1cs_constraint_system<FieldT> &cs,
                                        const FieldT &t)
{
    enter_block("Compute evaluations of A, B, C, H at t");

    size_t qap_num_vars;
    size_t qap_degree;
    size_t qap_num_inputs;
    qap_get_params(cs, &qap_num_vars, &qap_degree, &qap_num_inputs);

    std::shared_ptr<evaluation_domain<FieldT> > domain = get_evaluation_domain<FieldT>(qap_degree);

    ABCH_eval_at_t<FieldT> res;
    res.At.resize(3+qap_num_vars+1, FieldT::zero());
    res.Bt.resize(3+qap_num_vars+1, FieldT::zero());
    res.Ct.resize(3+qap_num_vars+1, FieldT::zero());
    res.Ht.reserve(qap_degree+1);

    const std::vector<FieldT> u = domain->lagrange_coeffs(t);

    /* support for ZK */
    const FieldT Z = domain->compute_Z(t);
    res.At[0] = Z;
    res.Bt[1] = Z;
    res.Ct[2] = Z;

    /*
     account for the additional constraint (1 + \sum_{i=1}^{qap_num_inputs} (i+1) * input_i) * 0 = 0,
     needed for soundness in input consistency
    */

    for (size_t i = 0; i <= qap_num_inputs; ++i)
    {
        res.At[3+i] += u[0] * FieldT(i+1);
    }

    /* account for all other constraints */
    for (size_t i = 0; i < cs.constraints.size(); ++i)
    {
        for (size_t j = 0; j < cs.constraints[i].a.terms.size(); ++j)
        {
            res.At[3+cs.constraints[i].a.terms[j].index] +=
                u[i+1]*cs.constraints[i].a.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].b.terms.size(); ++j)
        {
            res.Bt[3+cs.constraints[i].b.terms[j].index] +=
                u[i+1]*cs.constraints[i].b.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].c.terms.size(); ++j)
        {
            res.Ct[3+cs.constraints[i].c.terms[j].index] +=
                u[i+1]*cs.constraints[i].c.terms[j].coeff;
        }
    }

    FieldT ti = FieldT::one();
    for (size_t i = 0; i <= qap_degree; ++i)
    {
        res.Ht.emplace_back(ti);
        ti *= t;
    }

    enter_block("Compute query densities");
    res.non_zero_At = res.non_zero_Bt = res.non_zero_Ct = 0;
    for (size_t i = 0; i < 3+qap_num_vars+1; ++i)
    {
        if (!res.At[i].is_zero())
        {
            ++res.non_zero_At;
        }

        if (!res.Bt[i].is_zero())
        {
            ++res.non_zero_Bt;
        }

        if (!res.Ct[i].is_zero())
        {
            ++res.non_zero_Ct;
        }
    }

    res.non_zero_Ht = 0;
    for (size_t i = 0; i < qap_degree+1; ++i)
    {
        if (!res.Ht[i].is_zero())
        {
            ++res.non_zero_Ht;
        }
    }
    leave_block("Compute query densities");

    leave_block("Compute evaluations of A, B, C, H at t");
    return res;
}


/*
 Compute the coefficients
 h_0,h_1,...,h_m
 of the polynomial
 H(z) = (A(z)*B(z)-C(z))/Z(z)
 where
 A(z) = A_0(z) + \sum_{k=1}^{n} w_k A_k(z) + d1 * Z(z)
 B(z) = B_0(z) + \sum_{k=1}^{n} w_k B_k(z) + d2 * Z(z)
 C(z) = C_0(z) + \sum_{k=1}^{n} w_k C_k(z) + d3 * Z(z)
 Z(z) = "vanishing polynomial of set S"
 and
 n = number of variables of the QAP
 m = degree of the QAP

 This is done as follows:
 (1) compute evaluations of A,B,C on S = {sigma_1,...,sigma_m}
 (2) compute coefficients of A,B,C
 (3) compute evaluations of A,B,C on T = "coset of S"
 (4) compute evaluation of H on T
 (5) compute coefficients of H
 (6) patch H to account for d1,d2,d3
 [ add coefficients of the polynomial (A delta2 + B delta1 - delta3) + delta1*delta2*Z ]

 The code below is not as simple as the above high-level description due to
 some reshuffling to save space.
 */
template<typename FieldT>
std::vector<FieldT> qap_witness_map(const r1cs_constraint_system<FieldT> &cs,
                                    const r1cs_variable_assignment<FieldT> &w,
                                    const FieldT &d1,
                                    const FieldT &d2,
                                    const FieldT &d3)
{

    assert(cs.num_vars == w.size());

    size_t qap_num_vars;
    size_t qap_degree;
    size_t qap_num_inputs;
    qap_get_params(cs, &qap_num_vars, &qap_degree, &qap_num_inputs);

    std::shared_ptr<evaluation_domain<FieldT> > domain = get_evaluation_domain<FieldT>(qap_degree);

    enter_block("Compute evaluation of polynomials A, B on set S");
    std::vector<FieldT> aA(qap_degree, FieldT::zero()), aB(qap_degree, FieldT::zero());

    /* soundness patch */
    /* this accounts for the additional constraint (1 + \sum_{i=1}^{qap_num_inputs} (i+1) * input_i) * 0 = 0 */

    aA[0] = FieldT::one();
    for (size_t i = 0; i < qap_num_inputs; ++i)
    {
        aA[0] += w[i] * FieldT(i+2);
    }

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
    std::vector<FieldT> H(qap_degree+1, FieldT::zero());
#ifdef MULTICORE
#pragma omp parallel for
#endif
    /* add coefficients of the polynomial (A delta2 + B delta1 - delta3) + delta1*delta2*Z */
    for (size_t i = 0; i < qap_degree; ++i)
    {
        H[i] = d2*aA[i] + d1*aB[i];
    }
    H[0] -= d3;
    domain->add_poly_Z(d1*d2, H);

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
    for (size_t i = 0; i < qap_degree; ++i)
    {
        H_tmp[i] = aA[i]*aB[i];
    }
    std::vector<FieldT>().swap(aB); // destroy aB

    enter_block("Compute evaluation of polynomial C on set S");
    std::vector<FieldT> aC(qap_degree, FieldT::zero());
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
    for (size_t i = 0; i < qap_degree; ++i)
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
    for (size_t i = 0; i < qap_degree; ++i)
    {
        H[i] += H_tmp[i];
    }
    leave_block("Compute sum of H and ZK-patch");

    return H;
}

} // libsnark
#endif // QAP_TCC_
