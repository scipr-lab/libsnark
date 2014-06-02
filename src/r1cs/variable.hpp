/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef VARIABLE_HPP_
#define VARIABLE_HPP_

#include <cstddef>
#include <string>
#include <vector>

namespace libsnark {

typedef size_t var_index_t;
typedef long integer_coeff_t;

template<typename FieldT>
class linear_term;

template<typename FieldT>
class linear_combination;

template<typename FieldT>
class variable {
public:
    // corresponds to the variable "x_{index}"
    var_index_t index;

    variable(const var_index_t index = 0) : index(index) {};

    linear_term<FieldT> operator*(const integer_coeff_t i) const;
    linear_term<FieldT> operator*(const FieldT &el) const;

    linear_combination<FieldT> operator+(const linear_combination<FieldT> &other) const;
    linear_combination<FieldT> operator-(const linear_combination<FieldT> &other) const;

    linear_term<FieldT> operator-() const;

    bool operator==(const variable<FieldT> &other) const;
};

template<typename FieldT>
linear_term<FieldT> operator*(const integer_coeff_t i, const variable<FieldT> &var);

template<typename FieldT>
linear_term<FieldT> operator*(const FieldT &el, const variable<FieldT> &var);

template<typename FieldT>
linear_combination<FieldT> operator+(const integer_coeff_t i, const variable<FieldT> &var);

template<typename FieldT>
linear_combination<FieldT> operator+(const FieldT &el, const variable<FieldT> &var);

template<typename FieldT>
linear_combination<FieldT> operator-(const integer_coeff_t i, const variable<FieldT> &var);

template<typename FieldT>
linear_combination<FieldT> operator-(const FieldT &el, const variable<FieldT> &var);

template<typename FieldT>
class linear_term {
public:
    // corresponds to the linear term "coeff * x_{index}"
    var_index_t index;
    FieldT coeff;

    linear_term() {};
    linear_term(const variable<FieldT> &v);
    linear_term(const variable<FieldT> &v, const integer_coeff_t int_coeff);
    linear_term(const variable<FieldT> &v, const FieldT &coeff);

    linear_term<FieldT> operator*(const integer_coeff_t i) const;
    linear_term<FieldT> operator*(const FieldT &el) const;
    linear_combination<FieldT> operator+(const linear_combination<FieldT> &other) const;
    linear_combination<FieldT> operator-(const linear_combination<FieldT> &other) const;
    linear_term<FieldT> operator-() const;
    bool operator==(const linear_term<FieldT> &other) const;
};

template<typename FieldT>
linear_term<FieldT> operator*(const integer_coeff_t i, const linear_term<FieldT> &lt);

template<typename FieldT>
linear_term<FieldT> operator*(const FieldT &el, const linear_term<FieldT> &lt);

template<typename FieldT>
linear_combination<FieldT> operator+(const integer_coeff_t i, const linear_term<FieldT> &lt);

template<typename FieldT>
linear_combination<FieldT> operator+(const FieldT &el, const linear_term<FieldT> &lt);

template<typename FieldT>
linear_combination<FieldT> operator-(const integer_coeff_t i, const linear_term<FieldT> &lt);

template<typename FieldT>
linear_combination<FieldT> operator-(const FieldT &el, const linear_term<FieldT> &lt);


template<typename FieldT>
class linear_combination;
template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const linear_combination<FieldT> &sv);
template<typename FieldT>
std::istream& operator>>(std::istream &in, linear_combination<FieldT> &sv);

template<typename FieldT>
class linear_combination {
public:
    // a list of linear terms
    std::vector<linear_term<FieldT> > terms;

    linear_combination() {};
    linear_combination(const integer_coeff_t int_coeff);
    linear_combination(const FieldT &coeff);
    linear_combination(const variable<FieldT> &var);
    linear_combination(const linear_term<FieldT> &lt);

    void add_term(const variable<FieldT> &v);
    void add_term(const variable<FieldT> &v, const integer_coeff_t int_coeff);
    void add_term(const variable<FieldT> &v, const FieldT &coeff);

    void add_term(const linear_term<FieldT> &v);

    linear_combination<FieldT> operator*(const integer_coeff_t i) const;
    linear_combination<FieldT> operator*(const FieldT &el) const;

    linear_combination<FieldT> operator+(const linear_combination<FieldT> &other) const;
    linear_combination<FieldT> operator-(const linear_combination<FieldT> &other) const;
    linear_combination<FieldT> operator-() const;

    bool operator==(const linear_combination<FieldT> &other) const;
    friend std::ostream& operator<< <FieldT>(std::ostream &out, const linear_combination<FieldT> &sv);
    friend std::istream& operator>> <FieldT>(std::istream &in, linear_combination<FieldT> &sv);
};

template<typename FieldT>
linear_combination<FieldT> operator*(const integer_coeff_t i, const linear_combination<FieldT> &lc);
template<typename FieldT>
linear_combination<FieldT> operator*(const FieldT &el, const linear_combination<FieldT> &lc);

template<typename FieldT>
linear_combination<FieldT> operator+(const integer_coeff_t i, const linear_combination<FieldT> &lc);
template<typename FieldT>
linear_combination<FieldT> operator+(const FieldT &el, const linear_combination<FieldT> &lc);

template<typename FieldT>
linear_combination<FieldT> operator-(const integer_coeff_t i, const linear_combination<FieldT> &lc);
template<typename FieldT>
linear_combination<FieldT> operator-(const FieldT &el, const linear_combination<FieldT> &lc);


} // libsnark
#include "r1cs/variable.tcc"

#endif // VARIABLE_HPP_
