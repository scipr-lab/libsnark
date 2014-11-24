/** @file
 *****************************************************************************

 Implementation of interfaces for:
 - a variable (i.e., x_i),
 - a linear term (i.e., a_i * x_i), and
 - a linear combination (i.e., sum_i a_i * x_i).

 See variabe.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef VARIABLE_TCC_
#define VARIABLE_TCC_

#include <cassert>

#include "algebra/fields/bigint.hpp"

namespace libsnark {

template<typename FieldT>
linear_term<FieldT> variable<FieldT>::operator*(const integer_coeff_t i) const
{
    return linear_term<FieldT>(*this, i);
}

template<typename FieldT>
linear_term<FieldT> variable<FieldT>::operator*(const FieldT &el) const
{
    return linear_term<FieldT>(*this, el);
}

template<typename FieldT>
linear_combination<FieldT> variable<FieldT>::operator+(const linear_combination<FieldT> &other) const
{
    linear_combination<FieldT> result;

    result.add_term(*this);
    result.terms.insert(result.terms.begin(), other.terms.begin(), other.terms.end());

    return result;
}

template<typename FieldT>
linear_combination<FieldT> variable<FieldT>::operator-(const linear_combination<FieldT> &other) const
{
    return (*this) + (-other);
}

template<typename FieldT>
linear_term<FieldT> variable<FieldT>::operator-() const
{
    return linear_term<FieldT>(*this, -FieldT::one());
}

template<typename FieldT>
bool variable<FieldT>::operator==(const variable<FieldT> &other) const
{
    return (this->index == other.index);
}

template<typename FieldT>
linear_term<FieldT> operator*(const integer_coeff_t i, const variable<FieldT> &var)
{
    return linear_term<FieldT>(var, i);
}

template<typename FieldT>
linear_term<FieldT> operator*(const FieldT &el, const variable<FieldT> &var)
{
    return linear_term<FieldT>(var, el);
}

template<typename FieldT>
linear_combination<FieldT> operator+(const integer_coeff_t i, const variable<FieldT> &var)
{
    return linear_combination<FieldT>(i) + var;
}

template<typename FieldT>
linear_combination<FieldT> operator+(const FieldT &el, const variable<FieldT> &var)
{
    return linear_combination<FieldT>(el) + var;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const integer_coeff_t i, const variable<FieldT> &var)
{
    return linear_combination<FieldT>(i) - var;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const FieldT &el, const variable<FieldT> &var)
{
    return linear_combination<FieldT>(el) - var;
}

template<typename FieldT>
linear_term<FieldT>::linear_term(const variable<FieldT> &v) :
    index(v.index), coeff(FieldT::one())
{
}

template<typename FieldT>
linear_term<FieldT>::linear_term(const variable<FieldT> &v, const integer_coeff_t int_coeff) :
    index(v.index), coeff(FieldT(int_coeff))
{
}

template<typename FieldT>
linear_term<FieldT>::linear_term(const variable<FieldT> &v, const FieldT &coeff) :
    index(v.index), coeff(coeff)
{
}

template<typename FieldT>
linear_term<FieldT> linear_term<FieldT>::operator*(const integer_coeff_t i) const
{
    return (this->operator*(FieldT(i)));
}

template<typename FieldT>
linear_term<FieldT> linear_term<FieldT>::operator*(const FieldT &el) const
{
    return linear_term<FieldT>(this->index, el * this->coeff);
}

template<typename FieldT>
linear_combination<FieldT> operator+(const integer_coeff_t i, const linear_term<FieldT> &lt)
{
    return linear_combination<FieldT>(i) + lt;
}

template<typename FieldT>
linear_combination<FieldT> operator+(const FieldT &el, const linear_term<FieldT> &lt)
{
    return linear_combination<FieldT>(el) + lt;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const integer_coeff_t i, const linear_term<FieldT> &lt)
{
    return linear_combination<FieldT>(i) - lt;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const FieldT &el, const linear_term<FieldT> &lt)
{
    return linear_combination<FieldT>(el) - lt;
}

template<typename FieldT>
linear_combination<FieldT> linear_term<FieldT>::operator+(const linear_combination<FieldT> &other) const
{
    return linear_combination<FieldT>(*this) + other;
}

template<typename FieldT>
linear_combination<FieldT> linear_term<FieldT>::operator-(const linear_combination<FieldT> &other) const
{
    return (*this) + (-other);
}

template<typename FieldT>
linear_term<FieldT> linear_term<FieldT>::operator-() const
{
    return linear_term<FieldT>(this->index, -this->coeff);
}

template<typename FieldT>
bool linear_term<FieldT>::operator==(const linear_term<FieldT> &other) const
{
    return (this->index == other.index &&
            this->coeff == other.coeff);
}

template<typename FieldT>
linear_term<FieldT> operator*(const integer_coeff_t i, const linear_term<FieldT> &lt)
{
    return FieldT(i) * lt;
}

template<typename FieldT>
linear_term<FieldT> operator*(const FieldT &el, const linear_term<FieldT> &lt)
{
    return linear_term<FieldT>(lt.index, el * lt.coeff);
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const integer_coeff_t int_coeff)
{
    this->add_term(linear_term<FieldT>(0, int_coeff));
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const FieldT &coeff)
{
    this->add_term(linear_term<FieldT>(0, coeff));
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const variable<FieldT> &var)
{
    this->add_term(var);
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const linear_term<FieldT> &lt)
{
    this->add_term(lt);
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const variable<FieldT> &v)
{
    this->terms.emplace_back(linear_term<FieldT>(v.index, FieldT::one()));
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const variable<FieldT> &v, const integer_coeff_t int_coeff)
{
    this->terms.emplace_back(linear_term<FieldT>(v.index, int_coeff));
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const variable<FieldT> &v, const FieldT &coeff)
{
    this->terms.emplace_back(linear_term<FieldT>(v.index, coeff));
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const linear_term<FieldT> &other)
{
    this->terms.emplace_back(other);
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator*(const integer_coeff_t i) const
{
    return (*this) * FieldT(i);
}

template<typename FieldT>
FieldT linear_combination<FieldT>::evaluate(const std::vector<FieldT> &va) const
{
    FieldT acc = FieldT::zero();
    for (auto &t : terms)
    {
        acc += (t.index == 0 ? FieldT::one() : va[t.index-1]) * t.coeff;
    }
    return acc;
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator*(const FieldT &el) const
{
    linear_combination<FieldT> result;
    result.terms.reserve(this->terms.size());
    for (const linear_term<FieldT> &t : this->terms)
    {
        result.terms.emplace_back(t * el);
    }
    return result;
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator+(const linear_combination<FieldT> &other) const
{
    linear_combination<FieldT> result;

    auto it1 = this->terms.begin();
    auto it2 = other.terms.begin();

    /* invariant: it1 and it2 always point to unprocessed items in the corresponding linear combinations */
    while (it1 != this->terms.end() && it2 != other.terms.end())
    {
        if (it1->index < it2->index)
        {
            result.terms.emplace_back(*it1);
            ++it1;
        }
        else if (it1->index > it2->index)
        {
            result.terms.emplace_back(*it2);
            ++it2;
        }
        else
        {
            /* it1->index == it2->index */
            result.terms.emplace_back(linear_term<FieldT>(variable<FieldT>(it1->index), it1->coeff + it2->coeff));
            ++it1;
            ++it2;
        }
    }

    if (it1 != this->terms.end())
    {
        result.terms.insert(result.terms.end(), it1, this->terms.end());
    }
    else
    {
        result.terms.insert(result.terms.end(), it2, other.terms.end());
    }

    return result;
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator-(const linear_combination<FieldT> &other) const
{
    return (*this) + (-other);
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator-() const
{
    return (*this) * (-FieldT::one());
}

template<typename FieldT>
bool linear_combination<FieldT>::operator==(const linear_combination<FieldT> &other) const
{
    return (this->terms == other.terms);
}

template<typename FieldT>
bool linear_combination<FieldT>::is_valid(const size_t num_vars) const
{
    for (size_t i = 1; i < terms.size(); ++i)
    {
        if (terms[i-1].index >= terms[i].index)
        {
            return false;
        }
    }

    return true;
}

template<typename FieldT>
void linear_combination<FieldT>::print(const std::map<size_t, std::string> &variable_annotations) const
{
    for (auto &t : terms)
    {
        if (t.index == 0)
        {
            printf("    1 * ");
            t.coeff.print();
        }
        else
        {
            auto it = variable_annotations.find(t.index);
            printf("    x_%zu (%s) * ", t.index, (it == variable_annotations.end() ? "no annotation" : it->second.c_str()));
            t.coeff.print();
        }
    }
}

template<typename FieldT>
void linear_combination<FieldT>::print_with_assignment(const std::vector<FieldT> &v, const std::map<size_t, std::string> &variable_annotations) const
{
    for (auto &t : terms)
    {
        if (t.index == 0)
        {
            printf("    1 * ");
            t.coeff.print();
        }
        else
        {
            printf("    x_%zu * ", t.index);
            t.coeff.print();

            auto it = variable_annotations.find(t.index);
            printf("    where x_%zu (%s) was assigned value ", t.index,
                   (it == variable_annotations.end() ? "no annotation" : it->second.c_str()));
            v[t.index-1].print();
            printf("      i.e. negative of ");
            (-v[t.index-1]).print();
        }
    }
}

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const linear_combination<FieldT> &sv)
{
    out << sv.terms.size() << "\n";
    for (const linear_term<FieldT>& lt : sv.terms)
    {
        out << lt.index << "\n";
        out << lt.coeff << OUTPUT_NEWLINE;
    }

    return out;
}

template<typename FieldT>
std::istream& operator>>(std::istream &in, linear_combination<FieldT> &sv)
{
    sv.terms.clear();

    size_t s;
    in >> s;

    consume_newline(in);

    sv.terms.reserve(s);

    for (size_t i = 0; i < s; ++i)
    {
        linear_term<FieldT> lt;
        in >> lt.index;
        consume_newline(in);
        in >> lt.coeff;
        consume_OUTPUT_NEWLINE(in);
        sv.terms.emplace_back(lt);
    }

    return in;
}

template<typename FieldT>
linear_combination<FieldT> operator*(const integer_coeff_t i, const linear_combination<FieldT> &lc)
{
    return lc * i;
}

template<typename FieldT>
linear_combination<FieldT> operator*(const FieldT &el, const linear_combination<FieldT> &lc)
{
    return lc * el;
}

template<typename FieldT>
linear_combination<FieldT> operator+(const integer_coeff_t i, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(i) + lc;
}

template<typename FieldT>
linear_combination<FieldT> operator+(const FieldT &el, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(el) + lc;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const integer_coeff_t i, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(i) - lc;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const FieldT &el, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(el) - lc;
}

} // libsnark

#endif // VARIABLE_TCC
