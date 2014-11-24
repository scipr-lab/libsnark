/** @file
 *****************************************************************************

 Implementation of interfaces for a SSP ("Square Span Program").

 See ssp.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef SSP_TCC_
#define SSP_TCC_

#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "algebra/evaluation_domain/evaluation_domain.hpp"

namespace libsnark {

template<typename FieldT>
bool ssp_instance<FieldT>::is_satisfied(const ssp_witness<FieldT> &witness) const
{

    ssp_instance_evaluation<FieldT> eval_ssp_inst;

    eval_ssp_inst.domain = this->domain;

    eval_ssp_inst.num_vars = this->num_vars;
    eval_ssp_inst.degree = this->degree;
    eval_ssp_inst.num_inputs = this->num_inputs;

    eval_ssp_inst.t = FieldT::random_element();;

    eval_ssp_inst.Vt.resize(eval_ssp_inst.num_vars+1, FieldT::zero());
    eval_ssp_inst.Ht.reserve(eval_ssp_inst.degree+1);

    eval_ssp_inst.Zt = eval_ssp_inst.domain->compute_Z(eval_ssp_inst.t);

    const std::vector<FieldT> u = eval_ssp_inst.domain->lagrange_coeffs(eval_ssp_inst.t);

    for (size_t i = 0; i < eval_ssp_inst.num_vars+1; ++i)
    {
        for (auto &el : V_in_Lagrange_basis[i])
        {
            eval_ssp_inst.Vt[i] += u[el.first] * el.second;
        }
    }

    FieldT ti = FieldT::one();
    for (size_t i = 0; i < eval_ssp_inst.degree+1; ++i)
    {
        eval_ssp_inst.Ht.emplace_back(ti);
        ti *= eval_ssp_inst.t;
    }

    return eval_ssp_inst.is_satisfied(witness);
}

template<typename FieldT>
bool ssp_instance_evaluation<FieldT>::is_satisfied(const ssp_witness<FieldT> &witness) const
{

    if (this->num_vars != witness.num_vars)
    {
        return false;
    }

    if (this->degree != witness.degree)
    {
        return false;
    }

    if (this->num_inputs != witness.num_inputs)
    {
        return false;
    }

    if (this->num_vars != witness.coefficients_for_Vs.size())
    {
        return false;
    }

    if (this->degree+1 != witness.coefficients_for_H.size())
    {
        return false;
    }

    if (this->Vt.size() != this->num_vars+1)
    {
        return false;
    }

    if (this->Ht.size() != this->degree+1)
    {
        return false;
    }

    if (this->Zt != this->domain->compute_Z(this->t))
    {
        return false;
    }

    FieldT ans_V = this->Vt[0] + witness.d*this->Zt;
    FieldT ans_H = FieldT::zero();

    ans_V = ans_V + naive_plain_exp<FieldT, FieldT>(this->Vt.begin()+1, this->Vt.begin()+1+this->num_vars,
                                                    witness.coefficients_for_Vs.begin(), witness.coefficients_for_Vs.begin()+this->num_vars);
    ans_H = ans_H + naive_plain_exp<FieldT, FieldT>(this->Ht.begin(), this->Ht.begin()+this->degree+1,
                                                    witness.coefficients_for_H.begin(), witness.coefficients_for_H.begin()+this->degree+1);

    if (ans_V.squared() - FieldT::one() != ans_H * this->Zt)
    {
        return false;
    }

    return true;
}

} // libsnark

#endif // SSP_TCC_
