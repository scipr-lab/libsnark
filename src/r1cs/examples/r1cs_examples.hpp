/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_EXAMPLES_HPP_
#define R1CS_EXAMPLES_HPP_

#include "r1cs/r1cs.hpp"

namespace libsnark {

template<typename FieldT>
struct r1cs_example {
    r1cs_constraint_system<FieldT> constraint_system;
    r1cs_variable_assignment<FieldT> input;
    r1cs_variable_assignment<FieldT> witness;
    size_t num_inputs;

    r1cs_example<FieldT>() = default;
    r1cs_example<FieldT>(const r1cs_example<FieldT> &other) = default;
    r1cs_example<FieldT>(const r1cs_constraint_system<FieldT> &constraint_system,
                         const r1cs_variable_assignment<FieldT> &input,
                         const r1cs_variable_assignment<FieldT> &witness,
                         const size_t num_inputs) :
    constraint_system(constraint_system),
        input(input),
        witness(witness),
        num_inputs(num_inputs)
    {};
    r1cs_example<FieldT>(r1cs_constraint_system<FieldT> &&constraint_system,
                         r1cs_variable_assignment<FieldT> &&input,
                         r1cs_variable_assignment<FieldT> &&witness,
                         const size_t num_inputs) :
    constraint_system(std::move(constraint_system)),
        input(std::move(input)),
        witness(std::move(witness)),
        num_inputs(num_inputs)
    {};
};

template<typename FieldT>
r1cs_example<FieldT> gen_r1cs_example_Fr_input(const size_t num_constraints,
                                               const size_t num_inputs);

template<typename FieldT>
r1cs_example<FieldT> gen_r1cs_example_binary_input(const size_t num_constraints,
                                                   const size_t num_inputs);

} // libsnark

#include "r1cs/examples/r1cs_examples.tcc"

#endif // R1CS_EXAMPLES_HPP_
