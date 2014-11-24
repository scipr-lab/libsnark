/** @file
 *****************************************************************************

 Declaration of interfaces for:
 - a R1CS constraint,
 - a R1CS variable assignment, and
 - a R1CS constraint system.

 Above, R1CS stands for "Rank-1 Constraint System".

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef R1CS_HPP_
#define R1CS_HPP_

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "variable/variable.hpp"

namespace libsnark {

/************************* R1CS constraint ***********************************/

template<typename FieldT>
class r1cs_constraint;

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const r1cs_constraint<FieldT> &c);

template<typename FieldT>
std::istream& operator>>(std::istream &in, r1cs_constraint<FieldT> &c);

/**
 * A R1CS constraint is a formal expression of the form
 *
 *                < A , X > * < B , X > = < C , X > ,
 *
 * where X = (x_0,x_1,...,x_m) is a vector of formal variables and A,B,C each
 * consist of 1+m elements in <FieldT>.
 *
 * A R1CS constraint is used to construct a R1CS constraint system (see below).
 */
template<typename FieldT>
class r1cs_constraint {
public:

    linear_combination<FieldT> a, b, c;

    r1cs_constraint() {};
    r1cs_constraint(const linear_combination<FieldT> &a,
                    const linear_combination<FieldT> &b,
                    const linear_combination<FieldT> &c);

    r1cs_constraint(const std::initializer_list<linear_combination<FieldT> > A,
                    const std::initializer_list<linear_combination<FieldT> > B,
                    const std::initializer_list<linear_combination<FieldT> > C);

    bool operator==(const r1cs_constraint<FieldT> &other) const;

    friend std::ostream& operator<< <FieldT>(std::ostream &out, const r1cs_constraint<FieldT> &c);
    friend std::istream& operator>> <FieldT>(std::istream &in, r1cs_constraint<FieldT> &c);
};

/************************* R1CS variable assignment **************************/

/**
 * A R1CS variable assignment is a vector of <FieldT> elements that represents
 * a candidate solution to a R1CS constraint system (see below).
 */
template<typename FieldT>
using r1cs_variable_assignment = std::vector<FieldT>;


/************************* R1CS constraint system ****************************/

template<typename FieldT>
class r1cs_constraint_system;

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const r1cs_constraint_system<FieldT> &cs);

template<typename FieldT>
std::istream& operator>>(std::istream &in, r1cs_constraint_system<FieldT> &cs);

/**
 * A system of R1CS constraints looks like
 *
 *     { < A_k , X > * < B_k , X > = < C_k , X > }_{k=1}^{n}  .
 *
 * In other words, the system is satisfied if and only if there exist a
 * USCS variable assignment for which each R1CS constraint is satisfied.
 *
 * NOTE:
 * The 0-th variable (i.e., "x_{0}") always represents the constant 1.
 * Thus, the 0-th variable is not included in num_vars.
 */
template<typename FieldT>
class r1cs_constraint_system {
public:

    std::vector<r1cs_constraint<FieldT> > constraints;

    size_t num_inputs;
    size_t num_vars;

    r1cs_constraint_system() : num_inputs(0), num_vars(0) {};

#ifdef DEBUG
    std::map<size_t, std::string> constraint_annotations;
    std::map<size_t, std::string> variable_annotations;
#endif

    bool is_valid() const;
    bool is_satisfied(const r1cs_variable_assignment<FieldT> &va) const;

    void add_constraint(const r1cs_constraint<FieldT> &c);
    void add_constraint(const r1cs_constraint<FieldT> &c, const std::string &annotation);

    void swap_AB_if_beneficial();

    bool operator==(const r1cs_constraint_system<FieldT> &other) const;

    friend std::ostream& operator<< <FieldT>(std::ostream &out, const r1cs_constraint_system<FieldT> &cs);
    friend std::istream& operator>> <FieldT>(std::istream &in, r1cs_constraint_system<FieldT> &cs);

    void report_statistics() const;
};


} // libsnark

#include "r1cs/r1cs.tcc"

#endif // R1CS_HPP_
