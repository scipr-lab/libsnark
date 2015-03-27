/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/
#ifndef SET_COMMITMENT_GADGET_TCC_
#define SET_COMMITMENT_GADGET_TCC_

#include "common/data_structures/set_commitment.hpp"

namespace libsnark {

template<typename FieldT>
using set_commitment_variable = digest_variable<FieldT>;

template<typename FieldT>
set_commitment_gadget<FieldT>::set_commitment_gadget(protoboard<FieldT> &pb,
                                                     const size_t max_entries,
                                                     const pb_variable_array<FieldT> &element_bits,
                                                     const set_commitment_variable<FieldT> &root_digest,
                                                     const pb_linear_combination<FieldT> &check_successful,
                                                     const std::string &annotation_prefix) :
gadget<FieldT>(pb, annotation_prefix), tree_depth(log2(max_entries)), element_bits(element_bits),
    root_digest(root_digest), check_successful(check_successful)
{
    element_block.reset(new block_variable<FieldT>(pb, { element_bits }, FMT(annotation_prefix, " element_block")));
    element_digest.reset(new digest_variable<FieldT>(pb, CRH_with_bit_out_gadget<FieldT>::get_digest_len(),
                                                     FMT(annotation_prefix, " element_digest")));
    hash_element.reset(new CRH_with_bit_out_gadget<FieldT>(pb, element_bits.size(), *element_block, *element_digest, FMT(annotation_prefix, " hash_element")));
    address_bits.allocate(pb, tree_depth, FMT(annotation_prefix, " address_bits"));
    check_membership.reset(new merkle_tree_check_read_gadget<FieldT>(pb,
                                                                     tree_depth,
                                                                     address_bits,
                                                                     *element_digest,
                                                                     root_digest,
                                                                     check_successful,
                                                                     FMT(annotation_prefix, " check_membership")));
}

template<typename FieldT>
void set_commitment_gadget<FieldT>::generate_r1cs_constraints()
{
    hash_element->generate_r1cs_constraints();
    check_membership->generate_r1cs_constraints();
}

template<typename FieldT>
void set_commitment_gadget<FieldT>::generate_r1cs_witness(const merkle_authentication_path &path)
{
    hash_element->generate_r1cs_witness();
    for (size_t i = 0; i < tree_depth; ++i)
    {
        this->pb.val(address_bits[i]) = path[i].computed_is_right ? FieldT::one() : FieldT::zero();
        this->pb.val(address_bits[i]).print();
    }

    check_membership->generate_r1cs_witness(element_digest->bits.get_bits(this->pb),
                                            root_digest.bits.get_bits(this->pb), path);
}

template<typename FieldT>
size_t set_commitment_gadget<FieldT>::root_size_in_bits()
{
    return merkle_tree_check_read_gadget<FieldT>::root_size_in_bits();
}

template<typename FieldT>
void test_set_commitment_gadget()
{
    const size_t digest_len = CRH_with_bit_out_gadget<FieldT>::get_digest_len();
    knapsack_CRH_with_bit_out_gadget<FieldT>::sample_randomness(2*digest_len);

    const size_t max_set_size = 16;
    const size_t value_size = 20;

    set_commitment_accumulator<CRH_with_bit_out_gadget<FieldT> > accumulator(max_set_size, value_size);

    std::vector<bit_vector> set_elems;
    for (size_t i = 0; i < max_set_size; ++i)
    {
        bit_vector elem(value_size);
        std::generate(elem.begin(), elem.end(), [&]() { return std::rand() % 2; });
        set_elems.emplace_back(elem);
        accumulator.add(elem);
        assert(accumulator.is_in_set(elem));
    }

    protoboard<FieldT> pb;
    pb_variable_array<FieldT> element_bits;
    element_bits.allocate(pb, value_size, "element_bits");
    set_commitment_variable<FieldT> root_digest(pb, digest_len, "root_digest");

    pb_variable<FieldT> check_succesful;
    check_succesful.allocate(pb, "check_succesful");

    set_commitment_gadget<FieldT> sc(pb, max_set_size, element_bits, root_digest, check_succesful, "sc");
    sc.generate_r1cs_constraints();

    /* test all elements from set */
    for (size_t i = 0; i < max_set_size; ++i)
    {
        element_bits.fill_with_bits(pb, set_elems[i]);
        pb.val(check_succesful) = FieldT::one();
        sc.generate_r1cs_witness(accumulator.get_membership_proof(set_elems[i]));
        root_digest.bits.fill_with_bits(pb, accumulator.get_commitment());
        assert(pb.is_satisfied());
    }
    printf("membership tests OK\n");

    /* test an element not in set */
    for (size_t i = 0; i < value_size; ++i)
    {
        pb.val(element_bits[i]) = FieldT(std::rand() % 2);
    }

    pb.val(check_succesful) = FieldT::zero(); /* do not require the check result to be successful */
    sc.generate_r1cs_witness(accumulator.get_membership_proof(set_elems[0])); /* try it with invalid proof */
    root_digest.bits.fill_with_bits(pb, accumulator.get_commitment());
    assert(pb.is_satisfied());

    pb.val(check_succesful) = FieldT::one(); /* now require the check result to be succesful */
    sc.generate_r1cs_witness(accumulator.get_membership_proof(set_elems[0])); /* try it with invalid proof */
    root_digest.bits.fill_with_bits(pb, accumulator.get_commitment());
    assert(!pb.is_satisfied()); /* the protoboard should be unsatisfied */
    printf("non-membership test OK\n");
}

} // libsnark

#endif // SET_COMMITMENT_GADGET_TCC_
