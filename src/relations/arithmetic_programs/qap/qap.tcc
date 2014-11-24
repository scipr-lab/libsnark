/** @file
*****************************************************************************

Implementation of interfaces for a QAP ("Quadratic Arithmetic Program").

See qap.hpp .

*****************************************************************************
* @author     This file is part of libsnark, developed by SCIPR Lab
*             and contributors (see AUTHORS).
* @copyright  MIT license (see LICENSE file)
*****************************************************************************/

#ifndef QAP_TCC_
#define QAP_TCC_

#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "algebra/evaluation_domain/evaluation_domain.hpp"

namespace libsnark {

template<typename FieldT>
bool qap_instance<FieldT>::is_satisfied(const qap_witness<FieldT> &witness) const
{

    qap_instance_evaluation<FieldT> eval_qap_inst;

    eval_qap_inst.domain = this->domain;

    eval_qap_inst.num_vars = this->num_vars;
    eval_qap_inst.degree = this->degree;
    eval_qap_inst.num_inputs = this->num_inputs;

    eval_qap_inst.t = FieldT::random_element();;

    eval_qap_inst.At.resize(eval_qap_inst.num_vars+1, FieldT::zero());
    eval_qap_inst.Bt.resize(eval_qap_inst.num_vars+1, FieldT::zero());
    eval_qap_inst.Ct.resize(eval_qap_inst.num_vars+1, FieldT::zero());
    eval_qap_inst.Ht.reserve(eval_qap_inst.degree+1);

    eval_qap_inst.Zt = eval_qap_inst.domain->compute_Z(eval_qap_inst.t);

    const std::vector<FieldT> u = eval_qap_inst.domain->lagrange_coeffs(eval_qap_inst.t);

    for (size_t i = 0; i < eval_qap_inst.num_vars+1; ++i)
    {
        for (auto &el : A_in_Lagrange_basis[i])
        {
            eval_qap_inst.At[i] += u[el.first] * el.second;
        }

        for (auto &el : B_in_Lagrange_basis[i])
        {
            eval_qap_inst.Bt[i] += u[el.first] * el.second;
        }

        for (auto &el : C_in_Lagrange_basis[i])
        {
            eval_qap_inst.Ct[i] += u[el.first] * el.second;
        }
    }

    FieldT ti = FieldT::one();
    for (size_t i = 0; i < eval_qap_inst.degree+1; ++i)
    {
        eval_qap_inst.Ht.emplace_back(ti);
        ti *= eval_qap_inst.t;
    }

    return eval_qap_inst.is_satisfied(witness);
}

template<typename FieldT>
bool qap_instance_evaluation<FieldT>::is_satisfied(const qap_witness<FieldT> &witness) const
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

    if (this->num_vars != witness.coefficients_for_ABCs.size())
    {
        return false;
    }

    if (this->degree+1 != witness.coefficients_for_H.size())
    {
        return false;
    }

    if (this->At.size() != this->num_vars+1 || this->Bt.size() != this->num_vars+1 || this->Ct.size() != this->num_vars+1)
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

    FieldT ans_A = this->At[0] + witness.d1*this->Zt;
    FieldT ans_B = this->Bt[0] + witness.d2*this->Zt;
    FieldT ans_C = this->Ct[0] + witness.d3*this->Zt;
    FieldT ans_H = FieldT::zero();

    ans_A = ans_A + naive_plain_exp<FieldT, FieldT>(this->At.begin()+1, this->At.begin()+1+this->num_vars,
                                                    witness.coefficients_for_ABCs.begin(), witness.coefficients_for_ABCs.begin()+this->num_vars);
    ans_B = ans_B + naive_plain_exp<FieldT, FieldT>(this->Bt.begin()+1, this->Bt.begin()+1+this->num_vars,
                                                    witness.coefficients_for_ABCs.begin(), witness.coefficients_for_ABCs.begin()+this->num_vars);
    ans_C = ans_C + naive_plain_exp<FieldT, FieldT>(this->Ct.begin()+1, this->Ct.begin()+1+this->num_vars,
                                                    witness.coefficients_for_ABCs.begin(), witness.coefficients_for_ABCs.begin()+this->num_vars);
    ans_H = ans_H + naive_plain_exp<FieldT, FieldT>(this->Ht.begin(), this->Ht.begin()+this->degree+1,
                                                    witness.coefficients_for_H.begin(), witness.coefficients_for_H.begin()+this->degree+1);

    if (ans_A * ans_B - ans_C != ans_H * this->Zt)
    {
        return false;
    }

    return true;
}

} // libsnark

#endif // QAP_TCC_
