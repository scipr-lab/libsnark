/** @file
 *****************************************************************************

 Implementation of serialization routines.

 See serialization.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef SERIALIZATION_TCC_
#define SERIALIZATION_TCC_

#include <cassert>
#include <sstream>

namespace libsnark {

inline void consume_newline(std::istream &in)
{
    char c;
    in.read(&c, 1);
}

inline void consume_OUTPUT_NEWLINE(std::istream &in)
{
#ifdef BINARY_OUTPUT
    // nothing to consume
#else
    char c;
    in.read(&c, 1);
#endif
}

inline void consume_OUTPUT_SEPARATOR(std::istream &in)
{
#ifdef BINARY_OUTPUT
    // nothing to consume
#else
    char c;
    in.read(&c, 1);
#endif
}

inline void output_bool(std::ostream &out, const bool b)
{
    out << (b ? 1 : 0) << "\n";
}

inline void input_bool(std::istream &in, bool &b)
{
    size_t tmp;
    in >> tmp;
    consume_newline(in);
    assert(tmp == 0 || tmp == 1);

    b = (tmp == 1 ? true : false);
}

template<typename T>
T reserialize(const T &obj)
{
    std::stringstream ss;
    ss << obj;
    T tmp;
    ss >> tmp;
    assert(obj == tmp);
    return tmp;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T> &v)
{
    out << v.size() << "\n";
    for (const T& t : v)
    {
        out << t << OUTPUT_NEWLINE;
    }

    return out;
}

template<typename T>
std::istream& operator>>(std::istream& in, std::vector<T> &v)
{
    size_t size;
    in >> size;
    consume_newline(in);

    v.resize(0);
    for (size_t i = 0; i < size; ++i)
    {
        T elt;
        in >> elt;
        consume_OUTPUT_NEWLINE(in);
        v.push_back(elt);
    }

    return in;
}

}

#endif // SERIALIZATION_TCC_
