/** @file
 *****************************************************************************
 Implementation of PublicParams for Fp field arithmetic
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

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
