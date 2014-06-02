/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

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
    linear_combination<FieldT> result;

    result.add_term(*this);
    result.terms.insert(result.terms.begin(), other.terms.begin(), other.terms.end());

    return result;
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
    result.terms.insert(result.terms.end(), this->terms.begin(), this->terms.end());
    result.terms.insert(result.terms.end(), other.terms.begin(), other.terms.end());
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
