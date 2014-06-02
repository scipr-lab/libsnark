/** @file
 *******************************************************************************
 Implementation of interfaces for the language R1CS ("rank-1 constraint systems").

 See r1cs.hpp.
 *******************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef R1CS_TCC_
#define R1CS_TCC_

#include "r1cs/r1cs.hpp"

#include <algorithm>
#include <cassert>
#include <set>
#include "common/utils.hpp"
#include "common/profiling.hpp"
#include "algebra/fields/bigint.hpp"

namespace libsnark {

template<typename FieldT>
FieldT padded_inner_product(const linear_combination<FieldT> &r, const r1cs_variable_assignment<FieldT> &w)
{
    FieldT acc = FieldT::zero();
    for (size_t i = 0; i < r.terms.size(); ++i)
    {
        acc += (r.terms[i].index == 0 ? FieldT::one() : w[r.terms[i].index-1]) * r.terms[i].coeff;
    }
    return acc;
}


template<typename FieldT>
r1cs_constraint<FieldT>::r1cs_constraint(const linear_combination<FieldT> &a,
                                         const linear_combination<FieldT> &b,
                                         const linear_combination<FieldT> &c) :
    a(a), b(b), c(c)
{
}

template<typename FieldT>
r1cs_constraint<FieldT>::r1cs_constraint(const std::initializer_list<linear_term<FieldT> > A,
                                         const std::initializer_list<linear_term<FieldT> > B,
                                         const std::initializer_list<linear_term<FieldT> > C)
{
    a.terms.insert(a.terms.begin(), A.begin(), A.end());
    b.terms.insert(b.terms.begin(), B.begin(), B.end());
    c.terms.insert(c.terms.begin(), C.begin(), C.end());
}

template<typename FieldT>
bool r1cs_constraint<FieldT>::operator==(const r1cs_constraint<FieldT> &other) const
{
    return (this->a == other.a &&
            this->b == other.b &&
            this->c == other.c);
}

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const r1cs_constraint<FieldT> &c)
{
    out << c.a;
    out << c.b;
    out << c.c;

    return out;
}

template<typename FieldT>
std::istream& operator>>(std::istream &in, r1cs_constraint<FieldT> &c)
{
    in >> c.a;
    in >> c.b;
    in >> c.c;

    return in;
}

template<typename FieldT>
bool valid_vector(const linear_combination<FieldT> &r, const size_t num_vars)
{
    if (r.terms.empty())
    {
        return false;
    }

    std::set<var_index_t> s;
    for (size_t i = 0; i < r.terms.size(); ++i)
    {
        size_t idx = r.terms[i].index;

        if (idx > num_vars)
        {
            return false;
        }

        if (s.find(idx) != s.end())
        {
            return false;
        }
        s.insert(idx);
    }
    return true;
}

template<typename FieldT>
bool r1cs_constraint_system<FieldT>::is_valid() const
{
    if (this->num_inputs > this->num_vars) return false;

    for (size_t c = 0; c < constraints.size(); ++c)
    {
        if (!(valid_vector(constraints[c].a, this->num_vars) &&
              valid_vector(constraints[c].b, this->num_vars) &&
              valid_vector(constraints[c].c, this->num_vars)))
        {
            return false;
        }
    }

    return true;
}

template<typename FieldT>
void dump_product(const linear_combination<FieldT> &s, const r1cs_variable_assignment<FieldT> &w,
                  const std::map<size_t, std::string> &variable_annotations)
{
    for (size_t i = 0; i < s.terms.size(); ++i)
    {
        if (s.terms[i].index == 0)
        {
            printf("    1 * ");
            s.terms[i].coeff.print();
        }
        else
        {
            printf("    x_%zu * ", s.terms[i].index);
            s.terms[i].coeff.print();

            auto it = variable_annotations.find(s.terms[i].index);
            printf("    where x_%zu (%s) was assigned value ", s.terms[i].index,
                   (it == variable_annotations.end() ? "no annotation" : it->second.c_str()));
            w[s.terms[i].index-1].print();
            printf("      i.e. negative of ");
            (-w[s.terms[i].index-1]).print();
        }
    }
}

template<typename FieldT>
void dump_constraint(const r1cs_constraint<FieldT> &c, const r1cs_variable_assignment<FieldT> &w,
                     const std::map<size_t, std::string> &variable_annotations)
{
    printf("terms for a:\n"); dump_product(c.a, w, variable_annotations);
    printf("terms for b:\n"); dump_product(c.b, w, variable_annotations);
    printf("terms for c:\n"); dump_product(c.c, w, variable_annotations);
}

template<typename FieldT>
bool r1cs_constraint_system<FieldT>::is_satisfied(const r1cs_variable_assignment<FieldT> &w) const
{
    assert(w.size() == num_vars);

    for (size_t c = 0; c < constraints.size(); ++c)
    {
        FieldT ares = padded_inner_product(constraints[c].a, w);
        FieldT bres = padded_inner_product(constraints[c].b, w);
        FieldT cres = padded_inner_product(constraints[c].c, w);
        if (!(ares*bres == cres))
        {
#ifdef DEBUG
            auto it = constraint_annotations.find(c);
            printf("constraint %zu (%s) unsatisfied\n", c, (it == constraint_annotations.end() ? "no annotation" : it->second.c_str()));
            printf("<a,(1,x)> = "); ares.print();
            printf("<b,(1,x)> = "); bres.print();
            printf("<c,(1,x)> = "); cres.print();
            printf("constraint was:\n");
            dump_constraint(constraints[c], w, variable_annotations);
#endif // DEBUG
            return false;
        }
    }

    return true;
}

template<typename FieldT>
void r1cs_constraint_system<FieldT>::add_constraint(const r1cs_constraint<FieldT> &c)
{
    constraints.emplace_back(c);
}

template<typename FieldT>
void r1cs_constraint_system<FieldT>::add_constraint(const r1cs_constraint<FieldT> &c, const std::string &annotation)
{
#ifdef DEBUG
    constraint_annotations[constraints.size()] = annotation;
#endif
    constraints.emplace_back(c);
}

template<typename FieldT>
void r1cs_constraint_system<FieldT>::swap_AB_if_beneficial()
{
    enter_block("Call to r1cs_constraint_system::swap_AB_if_beneficial");

    enter_block("Estimate densities");
    bit_vector touched_by_A(this->num_vars + 1, false), touched_by_B(this->num_vars + 1, false);

    for (size_t i = 0; i < this->constraints.size(); ++i)
    {
        for (size_t j = 0; j < this->constraints[i].a.terms.size(); ++j)
        {
            touched_by_A[this->constraints[i].a.terms[j].index] = true;
        }

        for (size_t j = 0; j < this->constraints[i].b.terms.size(); ++j)
        {
            touched_by_B[this->constraints[i].b.terms[j].index] = true;
        }
    }

    size_t non_zero_A_count = 0, non_zero_B_count = 0;
    for (size_t i = 0; i < this->num_vars + 1; ++i)
    {
        non_zero_A_count += touched_by_A[i] ? 1 : 0;
        non_zero_B_count += touched_by_B[i] ? 1 : 0;
    }
    print_indent(); printf("* Non-zero A-count (estimate): %zu\n", non_zero_A_count);
    print_indent(); printf("* Non-zero B-count (estimate): %zu\n", non_zero_B_count);
    leave_block("Estimate densities");

    if (non_zero_B_count > non_zero_A_count)
    {
        enter_block("Perform the swap");
        for (size_t i = 0; i < this->constraints.size(); ++i)
        {
            std::swap(this->constraints[i].a, this->constraints[i].b);
        }
        leave_block("Perform the swap");
    }
    else
    {
        print_indent(); printf("Swap is not beneficial, not performing\n");
    }

    leave_block("Call to r1cs_constraint_system::swap_AB_if_beneficial");
}

template<typename FieldT>
bool r1cs_constraint_system<FieldT>::operator==(const r1cs_constraint_system<FieldT> &other) const
{
    return (this->constraints == other.constraints &&
            this->num_inputs == other.num_inputs &&
            this->num_vars == other.num_vars);
}

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const r1cs_constraint_system<FieldT> &cs)
{
    out << cs.num_inputs << "\n";
    out << cs.num_vars << "\n";

    out << cs.constraints.size() << "\n";
    for (const r1cs_constraint<FieldT>& c : cs.constraints)
    {
        out << c;
    }

    return out;
}

template<typename FieldT>
std::istream& operator>>(std::istream &in, r1cs_constraint_system<FieldT> &cs)
{
    in >> cs.num_inputs;
    in >> cs.num_vars;

    cs.constraints.clear();

    size_t s;
    in >> s;

    char b;
    in.read(&b, 1);

    cs.constraints.reserve(s);

    for (size_t i = 0; i < s; ++i)
    {
        r1cs_constraint<FieldT> c;
        in >> c;
        cs.constraints.emplace_back(c);
    }

    return in;
}

} // libsnark
#endif // R1CS_TCC_
