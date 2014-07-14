/** @file
 *****************************************************************************
 Declaration of interfaces for constructing a QAP ("Quadratic Arithmetic Program")
 from a R1CS ("Rank-1 Constraint System").

 The implementation follows, extends, and optimizes the basic approach described in Appendix E of:

 "SNARKs for C: Verifying Program Executions Succinctly and in Zero Knowledge",
 Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin, Eran Tromer, and Madars Virza,
 CRYPTO 2013,
 <http://eprint.iacr.org/2013/507>
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef QAP_HPP_
#define QAP_HPP_

#include "r1cs/r1cs.hpp"

namespace libsnark {

template<typename FieldT>
struct ABCH_eval_at_t {
    std::vector<FieldT> At, Bt, Ct, Ht;
    std::size_t non_zero_At, non_zero_Bt, non_zero_Ct, non_zero_Ht;

    ABCH_eval_at_t() = default;
    ABCH_eval_at_t& operator=(const ABCH_eval_at_t<FieldT> &other) = default;
    ABCH_eval_at_t& operator=(ABCH_eval_at_t<FieldT> &&other) = default;
    ABCH_eval_at_t(const ABCH_eval_at_t<FieldT> &other) = default;
    ABCH_eval_at_t(ABCH_eval_at_t<FieldT> &&other) = default;
};

/**
 * given an instance of R1CS, compute parameters for corresponding instance of R1CS
 */
template<typename FieldT>
void qap_get_params(const r1cs_constraint_system<FieldT> &cs,
                    size_t *qap_size,
                    size_t *qap_degree,
                    size_t *qap_num_inputs);

/**
 * instance map for R1CS->QAP reduction
 */
template<typename FieldT>
ABCH_eval_at_t<FieldT> qap_instance_map(const r1cs_constraint_system<FieldT> &cs,
                                        const FieldT &t);

/**
 * witness map for R1CS->QAP reduction (takes ZK into account when d1,d2,d3 are random)
 */
template<typename FieldT>
std::vector<FieldT> qap_witness_map(const r1cs_constraint_system<FieldT> &cs,
                                    const r1cs_variable_assignment<FieldT> &w,
                                    const FieldT &d1,
                                    const FieldT &d2,
                                    const FieldT &d3);

} // libsnark
#include "qap/qap.tcc"

#endif // QAP_HPP_
