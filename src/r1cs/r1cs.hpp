/** @file
 *****************************************************************************
 Declaration of interfaces for the language R1CS ("rank-1 constraint systems").

 An instance of the language is specified by a list of equations over a field <FieldT>,
 and each equation looks like:
        < A, (1,X) > * < B , (1,X) > = < C, (1,X) >
 where A,B,C are vectors over <FieldT>, and X is a vector of variables.
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

#include "r1cs/variable.hpp"

namespace libsnark {

/*
 Interfaces for the language R1CS ("rank-1 constraint systems").
 An instance of the language is specified by a list of equations over a field <FieldT>,
 and each equation looks like:
      < A, (1,X) > * < B , (1,X) > = < C, (1,X) >
 where A,B,C are vectors over <FieldT>, and X is a vector of variables.
*/

template<typename FieldT>
using r1cs_variable_assignment = std::vector<FieldT>;


template<typename FieldT>
FieldT padded_inner_product(const linear_combination<FieldT> &r,
                            const r1cs_variable_assignment<FieldT> &va);


template<typename FieldT>
class r1cs_constraint;
template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const r1cs_constraint<FieldT> &c);
template<typename FieldT>
std::istream& operator>>(std::istream &in, r1cs_constraint<FieldT> &c);

template<typename FieldT>
class r1cs_constraint {
public:
    // corresponds to the constraint <A,(1,X)> * <B,(1,X)> = <C,(1,X)>
    // where A,B,C are the dense vectors corresponding to the sparse vectors a,b,c
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


template<typename FieldT>
class r1cs_constraint_system;
template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const r1cs_constraint_system<FieldT> &cs);
template<typename FieldT>
std::istream& operator>>(std::istream &in, r1cs_constraint_system<FieldT> &cs);

template<typename FieldT>
class r1cs_constraint_system {
public:
    std::vector<r1cs_constraint<FieldT> > constraints;
    size_t num_inputs, num_vars;

    /*
      NOTE:
       the 0-th variable refers to the constant 1;
       thus, we do not include it when counting num_vars (in particular, max_index equals num_vars)
    */

    r1cs_constraint_system() : num_inputs(0), num_vars(0) {};

#ifdef DEBUG
    std::map<size_t, std::string> constraint_annotations;
    std::map<size_t, std::string> variable_annotations;
#endif

    bool is_valid() const; // sanity checks (e.g. num inputs <= num variables)
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
