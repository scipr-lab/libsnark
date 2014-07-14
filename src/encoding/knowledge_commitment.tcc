/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef KNOWLEDGE_COMMITMENT_TCC_
#define KNOWLEDGE_COMMITMENT_TCC_

#include "encoding/knowledge_commitment.hpp"
#include <algorithm>

namespace libsnark {

template<typename T1, typename T2>
knowledge_commitment<T1,T2>::knowledge_commitment(const T1 &g, const T2 &h) :
    g(g), h(h)
{
}

template<typename T1, typename T2>
knowledge_commitment<T1,T2> knowledge_commitment<T1,T2>::operator+(const knowledge_commitment<T1,T2> &other) const
{
    return knowledge_commitment<T1,T2>(this->g + other.g,
                                       this->h + other.h);
}

template<typename T1, typename T2>
bool knowledge_commitment<T1,T2>::operator==(const knowledge_commitment<T1,T2> &other) const
{
    return (this->g == other.g &&
            this->h == other.h);
}

template<typename T1, typename T2, mp_size_t m>
knowledge_commitment<T1,T2> operator*(const bigint<m> &lhs, const knowledge_commitment<T1,T2> &rhs)
{
    return knowledge_commitment<T1,T2>(lhs * rhs.g,
                                       lhs * rhs.h);
}

template<typename T1, typename T2, mp_size_t m, const bigint<m> &modulus_p>
knowledge_commitment<T1,T2> operator*(const Fp_model<m, modulus_p> &lhs, const knowledge_commitment<T1,T2> &rhs)
{
    return (lhs.as_bigint()) * rhs;
}

template<typename T1, typename T2>
knowledge_commitment<T1, T2> knowledge_commitment_vector<T1,T2>::get_value(const size_t idx) const
{
    if (!is_sparse)
    {
        return values[idx];
    }
    else
    {
        auto it = std::lower_bound(indices.begin(), indices.end(), idx);
        return (it != indices.end() && *it == idx) ? values[it - indices.begin()] : knowledge_commitment<T1,T2>(T1(),T2());
    }
}

template<typename T1, typename T2>
bool knowledge_commitment_vector<T1,T2>::operator==(const knowledge_commitment_vector<T1,T2> &other) const
{
    return (this->values == other.values &&
            this->indices == other.indices &&
            this->is_sparse == other.is_sparse);
}

template<typename T1,typename T2>
std::ostream& operator<<(std::ostream& out, const knowledge_commitment<T1,T2> &kc)
{
    out << kc.g << OUTPUT_SEPARATOR << kc.h;
    return out;
}

template<typename T1,typename T2>
std::istream& operator>>(std::istream& in, knowledge_commitment<T1,T2> &kc)
{
    in >> kc.g;
    consume_OUTPUT_SEPARATOR(in);
    in >> kc.h;
    return in;
}

template<typename T1,typename T2>
std::ostream& operator<<(std::ostream& out, const knowledge_commitment_vector<T1,T2> &v)
{
    out << (v.is_sparse ? 1 : 0) << "\n";
    out << v.original_size << "\n";
    out << v.indices.size() << "\n";
    for (const size_t& i : v.indices)
    {
        out << i << "\n";
    }

    out << v.values.size() << "\n";
    for (const knowledge_commitment<T1,T2>& t : v.values)
    {
        out << t << OUTPUT_NEWLINE;
    }

    return out;
}

template<typename T1,typename T2>
std::istream& operator>>(std::istream& in, knowledge_commitment_vector<T1,T2> &v)
{
    int i;
    in >> i;
    v.is_sparse = (i == 1);

    in >> v.original_size;
    consume_newline(in);

    size_t s;
    in >> s;
    v.indices.resize(s);
    for (size_t i = 0; i < s; ++i)
    {
        in >> v.indices[i];
        consume_newline(in);
    }

    v.values.clear();
    in >> s;
    consume_newline(in);
    v.values.reserve(s);

    for (size_t i = 0; i < s; ++i)
    {
        T1 t1;
        T2 t2;
        in >> t1;
        consume_OUTPUT_SEPARATOR(in);
        in >> t2;
        consume_OUTPUT_NEWLINE(in);
        v.values.emplace_back(knowledge_commitment<T1,T2>(t1,t2));
    }

    return in;
}

} // libsnark
#endif // KNOWLEDGE_COMMITMENT_TCC_
