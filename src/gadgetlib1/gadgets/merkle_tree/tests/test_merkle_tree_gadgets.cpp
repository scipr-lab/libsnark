/**
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "algebra/curves/bn128/bn128_pp.hpp"
#include "algebra/curves/edwards/edwards_pp.hpp"
#include "algebra/curves/mnt/mnt4/mnt4_pp.hpp"
#include "algebra/curves/mnt/mnt6/mnt6_pp.hpp"
#include "gadgetlib1/gadgets/merkle_tree/merkle_tree_check_read_gadget.hpp"
#include "gadgetlib1/gadgets/merkle_tree/merkle_tree_check_update_gadget.hpp"

using namespace libsnark;

int main(void)
{
    start_profiling();

    bn128_pp::init_public_params();
    test_merkle_tree_check_read_gadget<Fr<bn128_pp> >();
    test_merkle_tree_check_update_gadget<Fr<bn128_pp> >();

    edwards_pp::init_public_params();
    test_merkle_tree_check_read_gadget<Fr<edwards_pp> >();
    test_merkle_tree_check_update_gadget<Fr<edwards_pp> >();

    mnt4_pp::init_public_params();
    test_merkle_tree_check_read_gadget<Fr<mnt4_pp> >();
    test_merkle_tree_check_update_gadget<Fr<mnt4_pp> >();

    mnt6_pp::init_public_params();
    test_merkle_tree_check_read_gadget<Fr<mnt6_pp> >();
    test_merkle_tree_check_update_gadget<Fr<mnt6_pp> >();
}
