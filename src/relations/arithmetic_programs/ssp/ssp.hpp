/** @file
 *****************************************************************************

 Declaration of interfaces for a SSP ("Square Span Program").

 SSPs are defined in \[DFGK14].

 References:

 \[DFGK14]:
 "Square Span Programs with Applications to Succinct NIZK Arguments"
 George Danezis, Cedric Fournet, Jens Groth, Markulf Kohlweiss,
 ASIACRYPT 2014,
 <http://eprint.iacr.org/2014/718>

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef SSP_HPP_
#define SSP_HPP_

#include "algebra/evaluation_domain/evaluation_domain.hpp"

namespace libsnark {

/* forward declaration */
template<typename FieldT>
struct ssp_witness;

/**
 * A SSP instance.
 *
 * Specifically, the datastructure stores:
 * - a choice of domain (corresponding to a certain subset of the field);
 * - the number of variables, the degree, and the number of inputs; and
 * - coefficients of the V polynomials in the Lagrange basis.
 *
 * There is no need to store the Z polynomial because it is uniquely
 * determined by the domain (as Z is its vanishing polynomial).
 */
template<typename FieldT>
struct ssp_instance {

    std::shared_ptr<evaluation_domain<FieldT> > domain;

    size_t num_vars;
    size_t degree;
    size_t num_inputs;

    std::vector<std::map<size_t, FieldT> > V_in_Lagrange_basis;

    ssp_instance() = default;
    ssp_instance(const ssp_instance<FieldT> &other) = default;
    ssp_instance(ssp_instance<FieldT> &&other) = default;
    ssp_instance& operator=(const ssp_instance<FieldT> &other) = default;
    ssp_instance& operator=(ssp_instance<FieldT> &&other) = default;

    bool is_satisfied(const ssp_witness<FieldT> &witness) const;
};


/**
 * A SSP instance evaluation is a SSP instance that is evaluated at a field element t.
 *
 * Specifically, the datastructure stores:
 * - a choice of domain (corresponding to a certain subset of the field);
 * - the number of variables, the degree, and the number of inputs;
 * - a field element t;
 * - evaluations of the V (and Z) polynomials at t;
 * - evaluations of all monomials of t.
 */
template<typename FieldT>
struct ssp_instance_evaluation {

    std::shared_ptr<evaluation_domain<FieldT> > domain;

    size_t num_vars;
    size_t degree;
    size_t num_inputs;

    FieldT t;

    std::vector<FieldT> Vt, Ht;

    FieldT Zt;

    ssp_instance_evaluation() = default;
    ssp_instance_evaluation(const ssp_instance_evaluation<FieldT> &other) = default;
    ssp_instance_evaluation(ssp_instance_evaluation<FieldT> &&other) = default;
    ssp_instance_evaluation& operator=(const ssp_instance_evaluation<FieldT> &other) = default;
    ssp_instance_evaluation& operator=(ssp_instance_evaluation<FieldT> &&other) = default;

    bool is_satisfied(const ssp_witness<FieldT> &witness) const;
};

/**
 * A SSP witness.
 */
template<typename FieldT>
struct ssp_witness {

    FieldT d;

    size_t num_vars;
    size_t degree;
    size_t num_inputs;

    std::vector<FieldT> coefficients_for_Vs;
    std::vector<FieldT> coefficients_for_H;

    ssp_witness() = default;
    ssp_witness(const ssp_witness<FieldT> &other) = default;
    ssp_witness(ssp_witness<FieldT> &&other) = default;
    ssp_witness& operator=(const ssp_witness<FieldT> &other) = default;
    ssp_witness& operator=(ssp_witness<FieldT> &&other) = default;
};

} // libsnark

#include "relations/arithmetic_programs/ssp/ssp.tcc"

#endif // SSP_HPP_
