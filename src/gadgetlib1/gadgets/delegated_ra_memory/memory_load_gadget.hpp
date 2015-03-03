/** @file
 *****************************************************************************

 Declaration of interfaces for the memory load gadget.

 The gadget checks the following: given a root R, address A, value V, and
 authentication path P, check that P is a valid authentication path for the
 value V as the A-th leaf in a Merkle tree with root R.

 The gadget can be used to verify a memory load from a "delegated memory".

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef MEMORY_LOAD_GADGET_HPP_
#define MEMORY_LOAD_GADGET_HPP_

#include "gadgetlib1/gadgets/hashes/crh_gadget.hpp"
#include "gadgetlib1/gadgets/hashes/hash_io.hpp"
#include "gadgetlib1/gadgets/hashes/digest_selector_gadget.hpp"

namespace libsnark {

template<typename FieldT>
class memory_load_gadget : public gadget<FieldT> {
private:

    std::vector<CRH_with_bit_out_gadget<FieldT> > hashers;
    std::vector<block_variable<FieldT> > hasher_inputs;
    std::vector<digest_selector_gadget<FieldT> > propagators;
    std::vector<digest_variable<FieldT> > internal_left;
    std::vector<digest_variable<FieldT> > internal_right;
    std::vector<digest_variable<FieldT> > internal_output;

public:

    const size_t digest_size;
    const size_t tree_depth;
    pb_variable_array<FieldT> address_bits;
    digest_variable<FieldT> leaf;
    digest_variable<FieldT> root;

    memory_load_gadget(protoboard<FieldT> &pb,
                       const size_t tree_depth,
                       const pb_variable_array<FieldT> &address_bits,
                       const digest_variable<FieldT> &leaf_digest,
                       const digest_variable<FieldT> &root_digest,
                       const std::string &annotation_prefix);

    void generate_r1cs_constraints();

    void generate_r1cs_witness(const bit_vector &leaf, const bit_vector &root, const merkle_authentication_path &path);

    /* for debugging purposes */
    static size_t expected_constraints(const size_t tree_depth);
};

template<typename FieldT>
void test_memory_load_gadget();

} // libsnark

#include "gadgetlib1/gadgets/delegated_ra_memory/memory_load_gadget.tcc"

#endif // MEMORY_LOAD_GADGET_HPP_
