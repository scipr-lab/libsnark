/** @file
 *****************************************************************************

 Implementation of functions to sample R1CS examples with prescribed parameters
 (according to some distribution).

 See r1cs_examples.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_EXAMPLES_TCC_
#define R1CS_EXAMPLES_TCC_

#include <cassert>

#include "common/utils.hpp"

namespace libsnark {

template<typename FieldT>
r1cs_example<FieldT> generate_r1cs_example_with_field_input(const size_t num_constraints,
                                                            const size_t num_inputs)
{
    enter_block("Call to generate_r1cs_example_with_field_input");

    assert(num_inputs <= num_constraints + 2);

    r1cs_constraint_system<FieldT> cs;
    cs.num_inputs = num_inputs;
    cs.num_vars = 2 + num_constraints;

    r1cs_variable_assignment<FieldT> witness;
    FieldT a = FieldT::random_element();
    FieldT b = FieldT::random_element();
    witness.push_back(a);
    witness.push_back(b);

    for (size_t i = 0; i < num_constraints-1; ++i)
    {
        linear_combination<FieldT> A, B, C;

        if (i % 2)
        {
            // a * b = c
            A.add_term(i+1, 1);
            B.add_term(i+2, 1);
            C.add_term(i+3, 1);
            FieldT tmp = a*b;
            witness.push_back(tmp);
            a = b; b = tmp;
        }
        else
        {
            // a + b = c
            B.add_term(0, 1);
            A.add_term(i+1, 1);
            A.add_term(i+2, 1);
            C.add_term(i+3, 1);
            FieldT tmp = a+b;
            witness.push_back(tmp);
            a = b; b = tmp;
        }

        cs.add_constraint(r1cs_constraint<FieldT>(A, B, C));
    }

    linear_combination<FieldT> A, B, C;
    FieldT fin = FieldT::zero();
    for (size_t i = 1; i < cs.num_vars; ++i)
    {
        A.add_term(i, 1);
        B.add_term(i, 1);
        fin = fin + witness[i-1];
    }
    C.add_term(cs.num_vars, 1);
    cs.add_constraint(r1cs_constraint<FieldT>(A, B, C));
    witness.push_back(fin.squared());

    /* sanity checks */
    assert(cs.num_vars == witness.size());
    assert(cs.num_vars >= num_inputs);
    assert(cs.num_inputs == num_inputs);
    assert(cs.constraints.size() == num_constraints);
    assert(cs.is_satisfied(witness));

    r1cs_variable_assignment<FieldT> input(witness.begin(), witness.begin() + num_inputs);

    leave_block("Call to generate_r1cs_example_with_field_input");

    return r1cs_example<FieldT>(std::move(cs), std::move(input), std::move(witness));
}

template<typename FieldT>
r1cs_example<FieldT> generate_r1cs_example_with_binary_input(const size_t num_constraints,
                                                             const size_t num_inputs)
{
    enter_block("Call to generate_r1cs_example_with_binary_input");

    assert(num_inputs >= 1);

    r1cs_constraint_system<FieldT> cs;
    cs.num_inputs = num_inputs;
    cs.num_vars = num_inputs + num_constraints;

    r1cs_variable_assignment<FieldT> witness;
    for (size_t i = 0; i < num_inputs; ++i)
    {
        witness.push_back(FieldT(std::rand() % 2));
    }

    size_t lastvar = num_inputs-1;
    for (size_t i = 0; i < num_constraints; ++i)
    {
        ++lastvar;
        const size_t u = (i == 0 ? std::rand() % num_inputs : std::rand() % i);
        const size_t v = (i == 0 ? std::rand() % num_inputs : std::rand() % i);

        /* chose two random bits and XOR them together:
           res = u + v - 2 * u * v
           2 * u * v = u + v - res
        */
        linear_combination<FieldT> A, B, C;
        A.add_term(u+1, 2);
        B.add_term(v+1, 1);
        if (u == v)
        {
            C.add_term(u+1, 2);
        }
        else
        {
            C.add_term(u+1, 1);
            C.add_term(v+1, 1);
        }
        C.add_term(lastvar+1, -FieldT::one());

        cs.add_constraint(r1cs_constraint<FieldT>(A, B, C));
        witness.push_back(witness[u] + witness[v] - witness[u] * witness[v] - witness[u] * witness[v]);
    }

    /* sanity checks */
    assert(cs.num_vars == witness.size());
    assert(cs.num_vars >= num_inputs);
    assert(cs.num_inputs == num_inputs);
    assert(cs.constraints.size() == num_constraints);
    assert(cs.is_satisfied(witness));

    r1cs_variable_assignment<FieldT> input(witness.begin(), witness.begin() + num_inputs);

    leave_block("Call to generate_r1cs_example_with_binary_input");

    return r1cs_example<FieldT>(std::move(cs), std::move(input), std::move(witness));
}

} // libsnark

#endif // R1CS_EXAMPLES_TCC
