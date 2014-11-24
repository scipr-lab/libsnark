/** @file
 *****************************************************************************

 Declaration of interfaces for:
 - a USCS constraint,
 - a USCS variable assignment, and
 - a USCS constraint system.

 Above, USCS stands for "Unitary-Square Constraint System".

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef USCS_HPP_
#define USCS_HPP_

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "variable/variable.hpp"

namespace libsnark {

/************************* USCS constraint ***********************************/

/**
 * A USCS constraint is a formal expression of the form
 *
 *                \sum_{i=1}^{m} a_i * x_{i} ,
 *
 * where each a_i is in <FieldT> and each x_{i} is a formal variable.
 *
 * A USCS constraint is used to construct a USCS constraint system (see below).
 */
template<typename FieldT>
using uscs_constraint = linear_combination<FieldT>;


/************************* USCS variable assignment **************************/

/**
 * A USCS variable assignment is a vector of <FieldT> elements that represents
 * a candidate solution to a USCS constraint system (see below).
 */
template<typename FieldT>
using uscs_variable_assignment = std::vector<FieldT>;



/************************* USCS constraint system ****************************/

template<typename FieldT>
class uscs_constraint_system;

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const uscs_constraint_system<FieldT> &cs);

template<typename FieldT>
std::istream& operator>>(std::istream &in, uscs_constraint_system<FieldT> &cs);

/**
 * A system of USCS constraints looks like
 *
 *     { ( \sum_{i=1}^{m_k} a_{k,i} * x_{k,i} )^2 = 1 }_{k=1}^{n}  .
 *
 * In other words, the system is satisfied if and only if there exist a
 * USCS variable assignment for which each USCS constraint evaluates to -1 or 1.
 *
 * NOTE:
 * The 0-th variable (i.e., "x_{0}") always represents the constant 1.
 * Thus, the 0-th variable is not included in num_vars.
 */
template<typename FieldT>
class uscs_constraint_system {
public:

    std::vector<uscs_constraint<FieldT> > constraints;

    size_t num_inputs;
    size_t num_vars;

    uscs_constraint_system() : num_inputs(0), num_vars(0) {};

#ifdef DEBUG
    std::map<size_t, std::string> constraint_annotations;
    std::map<size_t, std::string> variable_annotations;
#endif

    bool is_valid() const;
    bool is_satisfied(const uscs_variable_assignment<FieldT> &va) const;

    void add_constraint(const uscs_constraint<FieldT> &c);
    void add_constraint(const uscs_constraint<FieldT> &c, const std::string &annotation);

    bool operator==(const uscs_constraint_system<FieldT> &other) const;

    friend std::ostream& operator<< <FieldT>(std::ostream &out, const uscs_constraint_system<FieldT> &cs);
    friend std::istream& operator>> <FieldT>(std::istream &in, uscs_constraint_system<FieldT> &cs);

    void report_statistics() const;
};


} // libsnark

#include "uscs/uscs.tcc"

#endif // USCS_HPP_
