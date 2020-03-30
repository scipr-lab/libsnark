/** @file
 *****************************************************************************

 Implementation of auxiliary functions for fooram.

 See fooram_aux.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <libff/common/serialization.hpp>

#include <libsnark/relations/ram_computations/rams/fooram/fooram_aux.hpp>

namespace libsnark {

fooram_architecture_params::fooram_architecture_params(const size_t w) : w(w)
{
}

size_t fooram_architecture_params::num_addresses() const
{
    return 1ul<<w;
}

size_t fooram_architecture_params::address_size() const
{
    return w;
}

size_t fooram_architecture_params::value_size() const
{
    return w;
}

size_t fooram_architecture_params::cpu_state_size() const
{
    return w;
}

size_t fooram_architecture_params::initial_pc_addr() const
{
    return 0;
}

memory_contents fooram_architecture_params::initial_memory_contents(const fooram_program &program,
                                                                    const fooram_input_tape &primary_input) const
{
    memory_contents m;
    /* fooram memory contents do not depend on program/input. */
    libff::UNUSED(program, primary_input);
    return m;
}

libff::bit_vector fooram_architecture_params::initial_cpu_state(const fooram_input_tape &primary_input) const
{
    libff::UNUSED(primary_input);
    libff::bit_vector state;
    state.resize(w, false);
    return state;
}

fooram_input_tape fooram_architecture_params::primary_input_from_boot_trace(const memory_store_trace &boot_trace) const
{
    /* fooram memory contents do not depend on program/input. */
    libff::UNUSED(boot_trace);
    fooram_input_tape tape;
    return tape;
}


void fooram_architecture_params::print() const
{
    printf("w = %zu\n", w);
}

bool fooram_architecture_params::operator==(const fooram_architecture_params &other) const
{
    return (this->w == other.w);
}

std::ostream& operator<<(std::ostream &out, const fooram_architecture_params &ap)
{
    out << ap.w << "\n";
    return out;
}

std::istream& operator>>(std::istream &in, fooram_architecture_params &ap)
{
    in >> ap.w;
    libff::consume_newline(in);
    return in;
}

} // libsnark
