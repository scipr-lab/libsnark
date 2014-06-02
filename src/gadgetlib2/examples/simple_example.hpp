/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef SIMPLE_EXAMPLE_HPP_
#define SIMPLE_EXAMPLE_HPP_

#include "common/types.hpp"
#include "r1cs_ppzksnark/examples/r1cs_examples.hpp"

namespace libsnark {

r1cs_example<Fr<default_pp> > gen_r1cs_example_from_gadgetlib2_protoboard(const size_t size);

} // libsnark

#endif // SIMPLE_EXAMPLE_HPP_
