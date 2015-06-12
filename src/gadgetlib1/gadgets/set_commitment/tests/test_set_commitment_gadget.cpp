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
#include "gadgetlib1/gadgets/set_commitment/set_commitment_gadget.hpp"
#include "gadgetlib1/gadgets/hashes/sha256/sha256_gadget.hpp"

using namespace libsnark;

template<typename ppT>
void test_all_set_commitment_gadgets()
{
    typedef Fr<ppT> FieldT;
    test_set_commitment_gadget<FieldT, CRH_with_bit_out_gadget<FieldT> >();
}

int main(void)
{
    start_profiling();

    bn128_pp::init_public_params();
    test_all_set_commitment_gadgets<bn128_pp>();

    edwards_pp::init_public_params();
    test_all_set_commitment_gadgets<edwards_pp>();

    mnt4_pp::init_public_params();
    test_all_set_commitment_gadgets<mnt4_pp>();

    mnt6_pp::init_public_params();
    test_all_set_commitment_gadgets<mnt6_pp>();
}
