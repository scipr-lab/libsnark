/**
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/
#ifndef HASH_IO_TCC_
#define HASH_IO_TCC_

namespace libsnark {

template<typename FieldT>
digest_variable<FieldT>::digest_variable(protoboard<FieldT> &pb,
                                         const size_t digest_size,
                                         const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix), digest_size(digest_size)
{
    bits.allocate(pb, digest_size, FMT(this->annotation_prefix, " bits"));
}

template<typename FieldT>
digest_variable<FieldT>::digest_variable(protoboard<FieldT> &pb,
                                         const size_t digest_size,
                                         const pb_variable_array<FieldT> &partial_bits,
                                         const pb_variable<FieldT> &padding,
                                         const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix), digest_size(digest_size)
{
    assert(bits.size() <= digest_size);
    bits = partial_bits;
    while (bits.size() != digest_size)
    {
        bits.emplace_back(padding);
    }
}

template<typename FieldT>
void digest_variable<FieldT>::generate_r1cs_constraints()
{
    for (size_t i = 0; i < digest_size; ++i)
    {
        generate_boolean_r1cs_constraint<FieldT>(this->pb, bits[i], FMT(this->annotation_prefix, " bits_%zu", i));
    }
}

template<typename FieldT>
void digest_variable<FieldT>::fill_with_bits(const bit_vector& contents)
{
    assert(contents.size() == digest_size);
    for (size_t i = 0; i < digest_size; ++i)
    {
        this->pb.val(bits[i]) = contents[i] ? FieldT::one() : FieldT::zero();
    }
}

template<typename FieldT>
block_variable<FieldT>::block_variable(protoboard<FieldT> &pb,
                                       const size_t block_size,
                                       const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix), block_size(block_size)
{
    bits.allocate(pb, block_size, FMT(this->annotation_prefix, " bits"));
}

template<typename FieldT>
block_variable<FieldT>::block_variable(protoboard<FieldT> &pb,
                                       const std::vector<pb_variable_array<FieldT> > &parts,
                                       const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix)
{
    for (auto &part : parts)
    {
        bits.insert(bits.end(), part.begin(), part.end());
    }
}

template<typename FieldT>
block_variable<FieldT>::block_variable(protoboard<FieldT> &pb,
                                       const digest_variable<FieldT> &left,
                                       const digest_variable<FieldT> &right,
                                       const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix)
{
    assert(left.bits.size() == right.bits.size());
    block_size = 2 * left.bits.size();
    bits.insert(bits.end(), left.bits.begin(), left.bits.end());
    bits.insert(bits.end(), right.bits.begin(), right.bits.end());
}

template<typename FieldT>
void block_variable<FieldT>::fill_with_bits(const bit_vector& contents)
{
    assert(contents.size() == block_size);
    for (size_t i = 0; i < block_size; ++i)
    {
        this->pb.val(bits[i]) = contents[i] ? FieldT::one() : FieldT::zero();
    }
}

} // libsnark
#endif // HASH_IO_TCC_
