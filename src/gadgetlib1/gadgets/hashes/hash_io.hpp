/**
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/
#ifndef HASH_IO_HPP_
#define HASH_IO_HPP_
#include <cstddef>
#include <vector>
#include "gadgetlib1/gadgets/basic_gadgets.hpp"

namespace libsnark {

struct merkle_authentication_node {
    bool computed_is_right;
    bit_vector aux_digest;

    bool operator==(const merkle_authentication_node &other) const
    {
        return (this->computed_is_right == other.computed_is_right &&
                this->aux_digest == other.aux_digest);
    }

    friend std::ostream& operator<<(std::ostream &out, const merkle_authentication_node &node)
    {
        output_bool(out, node.computed_is_right);
        output_bool_vector(out, node.aux_digest);
        return out;
    }

    friend std::istream& operator>>(std::istream &in, merkle_authentication_node &node)
    {
        input_bool(in, node.computed_is_right);
        input_bool_vector(in, node.aux_digest);
        return in;
    }
};

typedef std::vector<merkle_authentication_node> merkle_authentication_path;

template<typename FieldT>
class digest_variable : public gadget<FieldT> {
public:
    size_t digest_size;
    pb_variable_array<FieldT> bits;

    digest_variable<FieldT>(protoboard<FieldT> &pb,
                            const size_t digest_size,
                            const std::string &annotation_prefix);

    digest_variable<FieldT>(protoboard<FieldT> &pb,
                            const size_t digest_size,
                            const pb_variable_array<FieldT> &partial_bits,
                            const pb_variable<FieldT> &padding,
                            const std::string &annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness(const bit_vector& contents);
    bit_vector get_digest() const;
};

template<typename FieldT>
class block_variable : public gadget<FieldT> {
public:
    size_t block_size;
    pb_variable_array<FieldT> bits;

    block_variable(protoboard<FieldT> &pb,
                   const size_t block_size,
                   const std::string &annotation_prefix);

    block_variable(protoboard<FieldT> &pb,
                   const std::vector<pb_variable_array<FieldT> > &parts,
                   const std::string &annotation_prefix);

    block_variable(protoboard<FieldT> &pb,
                   const digest_variable<FieldT> &left,
                   const digest_variable<FieldT> &right,
                   const std::string &annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness(const bit_vector& contents);
    bit_vector get_block() const;
};

} // libsnark
#include "gadgetlib1/gadgets/hashes/hash_io.tcc"

#endif // HASH_IO_HPP_
