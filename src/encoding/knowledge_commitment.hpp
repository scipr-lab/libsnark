/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef KNOWLEDGE_COMMITMENT_HPP_
#define KNOWLEDGE_COMMITMENT_HPP_

#include <vector>

#include "algebra/curves/public_params.hpp"
#include "algebra/fields/fp.hpp"

namespace libsnark {

/*
 Interfaces for knowledge commitments.
*/

template<typename T1, typename T2>
struct knowledge_commitment {
    T1 g;
    T2 h;

    knowledge_commitment<T1,T2>() = default;
    knowledge_commitment<T1,T2>& operator=(const knowledge_commitment<T1,T2> &other) = default;
    knowledge_commitment<T1,T2>& operator=(knowledge_commitment<T1,T2> &&other) = default;
    knowledge_commitment<T1,T2>(const knowledge_commitment<T1,T2> &other) = default;
    knowledge_commitment<T1,T2>(knowledge_commitment<T1,T2> &&other) = default;
    knowledge_commitment<T1,T2>(const T1 &g, const T2 &h);
    knowledge_commitment<T1,T2> operator+(const knowledge_commitment<T1, T2> &other) const;

    bool operator==(const knowledge_commitment<T1,T2> &other) const;
};

template<typename T1, typename T2, mp_size_t m>
knowledge_commitment<T1,T2> operator*(const bigint<m> &lhs, const knowledge_commitment<T1,T2> &rhs);

template<typename T1, typename T2, mp_size_t m, const bigint<m> &modulus_p>
knowledge_commitment<T1,T2> operator*(const Fp_model<m, modulus_p> &lhs, const knowledge_commitment<T1,T2> &rhs);

template<typename ppT>
using G1G1_knowledge_commitment = knowledge_commitment<G1<ppT>, G1<ppT> >;

template<typename ppT>
using G2G1_knowledge_commitment = knowledge_commitment<G2<ppT>, G1<ppT> >;

template<typename T1, typename T2>
struct knowledge_commitment_vector {
public:
    std::vector<knowledge_commitment<T1, T2> > values;
    std::vector<size_t> indices;
    bool is_sparse;
    size_t original_size;

    knowledge_commitment<T1, T2> get_value(const size_t idx) const;

    knowledge_commitment_vector<T1,T2>() = default;
    knowledge_commitment_vector<T1,T2>& operator=(const knowledge_commitment_vector<T1,T2> &other) = default;
    knowledge_commitment_vector<T1,T2>& operator=(knowledge_commitment_vector<T1,T2> &&other) = default;
    knowledge_commitment_vector<T1,T2>(const knowledge_commitment_vector<T1,T2> &other) = default;
    knowledge_commitment_vector<T1,T2>(knowledge_commitment_vector<T1,T2> &&other) = default;

    size_t sparse_size() const { return values.size(); }

    size_t size_in_bits() const {
        if (is_sparse)
        {
            return values.size() * (T1::size_in_bits() + T2::size_in_bits() + 8 * sizeof(size_t)) + 8 * sizeof(size_t);
        }
        else
        {
            return values.size() * (T1::size_in_bits() + T2::size_in_bits()) + 8 * sizeof(size_t);
        }
    }

    bool operator==(const knowledge_commitment_vector<T1,T2> &other) const;
};

template<typename ppT>
using G1G1_knowledge_commitment_vector = knowledge_commitment_vector<G1<ppT>, G1<ppT> >;

template<typename ppT>
using G2G1_knowledge_commitment_vector = knowledge_commitment_vector<G2<ppT>, G1<ppT> >;

template<typename T1,typename T2>
std::ostream& operator<<(std::ostream& out, const knowledge_commitment<T1,T2> &kc);

template<typename T1,typename T2>
std::istream& operator>>(std::istream& in, knowledge_commitment<T1,T2> &kc);

template<typename T1,typename T2>
std::ostream& operator<<(std::ostream& out, const knowledge_commitment_vector<T1,T2> &v);

template<typename T1,typename T2>
std::istream& operator>>(std::istream& in, knowledge_commitment_vector<T1,T2> &v);

} // libsnark
#include "encoding/knowledge_commitment.tcc"

#endif // KNOWLEDGE_COMMITMENT_HPP_
