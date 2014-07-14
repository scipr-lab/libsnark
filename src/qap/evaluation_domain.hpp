/** @file
 *****************************************************************************
 Declaration of evaluation domains, and functions to select and use them.

 Both Lagrange evaluations and FFT/iFFT computations are conducted over
 specially-chosen subsets S of the field F, for efficiency reasons.

 #### Lagrange polynomials

 The descriptions below make use of the definition of a *Lagrange polynomial*.
 Given a field F, a subset S=(a_i)_i of F, and an index idx in {0,...,|S-1|},
 the idx-th Lagrange polynomial (wrt to subset S) is defined to be
 \f[   L_{idx,S}(z) := prod_{k \neq idx} (z - a_k) / prod_{k \neq idx} (a_{idx} - a_k)   \f]
 Note that, by construction:
 \f[   \forall j \neq idx: L_{idx,S}(a_{idx}) = 1  \text{ and }  L_{idx,S}(a_j) = 0   \f]

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef EVALUATION_DOMAIN_HPP_
#define EVALUATION_DOMAIN_HPP_

#include <memory>

namespace libsnark {

/*
 Both Lagrange evaluations and FFT/iFFT computations are conducted over
 specially-chosen subsets S of the field F, for efficiency reasons.

 The two functions below provide auxiliary functionality for:
 - computing the size of S (given a desired lower bound on its size)
 - accessing individual elements of S
*/

/*
 LAGRANGE POLYNOMIALS

 The descriptions below make use of the definition of a *Lagrange polynomial*.
 Given a field F, a subset S=(a_i)_i of F, and an index idx in {0,...,|S-1|},
 the idx-th Lagrange polynomial (wrt to subset S) is defined to be
   L_{idx,S}(z) := prod_{k \neq idx} (z - a_k) / prod_{k \neq idx} (a_idx - a_k)
 Note that, by construction,
   L_{idx,S}(a_idx) = 1 and L_{idx,S}(a_j) = 0 for all j \neq idx.
*/

template<typename FieldT>
class evaluation_domain {
public:
    const size_t m;
    evaluation_domain(const size_t m) : m(m) {};

    virtual void FFT(std::vector<FieldT> &a) = 0;
    virtual void iFFT(std::vector<FieldT> &a) = 0;
    virtual void cosetFFT(std::vector<FieldT> &a, const FieldT &g) = 0;
    virtual void icosetFFT(std::vector<FieldT> &a, const FieldT &g) = 0;

/*
 * Evaluation of all Lagrange polynomials.
 *
 * The inputs are:
 * - an integer m
 * - an element t
 * The output is a vector (b_{0},...,b_{m-1})
 * where b_{i} is the evaluation of L_{i,S}(z) at z = t
 * and S is a specially-chosen subset of F having size m.
 * (Roughly, S is chosen to be a set of roots of unity, or some closely-related set.)
*/
    virtual std::vector<FieldT> lagrange_coeffs(const FieldT &t) = 0;

    virtual FieldT get_element(const size_t idx) = 0;
    virtual FieldT compute_Z(const FieldT &t) = 0;
    virtual void add_poly_Z(const FieldT &coeff, std::vector<FieldT> &H) = 0;
    virtual void divide_by_Z_on_coset(std::vector<FieldT> &P) = 0;
};

/**
 * Returns an evaluation domain object with the size m of a set S with
 * such that |S| >= min_size. It chooses from select different implementations,
 * depending on the parameters.
 */
template<typename FieldT>
std::shared_ptr<evaluation_domain<FieldT> > get_evaluation_domain(const size_t min_size);

/**
 * Naive evaluation of a *single* Lagrange polynomial, used for testing purposes.
 * The inputs are:
 * - an integer m
 * - a domain S = (a_{0},...,a_{m-1}) of size m
 * - an element t
 * - and index idx in {0,...,m-1}
 * The output is the polynomial L_{idx,S}(z) evaluated at z = t.
 */
template<typename FieldT>
FieldT lagrange_eval(const size_t m, const std::vector<FieldT> domain, const FieldT &t, const size_t idx);

} // libsnark
#include "qap/evaluation_domain.tcc"

#endif // EVALUATION_DOMAIN_HPP_
