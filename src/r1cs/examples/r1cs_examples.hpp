/** @file
 *****************************************************************************

 Declaration of interfaces for a R1CS example, as well as functions to sample
 R1CS examples with prescribed parameters (according to some distribution).

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_EXAMPLES_HPP_
#define R1CS_EXAMPLES_HPP_

#include "r1cs/r1cs.hpp"

namespace libsnark {

/**
 * A R1CS example comprises a R1CS constraint system, R1CS input, and R1CS witness.
 */
template<typename FieldT>
struct r1cs_example {
    r1cs_constraint_system<FieldT> constraint_system;
    r1cs_variable_assignment<FieldT> input;
    r1cs_variable_assignment<FieldT> witness;

    r1cs_example<FieldT>() = default;
    r1cs_example<FieldT>(const r1cs_example<FieldT> &other) = default;
    r1cs_example<FieldT>(const r1cs_constraint_system<FieldT> &constraint_system,
                         const r1cs_variable_assignment<FieldT> &input,
                         const r1cs_variable_assignment<FieldT> &witness) :
        constraint_system(constraint_system),
        input(input),
        witness(witness)
    {};
    r1cs_example<FieldT>(r1cs_constraint_system<FieldT> &&constraint_system,
                         r1cs_variable_assignment<FieldT> &&input,
                         r1cs_variable_assignment<FieldT> &&witness) :
        constraint_system(std::move(constraint_system)),
        input(std::move(input)),
        witness(std::move(witness))
    {};
};

/**
 * Generate an R1CS example such that:
 * - the number of constraints of the R1CS constraint system is num_constraints;
 * - the number of variables of the R1CS constraint system is (approximately) num_constraints;
 * - the number of inputs of the R1CS constraint system is num_inputs;
 * - the R1CS input consists of ``full'' field elements (typically require the whole log|Field| bits to represent).
 */
template<typename FieldT>
r1cs_example<FieldT> generate_r1cs_example_with_field_input(const size_t num_constraints,
                                                            const size_t num_inputs);

/**
 * Generate an R1CS example such that:
 * - the number of constraints of the R1CS constraint system is num_constraints;
 * - the number of variables of the R1CS constraint system is (approximately) num_constraints;
 * - the number of inputs of the R1CS constraint system is num_inputs;
 * - the R1CS input consists of binary values (as opposed to ``full'' field elements).
 */
template<typename FieldT>
r1cs_example<FieldT> generate_r1cs_example_with_binary_input(const size_t num_constraints,
                                                             const size_t num_inputs);

} // libsnark

#include "r1cs/examples/r1cs_examples.tcc"

#endif // R1CS_EXAMPLES_HPP_
