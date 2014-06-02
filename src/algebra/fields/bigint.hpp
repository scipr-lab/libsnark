/** @file
 ********************************************************************************
 Declaration of bigint wrapper class around GMP's MPZ long integers.
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef BIGINT_HPP_
#define BIGINT_HPP_
#include <cstddef>
#include <iostream>
#include <gmp.h>

namespace libsnark {

#ifdef BINARY_OUTPUT
#define OUTPUT_NEWLINE ""
#define OUTPUT_SEPARATOR ""
#else
#define OUTPUT_NEWLINE "\n"
#define OUTPUT_SEPARATOR " "
#endif

template<mp_size_t n> class bigint;
template<mp_size_t n> std::ostream& operator<<(std::ostream &, const bigint<n>&);
template<mp_size_t n> std::istream& operator>>(std::istream &, bigint<n>&);

/**
 * Wrapper class around GMP's MPZ long integers. It supports arithmetic operations,
 * serialization and randomization.
 *
 * @todo
 * The serialization is fragile. Shoud be rewritten using a standard, portable-format
 * library like boost::serialize.
 *
 * However, for now the following conventions are used within the code.
 *
 * All algebraic objects support either binary or decimal output using
 * the standard C++ stream operators (operator<<, operator>>).
 *
 * The binary mode is activated by defining a BINARY_OUTPUT
 * preprocessor macro (e.g. g++ -DBINARY_OUTPUT ...).
 *
 * Binary output assumes that the stream is to be binary read at its
 * current position so any white space should be consumed beforehand.
 *
 * Consecutive algebraic objects are separated by OUTPUT_NEWLINE and
 * within themselves (e.g. X and Y coordinates for field elements) with
 * OUTPUT_SEPARATOR (as defined below).
 *
 * Therefore to dump two integers, two Fp elements and another integer
 * one would:
 *
 * out << 3 << "\n";
 * out << 4 << "\n";
 * out << FieldT(56) << OUTPUT_NEWLINE;
 * out << FieldT(78) << OUTPUT_NEWLINE;
 * out << 9 << "\n";
 *
 * Then reading back it its reader's responsibility (!) to consume "\n"
 * after 4, but Fp::operator<< will correctly consume OUTPUT_NEWLINE.
 *
 * The reader should also consume "\n" after 9, so that another field
 * element can be properly chained. This is especially important for
 * binary output.
 *
 * The binary serialization of algebraic objects is currently *not*
 * portable between machines of different word sizes.
 */
template<mp_size_t n>
class bigint {
public:
    static const mp_size_t N = n;

    mp_limb_t data[n] = {0};

    bigint() = default;
    bigint(const unsigned long x); /// Initalize from a small integer
    bigint(const char* s); /// Initialize from a string containing an integer in decimal notation
    bigint(const mpz_t r); /// Initialize from MPZ element

    void print() const;
    void print_hex() const;
    bool operator==(const bigint<n>& other) const;
    bool operator!=(const bigint<n>& other) const;
    void clear();
    bool is_zero() const;
    size_t max_bits() const { return n * GMP_NUMB_BITS; }
    size_t num_bits() const;

    unsigned long as_ulong() const;
    void to_mpz(mpz_t r) const;
    bool test_bit(const std::size_t bitno) const;

    bigint& randomize();

    friend std::ostream& operator<< <n>(std::ostream &out, const bigint<n> &b);
    friend std::istream& operator>> <n>(std::istream &in, bigint<n> &b);
};

inline void consume_newline(std::istream &in);
inline void consume_OUTPUT_NEWLINE(std::istream &in);
inline void consume_OUTPUT_SEPARATOR(std::istream &in);

} // libsnark
#include "algebra/fields/bigint.tcc"
#endif
