/** @file
 *****************************************************************************

 Implementation of interfaces for the tally compliance predicate.

 See tally_cp.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef TALLY_CP_TCC_
#define TALLY_CP_TCC_

#include <algorithm>
#include <functional>

namespace libsnark {

template<typename FieldT>
void tally_pcd_message<FieldT>::print() const
{
    assert(wordsize > 0);
    printf("Message type: %zu\n", this->type);

    std::function<bool(FieldT)> FieldT_to_bool = [](const FieldT &el) { return el == FieldT::one(); };

    printf("Message contents:\n");
    printf("   Total sum: ");
    bit_vector sum_bv(wordsize);
    std::transform(this->payload.begin(), this->payload.begin() + wordsize, sum_bv.begin(), FieldT_to_bool);
    convert_bit_vector_to_field_element<FieldT>(sum_bv).print();

    printf("   Number of summands: ");
    bit_vector count_bv(wordsize);
    std::transform(this->payload.begin() + wordsize, this->payload.begin() + 2 * wordsize, count_bv.begin(), FieldT_to_bool);
    convert_bit_vector_to_field_element<FieldT>(count_bv).print();
}

template<typename FieldT>
tally_cp_handler<FieldT>::tally_cp_handler(const size_t type, const size_t max_arity, const size_t wordsize) :
    compliance_predicate_handler<FieldT, protoboard<FieldT> >(protoboard<FieldT>()),
    wordsize(wordsize)
{
    message_length = 2*wordsize;

    type_out.allocate(this->pb, "type_out");

    sum_out_bits.allocate(this->pb, wordsize, "sum_out_bits");
    count_out_bits.allocate(this->pb, wordsize, "count_out_bits");

    arity.allocate(this->pb, "arity");

    type_in.resize(max_arity);
    sum_in_bits.resize(max_arity);
    count_in_bits.resize(max_arity);
    for (size_t i = 0; i < max_arity; ++i)
    {
        type_in[i].allocate(this->pb, FMT("", "type_in_%zu", i));
        sum_in_bits[i].allocate(this->pb, wordsize, FMT("", "sum_in_bits_%zu", i));
        count_in_bits[i].allocate(this->pb, wordsize, FMT("", "count_in_bits_%zu", i));
    }

    local_data.allocate(this->pb, "local_data");

    sum_out_packed.allocate(this->pb, "sum_out_packed");
    count_out_packed.allocate(this->pb, "count_out_packed");

    sum_in_packed.allocate(this->pb, max_arity, "sum_in_packed");
    count_in_packed.allocate(this->pb, max_arity, "count_in_packed");

    dummy.allocate(this->pb, "dummy"); // equals square of local_data

    sum_in_packed_aux.allocate(this->pb, max_arity, "sum_in_packed_aux");
    count_in_packed_aux.allocate(this->pb, max_arity, "count_in_packed_aux");

    type_val_inner_product.allocate(this->pb, "type_val_inner_product");
    compute_type_val_inner_product.reset(new inner_product_gadget<FieldT>(this->pb, type_in, sum_in_packed, type_val_inner_product, "compute_type_val_inner_product"));

    unpack_sum_out.reset(new packing_gadget<FieldT>(this->pb, sum_out_bits, sum_out_packed, "pack_sum_out"));
    unpack_count_out.reset(new packing_gadget<FieldT>(this->pb, count_out_bits, count_out_packed, "pack_count_out"));

    for (size_t i = 0; i < max_arity; ++i)
    {
        pack_sum_in.emplace_back(packing_gadget<FieldT>(this->pb, sum_in_bits[i], sum_in_packed[i], FMT("", "pack_sum_in_%zu", i)));
        pack_count_in.emplace_back(packing_gadget<FieldT>(this->pb, count_in_bits[i], count_in_packed[i], FMT("", "pack_count_in_%zu", i)));
    }

    arity_indicators.allocate(this->pb, max_arity+1, "arity_indicators");

    /* set parameters */
    this->pb.set_input_sizes(message_length + 1); /* +1 accounts for type */

    this->name = type * 100;
    this->type = type;
    this->outgoing_message_payload_length = message_length;
    this->max_arity = max_arity;
    this->incoming_message_payload_lengths.resize(max_arity, message_length);
    this->local_data_length = 1;
    const size_t all_but_witness_length = (1+max_arity)*(1+message_length) + 2; // output, arity, input, local_data
    this->witness_length = this->pb.num_variables() - all_but_witness_length;
    this->relies_on_same_type_inputs = false;
}

template<typename FieldT>
void tally_cp_handler<FieldT>::generate_r1cs_constraints()
{
    unpack_sum_out->generate_r1cs_constraints(true);
    unpack_count_out->generate_r1cs_constraints(true);

    for (size_t i = 0; i < this->max_arity; ++i)
    {
        pack_sum_in[i].generate_r1cs_constraints(true);
        pack_count_in[i].generate_r1cs_constraints(true);
    }

    for (size_t i = 0; i < this->max_arity; ++i)
    {
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(type_in[i], sum_in_packed_aux[i], sum_in_packed[i]), FMT("", "initial_sum_%zu_is_zero", i));
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(type_in[i], count_in_packed_aux[i], count_in_packed[i]), FMT("", "initial_sum_%zu_is_zero", i));
    }

    /* constrain arity indicator variables so that arity_indicators[arity] = 1 and arity_indicators[i] = 0 for any other i */
    for (size_t i = 0; i < this->max_arity; ++i)
    {
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(arity - FieldT(i), arity_indicators[i], 0), FMT("", "arity_indicators_%zu", i));
    }

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, pb_sum<FieldT>(arity_indicators), 1), "arity_indicators");

    /* require that types of messages that are past arity (i.e. unbound wires) carry 0 */
    for (size_t i = 0; i < this->max_arity; ++i)
    {
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(0 + pb_sum<FieldT>(pb_variable_array<FieldT>(arity_indicators.begin(), arity_indicators.begin() + i)), type_in[i], 0), FMT("", "unbound_types_%zu", i));
    }

    /* sum_out = local_data + \sum_i type[i] * sum_in[i] */
    compute_type_val_inner_product->generate_r1cs_constraints();
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, type_val_inner_product + local_data, sum_out_packed), "update_sum");

    /* count_out = 1 + \sum_i count_in[i] */
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, 1 + pb_sum<FieldT>(count_in_packed), count_out_packed), "update_count");

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(local_data, local_data, dummy), "dummy_witness");
}

template<typename FieldT>
void tally_cp_handler<FieldT>::generate_r1cs_witness(const std::vector<tally_pcd_message<FieldT> > &input, const r1cs_pcd_local_data<FieldT> &ld)
{
    assert(ld.payload.size() == 1);
    this->pb.clear_values();
    this->pb.val(arity) = FieldT(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        this->pb.val(type_in[i]) = FieldT(input[i].type);
        sum_in_bits[i].fill_with_field_elements(this->pb, std::vector<FieldT>(input[i].payload.begin() + 0*wordsize, input[i].payload.begin() + 1*wordsize));
        count_in_bits[i].fill_with_field_elements(this->pb, std::vector<FieldT>(input[i].payload.begin() + 1*wordsize, input[i].payload.begin() + 2*wordsize));
    }

    for (size_t i = 0; i < this->max_arity; ++i)
    {
        pack_sum_in[i].generate_r1cs_witness_from_bits();
        pack_count_in[i].generate_r1cs_witness_from_bits();

        if (!this->pb.val(type_in[i]).is_zero())
        {
            this->pb.val(sum_in_packed_aux[i]) = this->pb.val(sum_in_packed[i]) * this->pb.val(type_in[i]).inverse();
            this->pb.val(count_in_packed_aux[i]) = this->pb.val(count_in_packed[i]) * this->pb.val(type_in[i]).inverse();
        }
    }

    for (size_t i = 0; i < this->max_arity + 1; ++i)
    {
        this->pb.val(arity_indicators[i]) = (input.size() == i ? FieldT::one() : FieldT::zero());
    }

    this->pb.val(local_data) = ld.payload[0];

    this->pb.val(dummy) = this->pb.val(local_data) * this->pb.val(local_data);

    compute_type_val_inner_product->generate_r1cs_witness();
    this->pb.val(sum_out_packed) = this->pb.val(local_data) + this->pb.val(type_val_inner_product);

    this->pb.val(count_out_packed) = FieldT::one();
    for (size_t i = 0; i < this->max_arity; ++i)
    {
        this->pb.val(count_out_packed) += this->pb.val(count_in_packed[i]);
    }

    unpack_sum_out->generate_r1cs_witness_from_packed();
    unpack_count_out->generate_r1cs_witness_from_packed();

    this->pb.val(type_out) = FieldT(this->type);
}

template<typename FieldT>
tally_pcd_message<FieldT> tally_cp_handler<FieldT>::get_base_case_message() const
{
    tally_pcd_message<FieldT> result;
    result.type = 0;
    result.payload.resize(2*wordsize, FieldT::zero());
    result.wordsize = wordsize;
    return result;
}

} // libsnark

#endif // TALLY_CP_TCC_
