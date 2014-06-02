/** @file
 *******************************************************************************
 Implementation of PublicParams for Fp field arithmetic
 *******************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#include <cassert>
#include <vector>
#include "pp.hpp"

namespace gadgetlib2 {

PublicParams::PublicParams(const std::size_t log_p) : log_p(log_p) {}

Fp PublicParams::getFp(long x) const {
    return Fp(x);
}

PublicParams::~PublicParams() {}

PublicParams initPublicParamsFromDefaultPp() {
    libsnark::init_public_params<libsnark::default_pp>();
    const std::size_t log_p = libsnark::Fr<libsnark::default_pp>::num_bits;
    return PublicParams(log_p);
}

} // namespace gadgetlib2
