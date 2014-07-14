/** @file
 *****************************************************************************
 Implementation of an adapter for interfacing to SNARKs.
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "adapters.hpp"

using gadgetlib2::Variable;
using gadgetlib2::Rank1Constraint;

namespace gadgetlib2 {

typedef GadgetLibAdapter GLA;

GLA::linear_term_t GLA::convert(const LinearTerm& lt) const {
    const variable_index_t var = lt.variable_.index_;
    const Fp_elem_t coeff = convert(lt.coeff_);
    return{ var, coeff };
}

GLA::linear_combination_t GLA::convert(const LinearCombination& lc) const {
    sparse_vec_t sparse_vec;
    sparse_vec.reserve(lc.linearTerms_.size());
    for (auto lt : lc.linearTerms_) {
        sparse_vec.emplace_back(convert(lt));
    }
    const Fp_elem_t offset = convert(lc.constant_);
    return{ sparse_vec, offset };
}

GLA::constraint_t GLA::convert(const Constraint& constraint) const {
    const auto rank1_constraint = dynamic_cast<const Rank1Constraint&>(constraint);
    return constraint_t(convert(rank1_constraint.a()),
        convert(rank1_constraint.b()),
        convert(rank1_constraint.c()));
}

GLA::constraint_sys_t GLA::convert(const ConstraintSystem& constraint_sys) const {
    constraint_sys_t retval;
    retval.reserve(constraint_sys.constraintsPtrs_.size());
    for (auto constraintPtr : constraint_sys.constraintsPtrs_) {
        retval.emplace_back(convert(*constraintPtr));
    }
    return retval;
}

GLA::assignment_t GLA::convert(const VariableAssignment& assignment) const {
    assignment_t retval;
    for (const auto assignmentPair : assignment) {
        const variable_index_t var = assignmentPair.first.index_;
        const Fp_elem_t elem = convert(assignmentPair.second);
        retval[var] = elem;
    }
    return retval;
}

void GLA::resetVariableIndex() { // This is a hack, used for testing
    Variable::nextFreeIndex_ = 0;
}

GLA::protoboard_t GLA::convert(const Protoboard& pb) const {
    return protoboard_t(convert(pb.constraintSystem()), convert(pb.assignment()));
}

GLA::Fp_elem_t GLA::convert(FElem fElem) const {
    using gadgetlib2::R1P_Elem;
    fElem.promoteToFieldType(gadgetlib2::R1P);
    const R1P_Elem* pR1P = dynamic_cast<R1P_Elem*>(fElem.elem_.get());
    return pR1P->elem_;
}

bool operator==(const GLA::linear_combination_t& lhs,
    const GLA::linear_term_t& rhs) {
    return lhs.first.size() == 1 &&
        lhs.first.at(0) == rhs &&
        lhs.second == Fp(0);
}

}
