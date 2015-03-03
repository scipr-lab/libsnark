/** @file
 *****************************************************************************

 Implementation of interfaces for the memory load&store gadget.

 See memory_load_store_gadget.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MEMORY_LOAD_STORE_GADGET_TCC_
#define MEMORY_LOAD_STORE_GADGET_TCC_

namespace libsnark {

template<typename FieldT>
memory_load_store_gadget<FieldT>::memory_load_store_gadget(protoboard<FieldT> &pb,
                                                           const size_t tree_depth,
                                                           const pb_variable_array<FieldT> &addr_bits,
                                                           const digest_variable<FieldT> &prev_leaf_digest,
                                                           const digest_variable<FieldT> &prev_root_digest,
                                                           const digest_variable<FieldT> &next_leaf_digest,
                                                           const digest_variable<FieldT> &next_root_digest,
                                                           const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix),
    digest_size(knapsack_CRH_with_bit_out_gadget<FieldT>::get_digest_len()),
    tree_depth(tree_depth),
    addr_bits(addr_bits),
    prev_leaf_digest(prev_leaf_digest),
    prev_root_digest(prev_root_digest),
    next_leaf_digest(next_leaf_digest),
    next_root_digest(next_root_digest)
{
    assert(tree_depth > 0);
    assert(tree_depth == addr_bits.size());

    knapsack_CRH_with_bit_out_gadget<FieldT>::sample_randomness(2*digest_size);

    for (size_t i = 0; i < tree_depth; ++i)
    {
        prev_internal_left.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " prev_internal_left_%zu", i)));
        prev_internal_right.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " prev_internal_right_%zu", i)));

        next_internal_left.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " next_internal_left_%zu", i)));
        next_internal_right.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " next_internal_right_%zu", i)));
    }

    for (size_t i = 0; i < tree_depth-1; ++i)
    {
        prev_internal_output.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " prev_internal_output_%zu", i)));

        next_internal_output.emplace_back(digest_variable<FieldT>(pb, digest_size, FMT(this->annotation_prefix, " next_internal_output_%zu", i)));
    }

    for (size_t i = 0; i < tree_depth; ++i)
    {
        block_variable<FieldT> prev_inp(pb, prev_internal_left[i], prev_internal_right[i], FMT(this->annotation_prefix, " prev_inp_%zu", i));
        prev_hasher_inputs.emplace_back(prev_inp);
        prev_hashers.emplace_back(CRH_with_bit_out_gadget<FieldT>(pb, 2*digest_size, prev_inp, (i == 0 ? prev_root_digest : prev_internal_output[i-1]),
                                                     FMT(this->annotation_prefix, " prev_hashers_%zu", i)));

        block_variable<FieldT> next_inp(pb, next_internal_left[i], next_internal_right[i], FMT(this->annotation_prefix, " next_inp_%zu", i));
        next_hasher_inputs.emplace_back(next_inp);
        next_hashers.emplace_back(CRH_with_bit_out_gadget<FieldT>(pb, 2*digest_size, next_inp, (i == 0 ? next_root_digest : next_internal_output[i-1]),
                                                     FMT(this->annotation_prefix, " next_hashers_%zu", i)));
    }

    for (size_t i = 0; i < tree_depth; ++i)
    {
        prev_propagators.emplace_back(digest_selector_gadget<FieldT>(pb, digest_size, i < tree_depth -1 ? prev_internal_output[i] : prev_leaf_digest,
                                                                       addr_bits[tree_depth-1-i], prev_internal_left[i], prev_internal_right[i],
                                                                       FMT(this->annotation_prefix, " prev_propagators_%zu", i)));
        next_propagators.emplace_back(digest_selector_gadget<FieldT>(pb, digest_size, i < tree_depth -1 ? next_internal_output[i] : next_leaf_digest,
                                                                       addr_bits[tree_depth-1-i], next_internal_left[i], next_internal_right[i],
                                                                       FMT(this->annotation_prefix, " next_propagators_%zu", i)));
    }
}

template<typename FieldT>
void memory_load_store_gadget<FieldT>::generate_r1cs_constraints()
{
    /* ensure bitness of authentication path plus all prev computed values */
    for (size_t i = 0; i < tree_depth; ++i)
    {
        prev_internal_left[i].generate_r1cs_constraints();
        prev_internal_right[i].generate_r1cs_constraints();
    }

    /* ensure correct hash computations */
    for (size_t i = 0; i < tree_depth; ++i)
    {
        prev_hashers[i].generate_r1cs_constraints(false); // we check root outside and prev_left/prev_right above
        next_hashers[i].generate_r1cs_constraints(true); // however we must check right side hashes
    }

    /* ensure consistency of internal_left/internal_right with internal_output */
    for (size_t i = 0; i < tree_depth; ++i)
    {
        prev_propagators[i].generate_r1cs_constraints();
        next_propagators[i].generate_r1cs_constraints();
    }

    /* ensure that left auxiliary input and right auxiliary input match */
    // this, by implication, takes care of bitness of right auxiliary input
    for (size_t i = 0; i < tree_depth; ++i)
    {
        for (size_t j = 0; j < digest_size; ++j)
        {
            /*
              addr * (prev_left - next_left) + (1 - addr) * (prev_right - next_right) = 0
              addr * (prev_left - next_left - prev_right + next_right) = next_right - prev_right
            */
            this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(addr_bits[tree_depth-1-i],
                                                                 prev_internal_left[i].bits[j] - next_internal_left[i].bits[j] - prev_internal_right[i].bits[j] + next_internal_right[i].bits[j],
                                                                 next_internal_right[i].bits[j] - prev_internal_right[i].bits[j]),
                                         FMT(this->annotation_prefix, " aux_check_%zu_%zu", i, j));
        }
    }
}

template<typename FieldT>
void memory_load_store_gadget<FieldT>::generate_r1cs_witness(const bit_vector &prev_leaf, const bit_vector &prev_root, const merkle_authentication_path &prev_path, const bit_vector &next_leaf)
{
    /* fill in the leafs, everything else will be filled by hashers/propagators */
    prev_leaf_digest.fill_with_bits(prev_leaf);
    next_leaf_digest.fill_with_bits(next_leaf);

    /* do the hash computations bottom-up */
    for (int i = tree_depth-1; i >= 0; --i)
    {
        /* fill the non-path node */
        if (prev_path[i].computed_is_right)
        {
            this->pb.val(addr_bits[tree_depth-1-i]) = FieldT::one();
            prev_internal_left[i].fill_with_bits(prev_path[i].aux_digest);
            next_internal_left[i].fill_with_bits(prev_path[i].aux_digest);
        }
        else
        {
            this->pb.val(addr_bits[tree_depth-1-i]) = FieldT::zero();
            prev_internal_right[i].fill_with_bits(prev_path[i].aux_digest);
            next_internal_right[i].fill_with_bits(prev_path[i].aux_digest);
        }

        /* propagate previous input */
        prev_propagators[i].generate_r1cs_witness();
        next_propagators[i].generate_r1cs_witness();

        /* compute hash */
        prev_hashers[i].generate_r1cs_witness(prev_hasher_inputs[i].bits.get_bits(this->pb));
        next_hashers[i].generate_r1cs_witness(next_hasher_inputs[i].bits.get_bits(this->pb));
    }

    prev_root_digest.fill_with_bits(prev_root);
}

template<typename FieldT>
size_t memory_load_store_gadget<FieldT>::expected_constraints(const size_t tree_depth)
{
    const size_t digest_size = CRH_with_bit_out_gadget<FieldT>::get_digest_len();

    const size_t hasher_constraints = 2 * tree_depth * CRH_with_bit_out_gadget<FieldT>::expected_constraints();
    const size_t propagator_constraints = 2 * tree_depth * digest_size;
    const size_t aux_digest_constraints = tree_depth * digest_size;
    const size_t aux_equality_constraints = tree_depth * digest_size;
    return hasher_constraints + propagator_constraints + aux_digest_constraints + aux_equality_constraints;
}

template<typename FieldT>
void test_memory_load_store_gadget()
{
    /* prepare test */
    const size_t digest_len = CRH_with_bit_out_gadget<FieldT>::get_digest_len();
    knapsack_CRH_with_bit_out_gadget<FieldT>::sample_randomness(2*digest_len);

    const size_t tree_depth = 16;
    std::vector<merkle_authentication_node> prev_path(tree_depth);

    bit_vector prev_load_hash(digest_len);
    std::generate(prev_load_hash.begin(), prev_load_hash.end(), [&]() { return std::rand() % 2; });
    bit_vector prev_store_hash(digest_len);
    std::generate(prev_store_hash.begin(), prev_store_hash.end(), [&]() { return std::rand() % 2; });

    bit_vector loaded_leaf = prev_load_hash;
    bit_vector stored_leaf = prev_store_hash;

    bit_vector addr_bits;

    for (long level = tree_depth-1; level >= 0; --level)
    {
        const bool computed_is_right = (std::rand() % 2);
        addr_bits.push_back(computed_is_right);
        bit_vector other(digest_len);
        std::generate(other.begin(), other.end(), [&]() { return std::rand() % 2; });

        bit_vector load_block = prev_load_hash;
        load_block.insert(computed_is_right ? load_block.begin() : load_block.end(), other.begin(), other.end());
        bit_vector store_block = prev_store_hash;
        store_block.insert(computed_is_right ? store_block.begin() : store_block.end(), other.begin(), other.end());

        bit_vector load_h = CRH_with_bit_out_gadget<FieldT>::get_hash(load_block);
        bit_vector store_h = CRH_with_bit_out_gadget<FieldT>::get_hash(store_block);

        prev_path[level].computed_is_right = computed_is_right;
        prev_path[level].aux_digest = other;

        prev_load_hash = load_h;
        prev_store_hash = store_h;
    }

    bit_vector load_root = prev_load_hash;
    bit_vector store_root = prev_store_hash;

    /* execute the test */
    protoboard<FieldT> pb;
    pb_variable_array<FieldT> addr_bits_va;
    addr_bits_va.allocate(pb, tree_depth, "addr_bits");
    digest_variable<FieldT> prev_leaf_digest(pb, digest_len, "prev_leaf_digest");
    digest_variable<FieldT> prev_root_digest(pb, digest_len, "prev_root_digest");
    digest_variable<FieldT> next_leaf_digest(pb, digest_len, "next_leaf_digest");
    digest_variable<FieldT> next_root_digest(pb, digest_len, "next_root_digest");
    memory_load_store_gadget<FieldT> mls(pb, tree_depth, addr_bits_va, prev_leaf_digest, prev_root_digest, next_leaf_digest, next_root_digest, "mls");
    mls.generate_r1cs_constraints();
    mls.generate_r1cs_witness(loaded_leaf, load_root, prev_path, stored_leaf);
    prev_leaf_digest.fill_with_bits(loaded_leaf);
    prev_root_digest.fill_with_bits(load_root);

    next_leaf_digest.fill_with_bits(stored_leaf);
    next_root_digest.fill_with_bits(store_root);
    mls.addr_bits.fill_with_bits(pb, addr_bits);
    assert(pb.is_satisfied());

    const size_t num_constraints = pb.num_constraints();
    const size_t expected_constraints = memory_load_store_gadget<FieldT>::expected_constraints(tree_depth);
    assert(num_constraints == expected_constraints);
}

} // libsnark

#endif // MEMORY_LOAD_STORE_GADGET_TCC_
