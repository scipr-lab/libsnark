/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef INTEGRATION_HPP_
#define INTEGRATION_HPP_

#include "common/types.hpp"
#include "r1cs/r1cs.hpp"
#include "gadgetlib2/protoboard.hpp"

namespace libsnark {

r1cs_constraint_system<Fr<default_pp> > get_constraint_system_from_gadgetlib2(const gadgetlib2::Protoboard &pb);
r1cs_variable_assignment<Fr<default_pp> > get_variable_assignment_from_gadgetlib2(const gadgetlib2::Protoboard &pb);

} // libsnark

#endif // INTEGRATION_HPP_
