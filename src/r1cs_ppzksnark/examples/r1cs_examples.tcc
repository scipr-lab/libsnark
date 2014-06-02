/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef R1CS_EXAMPLES_TCC_
#define R1CS_EXAMPLES_TCC_

#include <cassert>
#include "common/utils.hpp"

namespace libsnark {

/* NOTE: all examples here actually generate one constraint less to account for soundness constraint in QAP */

template<typename FieldT>
r1cs_example<FieldT> gen_r1cs_example_Fr_input(const size_t num_constraints,
                                               const size_t num_inputs)
{
    const size_t new_num_constraints = num_constraints - 1;
    assert(num_inputs <= new_num_constraints + 2);

    r1cs_constraint_system<FieldT> q;
    r1cs_variable_assignment<FieldT> va;
    q.num_inputs = num_inputs;
    q.num_vars = 2 + new_num_constraints;

    FieldT a = FieldT::random_element(), b = FieldT::random_element();
    va.push_back(a); va.push_back(b);

    for (size_t i = 0; i < new_num_constraints-1; ++i)
    {
        linear_combination<FieldT> A, B, C;
        if (i % 2)
        {
            // a * b = c
            A.add_term(i+1, 1);
            B.add_term(i+2, 1);
            C.add_term(i+3, 1);
            FieldT tmp = a*b;
            va.push_back(tmp);
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
            va.push_back(tmp);
            a = b; b = tmp;
        }

        q.add_constraint(r1cs_constraint<FieldT>(A, B, C));
    }

    linear_combination<FieldT> A, B, C;
    FieldT fin = FieldT::zero();
    for (size_t i = 1; i < q.num_vars; ++i)
    {
        A.add_term(i, 1);
        B.add_term(i, 1);
        fin = fin + va[i-1];
    }
    C.add_term(q.num_vars, 1);
    q.add_constraint(r1cs_constraint<FieldT>(A, B, C));
    va.push_back(fin.squared());

    r1cs_variable_assignment<FieldT> input(va.begin(), va.begin() + num_inputs);
    return r1cs_example<FieldT>(std::move(q), std::move(input), std::move(va), num_inputs);
}

template<typename FieldT>
r1cs_example<FieldT> gen_r1cs_example_binary_input(const size_t num_constraints,
                                                   const size_t num_inputs)
{
    const size_t new_num_constraints = num_constraints - 1;
    assert(num_inputs >= 1);

    r1cs_constraint_system<FieldT> q;
    r1cs_variable_assignment<FieldT> va;
    q.num_inputs = num_inputs;
    q.num_vars = num_inputs + new_num_constraints;

    for (size_t i = 0; i < num_inputs; ++i)
    {
        va.push_back(FieldT(std::rand() % 2));
    }

    size_t lastvar = num_inputs-1;
    for (size_t i = 0; i < new_num_constraints; ++i)
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

        q.add_constraint(r1cs_constraint<FieldT>(A, B, C));
        va.push_back(va[u] + va[v] - va[u] * va[v] - va[u] * va[v]);
    }
    assert(va.size() == q.num_vars);

    r1cs_variable_assignment<FieldT> input(va.begin(), va.begin() + num_inputs);
    return r1cs_example<FieldT>(std::move(q), std::move(input), std::move(va), num_inputs);
}

} // libsnark
#endif // R1CS_EXAMPLES_TCC
