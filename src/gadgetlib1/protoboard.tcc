/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef PROTOBOARD_TCC_
#define PROTOBOARD_TCC_

#include <cstdio>
#include <cstdarg>
#include "common/profiling.hpp"

namespace libsnark {

template<typename FieldT>
protoboard<FieldT>::protoboard()
{
    constant_term = FieldT::one();

#ifdef DEBUG
    constraint_system.variable_annotations[0] = "ONE";
#endif

    constraint_system.num_vars = 0;
    constraint_system.num_inputs = 0;
    next_free_var = 1;
    next_free_lc = 0;
}

template<typename FieldT>
var_index_t protoboard<FieldT>::allocate_var_index(const std::string &annotation)
{
    ++constraint_system.num_vars;
#ifdef DEBUG
    assert(annotation != "");
    constraint_system.variable_annotations[constraint_system.num_vars] = annotation;
#endif
    values.emplace_back(FieldT::zero());
    return next_free_var++;
}

template<typename FieldT>
lc_index_t protoboard<FieldT>::allocate_lc_index()
{
    lc_values.emplace_back(FieldT::zero());
    return next_free_lc++;
}

template<typename FieldT>
FieldT& protoboard<FieldT>::val(const pb_variable<FieldT> &var)
{
    assert(var.index <= values.size());
    return (var.index == 0 ? constant_term : values[var.index-1]);
}

template<typename FieldT>
FieldT protoboard<FieldT>::val(const pb_variable<FieldT> &var) const
{
    assert(var.index <= values.size());
    return (var.index == 0 ? constant_term : values[var.index-1]);
}

template<typename FieldT>
FieldT& protoboard<FieldT>::lc_val(const pb_linear_combination<FieldT> &lc)
{
    if (lc.is_variable)
    {
        return this->val(pb_variable<FieldT>(lc.index));
    }
    else
    {
        assert(lc.index < lc_values.size());
        return lc_values[lc.index];
    }
}

template<typename FieldT>
FieldT protoboard<FieldT>::lc_val(const pb_linear_combination<FieldT> &lc) const
{
    if (lc.is_variable)
    {
        return this->val(pb_variable<FieldT>(lc.index));
    }
    else
    {
        assert(lc.index < lc_values.size());
        return lc_values[lc.index];
    }
}

template<typename FieldT>
void protoboard<FieldT>::add_r1cs_constraint(const r1cs_constraint<FieldT> &constr, const std::string &annotation)
{
#ifdef DEBUG
    assert(annotation != "");
    constraint_system.constraint_annotations[constraint_system.constraints.size()] = annotation;
#endif
    constraint_system.constraints.emplace_back(constr);
}

template<typename FieldT>
void protoboard<FieldT>::augment_annotation(const pb_variable<FieldT> &v, const std::string &postfix)
{
#ifdef DEBUG
    auto it = constraint_system.variable_annotations.find(v.index);
    constraint_system.variable_annotations[v.index] = (it == constraint_system.variable_annotations.end() ? "" : it->second) + postfix;
#endif
}

template<typename FieldT>
bool protoboard<FieldT>::is_satisfied()
{
    return constraint_system.is_satisfied(values);
}

template<typename FieldT>
void protoboard<FieldT>::dump_variables()
{
#ifdef DEBUG
    for (size_t i = 0; i < constraint_system.num_vars; ++i)
    {
        printf("%-40s --> ", constraint_system.variable_annotations[i].c_str());
        values[i].as_bigint().print_hex();
    }
#endif
}

} // libsnark
#endif // PROTOBOARD_TCC_
