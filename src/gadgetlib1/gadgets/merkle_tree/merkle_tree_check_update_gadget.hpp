/** @file
 *****************************************************************************

 Declaration of interfaces for the Merkle tree check read gadget.

 The gadget checks the following: given two roots R1 and R2, address A, two
 values V1 and V2, and authentication path P, check that
 - P is a valid authentication path for the value V1 as the A-th leaf in a Merkle tree with root R1, and
 - P is a valid authentication path for the value V2 as the A-th leaf in a Merkle tree with root R2.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MERKLE_TREE_CHECK_UPDATE_GADGET_HPP_
#define MERKLE_TREE_CHECK_UPDATE_GADGET_HPP_

#include "gadgetlib1/gadgets/hashes/crh_gadget.hpp"
#include "gadgetlib1/gadgets/hashes/hash_io.hpp"
#include "gadgetlib1/gadgets/hashes/digest_selector_gadget.hpp"

namespace libsnark {

template<typename FieldT>
class merkle_tree_check_update_gadget : public gadget<FieldT> {
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

    std::shared_ptr<digest_variable<FieldT> > computed_next_root;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > check_next_root;

public:

    const size_t digest_size;
    const size_t tree_depth;

    pb_variable_array<FieldT> addr_bits;
    digest_variable<FieldT> prev_leaf_digest;
    digest_variable<FieldT> prev_root_digest;
    digest_variable<FieldT> next_leaf_digest;
    digest_variable<FieldT> next_root_digest;
    pb_linear_combination<FieldT> update_successful;

    merkle_tree_check_update_gadget(protoboard<FieldT> &pb,
                                    const size_t tree_depth,
                                    const pb_variable_array<FieldT> &addr_bits,
                                    const digest_variable<FieldT> &prev_leaf_digest,
                                    const digest_variable<FieldT> &prev_root_digest,
                                    const digest_variable<FieldT> &next_leaf_digest,
                                    const digest_variable<FieldT> &next_root_digest,
                                    const pb_linear_combination<FieldT> &update_successful,
                                    const std::string &annotation_prefix);

    void generate_r1cs_constraints();

    void generate_r1cs_witness(const bit_vector &prev_leaf, const bit_vector &prev_root, const merkle_authentication_path &prev_path, const bit_vector &next_leaf);

    static size_t root_size_in_bits();
    /* for debugging purposes */
    static size_t expected_constraints(const size_t tree_depth);
};

template<typename FieldT>
void test_merkle_tree_check_update_gadget();

} // libsnark

#include "gadgetlib1/gadgets/merkle_tree/merkle_tree_check_update_gadget.tcc"

#endif // MERKLE_TREE_CHECK_UPDATE_GADGET_HPP_
