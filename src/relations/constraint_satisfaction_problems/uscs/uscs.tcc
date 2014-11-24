/** @file
 *****************************************************************************

 Implementation of interfaces for:
 - a USCS constraint,
 - a USCS variable assignment, and
 - a USCS constraint system.

 See uscs.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef USCS_TCC_
#define USCS_TCC_

#include "relations/constraint_satisfaction_problems/uscs/uscs.hpp"

#include <algorithm>
#include <cassert>
#include <set>
#include "common/utils.hpp"
#include "common/profiling.hpp"
#include "algebra/fields/bigint.hpp"

namespace libsnark {

template<typename FieldT>
bool uscs_constraint_system<FieldT>::is_valid() const
{
    if (this->num_inputs > this->num_vars) return false;

    for (size_t c = 0; c < constraints.size(); ++c)
    {
        if (!valid_vector(constraints[c], this->num_vars))
        {
            return false;
        }
    }

    return true;
}

template<typename FieldT>
void dump_uscs_constraint(const uscs_constraint<FieldT> &c, const uscs_variable_assignment<FieldT> &w,
                          const std::map<size_t, std::string> &variable_annotations)
{
    printf("terms:\n");
    c.print_with_assignment(w, variable_annotations);
}

template<typename FieldT>
bool uscs_constraint_system<FieldT>::is_satisfied(const uscs_variable_assignment<FieldT> &w) const
{
    assert(w.size() == num_vars);

    for (size_t c = 0; c < constraints.size(); ++c)
    {
        FieldT res = constraints[c].evaluate(w);
        if (!(res.squared() == FieldT::one()))
        {
#ifdef DEBUG
            auto it = constraint_annotations.find(c);
            printf("constraint %zu (%s) unsatisfied\n", c, (it == constraint_annotations.end() ? "no annotation" : it->second.c_str()));
            printf("<a,(1,x)> = "); res.print();
            printf("constraint was:\n");
            dump_uscs_constraint(constraints[c], w, variable_annotations);
#endif // DEBUG
            return false;
        }
    }

    return true;
}

template<typename FieldT>
void uscs_constraint_system<FieldT>::add_constraint(const uscs_constraint<FieldT> &c)
{
    constraints.emplace_back(c);
}

template<typename FieldT>
void uscs_constraint_system<FieldT>::add_constraint(const uscs_constraint<FieldT> &c, const std::string &annotation)
{
#ifdef DEBUG
    constraint_annotations[constraints.size()] = annotation;
#endif
    constraints.emplace_back(c);
}

template<typename FieldT>
bool uscs_constraint_system<FieldT>::operator==(const uscs_constraint_system<FieldT> &other) const
{
    return (this->constraints == other.constraints &&
            this->num_inputs == other.num_inputs &&
            this->num_vars == other.num_vars);
}

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const uscs_constraint_system<FieldT> &cs)
{
    out << cs.num_inputs << "\n";
    out << cs.num_vars << "\n";

    out << cs.constraints.size() << "\n";
    for (const uscs_constraint<FieldT>& c : cs.constraints)
    {
        out << c;
    }

    return out;
}

template<typename FieldT>
std::istream& operator>>(std::istream &in, uscs_constraint_system<FieldT> &cs)
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
        uscs_constraint<FieldT> c;
        in >> c;
        cs.constraints.emplace_back(c);
    }

    return in;
}

template<typename FieldT>
void uscs_constraint_system<FieldT>::report_statistics() const
{
#ifdef DEBUG
    for (size_t i = 0; i < constraints.size(); ++i)
    {
        auto &constr = constraints[i];
        bool a_is_const = true;
        for (auto &t : constr.terms)
        {
            a_is_const = a_is_const && (t.index == 0);
        }

        if (a_is_const)
        {
            auto it = constraint_annotations.find(i);
            printf("%s\n", (it == constraint_annotations.end() ? FORMAT("", "constraint_%zu", i) : it->second).c_str());
        }
    }
#endif
}

} // libsnark

#endif // USCS_TCC_
