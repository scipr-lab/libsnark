/** @file
 ********************************************************************************
 Declaration of misc math and serialization utility functions
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace libsnark {

typedef std::vector<bool> bit_vector;

/// returns ceil(log2(n)), so 1<<log2(n) is the smallest power of 2, that is not less than n
size_t log2(size_t n);

size_t to_twos_complement(int i, size_t w);
int from_twos_complement(size_t i, size_t w);

size_t bitreverse(size_t n, const size_t l);
bit_vector int_list_to_bits(const std::initializer_list<unsigned long> &l, const size_t wordsize);
long long div_ceil(long long x, long long y);

bool is_little_endian();

typedef std::vector<size_t> permutation;

std::string FORMAT(const std::string &prefix, const char* format, ...);

#ifdef DEBUG
#define FMT FORMAT
#else
#define FMT(...) ""
#endif

void serialize_bit_vector(std::ostream &out, const bit_vector &v);
void deserialize_bit_vector(std::istream &in, bit_vector &v);

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

} // libsnark
#endif // UTILS_HPP_
