/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef PROTOBOARD_HPP_
#define PROTOBOARD_HPP_

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include "gadgetlib1/pb_variable.hpp"
#include "r1cs/r1cs.hpp"
#include "common/utils.hpp"

namespace libsnark {

template<typename FieldT>
class r1cs_constraint;

template<typename FieldT>
class r1cs_constraint_system;

template<typename FieldT>
class protoboard {
private:
    FieldT constant_term;
public:
    var_index_t next_free_var;
    lc_index_t next_free_lc;

    /* values[i] is the value of the i-th variable (set in witness
       generation), values[0] will always hold 1 */
    std::vector<FieldT> values;
    r1cs_constraint_system<FieldT> constraint_system;

    std::vector<FieldT> lc_values;

    protoboard();
private:
    var_index_t allocate_var_index(const std::string &annotation="");
    lc_index_t allocate_lc_index();

public:
    FieldT& val(const pb_variable<FieldT> &var);
    FieldT val(const pb_variable<FieldT> &var) const;

    FieldT& lc_val(const pb_linear_combination<FieldT> &lc);
    FieldT lc_val(const pb_linear_combination<FieldT> &lc) const;

    void add_r1cs_constraint(const r1cs_constraint<FieldT> &constr, const std::string &annotation="");
    void augment_annotation(const pb_variable<FieldT> &v, const std::string &postfix);
    bool is_satisfied();
    void dump_variables();

    size_t num_constraints() { return constraint_system.constraints.size(); }
    size_t num_vars() { return next_free_var - 1; }

    friend class pb_variable<FieldT>;
    friend class pb_linear_combination<FieldT>;
};

} // libsnark
#include "gadgetlib1/protoboard.tcc"
#endif // PROTOBOARD_HPP_
