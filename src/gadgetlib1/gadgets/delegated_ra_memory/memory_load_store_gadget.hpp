/** @file
 *****************************************************************************

 Declaration of interfaces for the memory load&store gadget.

 The gadget checks the following: given two roots R1 and R2, address A, two
 values V1 and V2, and authentication path P, check that
 - P is a valid authentication path for the value V1 as the A-th leaf in a Merkle tree with root R1, and
 - P is a valid authentication path for the value V2 as the A-th leaf in a Merkle tree with root R2.

 The gadget can be used to verify a memory load, followed by a store to the
 same address, from a "delegated memory".

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MEMORY_LOAD_STORE_GADGET_HPP_
#define MEMORY_LOAD_STORE_GADGET_HPP_

#include "gadgetlib1/gadgets/hashes/crh_gadget.hpp"
#include "gadgetlib1/gadgets/hashes/hash_io.hpp"
#include "gadgetlib1/gadgets/hashes/digest_selector_gadget.hpp"

namespace libsnark {

template<typename FieldT>
class memory_load_store_gadget : public gadget<FieldT> {
private:

    std::vector<CRH_with_bit_out_gadget<FieldT> > prev_hashers;
    std::vector<block_variable<FieldT> > prev_hasher_inputs;
    std::vector<digest_selector_gadget<FieldT> > prev_propagators;
    std::vector<digest_variable<FieldT> > prev_internal_left;
    std::vector<digest_variable<FieldT> > prev_internal_right;
    std::vector<digest_variable<FieldT> > prev_internal_output;

    std::vector<CRH_with_bit_out_gadget<FieldT> > next_hashers;
    std::vector<block_variable<FieldT> > next_hasher_inputs;
    std::vector<digest_selector_gadget<FieldT> > next_propagators;
    std::vector<digest_variable<FieldT> > next_internal_left;
    std::vector<digest_variable<FieldT> > next_internal_right;
    std::vector<digest_variable<FieldT> > next_internal_output;

public:

    const size_t digest_size;
    const size_t tree_depth;

    pb_variable_array<FieldT> addr_bits;
    digest_variable<FieldT> prev_leaf_digest;
    digest_variable<FieldT> prev_root_digest;
    digest_variable<FieldT> next_leaf_digest;
    digest_variable<FieldT> next_root_digest;

    memory_load_store_gadget(protoboard<FieldT> &pb,
                             const size_t tree_depth,
                             const pb_variable_array<FieldT> &addr_bits,
                             const digest_variable<FieldT> &prev_leaf_digest,
                             const digest_variable<FieldT> &prev_root_digest,
                             const digest_variable<FieldT> &next_leaf_digest,
                             const digest_variable<FieldT> &next_root_digest,
                             const std::string &annotation_prefix);

    void generate_r1cs_constraints();

    void generate_r1cs_witness(const bit_vector &prev_leaf, const bit_vector &prev_root, const merkle_authentication_path &prev_path, const bit_vector &next_leaf);

    /* for debugging purposes */
    static size_t expected_constraints(const size_t tree_depth);
};

template<typename FieldT>
void test_memory_load_store_gadget();

} // libsnark

#include "gadgetlib1/gadgets/delegated_ra_memory/memory_load_store_gadget.tcc"

#endif // MEMORY_LOAD_STORE_GADGET_HPP_
