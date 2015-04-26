/** @file
 *****************************************************************************

 Implementation of interfaces for the Merkle tree check read.

 See merkle_tree_check_read_gadget.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MERKLE_TREE_CHECK_READ_GADGET_TCC_
#define MERKLE_TREE_CHECK_READ_GADGET_TCC_

namespace libsnark {

template<typename FieldT>
merkle_tree_check_read_gadget<FieldT>::merkle_tree_check_read_gadget(protoboard<FieldT> &pb,
                                                                     const size_t tree_depth,
                                                                     const pb_linear_combination_array<FieldT> &address_bits,
                                                                     const digest_variable<FieldT> &leaf,
                                                                     const digest_variable<FieldT> &root,
                                                                     const pb_linear_combination<FieldT> &read_successful,
                                                                     const std::string &annotation_prefix) :
gadget<FieldT>(pb, annotation_prefix),
    digest_size(CRH_with_bit_out_gadget<FieldT>::get_digest_len()),
    tree_depth(tree_depth),
    address_bits(address_bits),
    leaf(leaf),
    root(root),
    read_successful(read_successful)
{
    assert(tree_depth > 0);
    assert(tree_depth == address_bits.size());

    knapsack_CRH_with_bit_out_gadget<FieldT>::sample_randomness(2*digest_size);

    for (size_t i = 0; i < tree_depth; ++i)
    {
        internal_left.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " internal_left_%zu", i)));
        internal_right.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " internal_right_%zu", i)));
    }

    for (size_t i = 0; i < tree_depth-1; ++i)
    {
        internal_output.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " internal_output_%zu", i)));
    }

    computed_root.reset(new digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " computed_root")));

    for (size_t i = 0; i < tree_depth; ++i)
    {
        block_variable<FieldT> inp(pb, internal_left[i], internal_right[i], FMT(this->annotation_prefix, " inp_%zu", i));
        hasher_inputs.emplace_back(inp);
        hashers.emplace_back(CRH_with_bit_out_gadget<FieldT>(pb, 2*digest_size, inp, (i == 0 ? *computed_root : internal_output[i-1]),
                                                             FMT(this->annotation_prefix, " load_hashers_%zu", i)));
    }

    for (size_t i = 0; i < tree_depth; ++i)
    {
        propagators.emplace_back(digest_selector_gadget<FieldT>(pb, digest_size, i < tree_depth - 1 ? internal_output[i] : leaf,
                                                                address_bits[tree_depth-1-i], internal_left[i], internal_right[i],
                                                                FMT(this->annotation_prefix, " digest_selector_%zu", i)));
    }

    check_root.reset(new bit_vector_copy_gadget<FieldT>(pb, computed_root->bits, root.bits, read_successful, FieldT::capacity(), FMT(annotation_prefix, " check_root")));
}

template<typename FieldT>
void merkle_tree_check_read_gadget<FieldT>::generate_r1cs_constraints()
{
    /* enforce bitness of internal results and aux values */
    for (size_t i = 0; i < tree_depth; ++i)
    {
        internal_left[i].generate_r1cs_constraints();
        internal_right[i].generate_r1cs_constraints();
    }

    /* ensure correct hash computations */
    for (size_t i = 0; i < tree_depth; ++i)
    {
        hashers[i].generate_r1cs_constraints(false); // we check root outside and internal_left/internal_right above
    }

    /* ensure consistency of internal_left/internal_right with internal_output */
    for (size_t i = 0; i < tree_depth; ++i)
    {
        propagators[i].generate_r1cs_constraints();
    }

    check_root->generate_r1cs_constraints(false, false);
}

template<typename FieldT>
void merkle_tree_check_read_gadget<FieldT>::generate_r1cs_witness(const bit_vector &leaf_digest, const bit_vector &root_digest, const merkle_authentication_path &path)
{
    assert(path.size() == tree_depth);

    /* fill in the leaf, everything else will be filled by hashers/propagators */
    leaf.generate_r1cs_witness(leaf_digest);

    /* do the hash computations bottom-up */
    for (int i = tree_depth-1; i >= 0; --i)
    {
        /* fill the non-path node */
        if (path[i].computed_is_right)
        {
            this->pb.lc_val(address_bits[tree_depth-1-i]) = FieldT::one();
            internal_left[i].generate_r1cs_witness(path[i].aux_digest);
        }
        else
        {
            this->pb.lc_val(address_bits[tree_depth-1-i]) = FieldT::zero();
            internal_right[i].generate_r1cs_witness(path[i].aux_digest);
        }

        /* propagate previous input */
        propagators[i].generate_r1cs_witness();

        /* compute hash */
        hashers[i].generate_r1cs_witness();
    }

    check_root->generate_r1cs_witness();
}

template<typename FieldT>
size_t merkle_tree_check_read_gadget<FieldT>::root_size_in_bits()
{
    return CRH_with_bit_out_gadget<FieldT>::get_digest_len();
}

template<typename FieldT>
size_t merkle_tree_check_read_gadget<FieldT>::expected_constraints(const size_t tree_depth)
{
    const size_t hasher_constraints = tree_depth * CRH_with_bit_out_gadget<FieldT>::expected_constraints();
    const size_t propagator_constraints = tree_depth * CRH_with_bit_out_gadget<FieldT>::get_digest_len();
    const size_t aux_digest_constraints = tree_depth * CRH_with_bit_out_gadget<FieldT>::get_digest_len();
    const size_t check_root_constraints = 3 * div_ceil(CRH_with_bit_out_gadget<FieldT>::get_digest_len(), FieldT::capacity());
    return hasher_constraints + propagator_constraints + aux_digest_constraints + check_root_constraints;
}

template<typename FieldT>
void test_merkle_tree_check_read_gadget()
{
    /* prepare test */
    const size_t digest_len = CRH_with_bit_out_gadget<FieldT>::get_digest_len();
    knapsack_CRH_with_bit_out_gadget<FieldT>::sample_randomness(2*digest_len);

    const size_t tree_depth = 16;
    std::vector<merkle_authentication_node> path(tree_depth);

    bit_vector prev_hash(digest_len);
    std::generate(prev_hash.begin(), prev_hash.end(), [&]() { return std::rand() % 2; });
    bit_vector leaf = prev_hash;

    bit_vector address_bits;

    for (long level = tree_depth-1; level >= 0; --level)
    {
        const bool computed_is_right = (std::rand() % 2);
        address_bits.push_back(computed_is_right);
        bit_vector other(digest_len);
        std::generate(other.begin(), other.end(), [&]() { return std::rand() % 2; });

        bit_vector block = prev_hash;
        block.insert(computed_is_right ? block.begin() : block.end(), other.begin(), other.end());
        bit_vector h = CRH_with_bit_out_gadget<FieldT>::get_hash(block);

        path[level].computed_is_right = computed_is_right;
        path[level].aux_digest = other;

        prev_hash = h;
    }
    bit_vector root = prev_hash;

    /* execute test */
    protoboard<FieldT> pb;
    pb_variable_array<FieldT> address_bits_va;
    address_bits_va.allocate(pb, tree_depth, "address_bits");
    digest_variable<FieldT> leaf_digest(pb, digest_len, "input_block");
    digest_variable<FieldT> root_digest(pb, digest_len, "output_digest");
    merkle_tree_check_read_gadget<FieldT> ml(pb, tree_depth, address_bits_va, leaf_digest, root_digest, ONE, "ml");

    ml.generate_r1cs_constraints();
    ml.generate_r1cs_witness(leaf, root, path);

    address_bits_va.fill_with_bits(pb, address_bits);
    leaf_digest.generate_r1cs_witness(leaf);
    root_digest.generate_r1cs_witness(root);
    assert(pb.is_satisfied());

    const size_t num_constraints = pb.num_constraints();
    const size_t expected_constraints = merkle_tree_check_read_gadget<FieldT>::expected_constraints(tree_depth);

    assert(num_constraints == expected_constraints);
}

} // libsnark

#endif // MERKLE_TREE_CHECK_READ_GADGET_TCC_
