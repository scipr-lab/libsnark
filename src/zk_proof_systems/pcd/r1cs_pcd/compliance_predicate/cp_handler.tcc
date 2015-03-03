/** @file
 *****************************************************************************

 Implementation of interfaces for a compliance predicate handler.

 See cp_handler.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef CP_HANDLER_TCC_
#define CP_HANDLER_TCC_

namespace libsnark {

template<typename FieldT>
r1cs_pcd_message_variable<FieldT>::r1cs_pcd_message_variable(protoboard<FieldT> &pb,
                                                             const size_t payload_size,
                                                             const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix),
    payload_size(payload_size)
{
    type.allocate(pb, FMT(annotation_prefix, " type"));
    payload.allocate(pb, payload_size, FMT(annotation_prefix, " payload"));

    all_vars = payload;
    all_vars.insert(all_vars.begin(), type);
}

template<typename FieldT>
void r1cs_pcd_message_variable<FieldT>::generate_r1cs_witness(const r1cs_pcd_message<FieldT> &msg)
{
    this->pb.val(type) = FieldT(msg.type);
    payload.fill_with_field_elements(this->pb, msg.payload);
}

template<typename FieldT>
r1cs_pcd_message<FieldT> r1cs_pcd_message_variable<FieldT>::get_message() const
{
    r1cs_pcd_message<FieldT> result;
    result.type = this->pb.val(type).as_ulong();
    result.payload = payload.get_vals(this->pb);

    return result;
}

template<typename FieldT>
r1cs_pcd_local_data_variable<FieldT>::r1cs_pcd_local_data_variable(protoboard<FieldT> &pb,
                                                                   const std::string &annotation_prefix) :
    gadget<FieldT>(pb, annotation_prefix)
{
}

template<typename FieldT, typename protoboardT>
compliance_predicate_handler<FieldT, protoboardT>::compliance_predicate_handler(const protoboardT &pb) :
    pb(pb)
{
}

template<typename FieldT, typename protoboardT>
r1cs_pcd_compliance_predicate<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_compliance_predicate() const
{
    return r1cs_pcd_compliance_predicate<FieldT>(name,
                                                 type,
                                                 pb.get_constraint_system(),
                                                 outgoing_message_payload_length,
                                                 max_arity,
                                                 incoming_message_payload_lengths,
                                                 local_data_length,
                                                 witness_length,
                                                 relies_on_same_type_inputs);
}

template<typename FieldT, typename protoboardT>
r1cs_variable_assignment<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_full_variable_assignment() const
{
    return pb.full_variable_assignment();
}

template<typename FieldT, typename protoboardT>
r1cs_pcd_message<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_outgoing_message() const
{
    const r1cs_variable_assignment<FieldT> va = pb.full_variable_assignment();
    size_t pos = 0;
    r1cs_pcd_message<FieldT> result;
    result.type = va[pos].as_ulong();
    pos += 1;
    const size_t len = outgoing_message_payload_length;
    result.payload = r1cs_variable_assignment<FieldT>(va.begin() + pos, va.begin() + pos + len);
    return result;
}

template<typename FieldT, typename protoboardT>
size_t compliance_predicate_handler<FieldT, protoboardT>::get_arity() const
{
    const r1cs_variable_assignment<FieldT> va = pb.full_variable_assignment();
    const size_t pos = 1 + outgoing_message_payload_length;
    return va[pos].as_ulong();
}

template<typename FieldT, typename protoboardT>
r1cs_pcd_message<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_incoming_message(const size_t message_idx) const
{
    const r1cs_variable_assignment<FieldT> va = pb.full_variable_assignment();
    assert(message_idx < max_arity);
    size_t pos = 1 + outgoing_message_payload_length + 1; // constant term + output msg + arity
    for (size_t i = 0; i < message_idx; ++i)
    {
        pos += 1 + incoming_message_payload_lengths[i]; // type + payload
    }

    r1cs_pcd_message<FieldT> result;
    result.type = va[pos].as_ulong();
    pos += 1;
    const size_t len = incoming_message_payload_lengths[message_idx];
    result.payload = r1cs_variable_assignment<FieldT>(va.begin() + pos, va.begin() + pos + len);
    return result;
}

template<typename FieldT, typename protoboardT>
r1cs_pcd_local_data<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_local_data() const
{
    const r1cs_variable_assignment<FieldT> va = pb.full_variable_assignment();
    size_t pos = 1 + outgoing_message_payload_length + 1; // constant term + output msg + arity
    for (size_t i = 0; i < max_arity; ++i)
    {
        pos += 1 + incoming_message_payload_lengths[i]; // type + payload
    }

    const size_t len = local_data_length;

    r1cs_pcd_local_data<FieldT> result;
    result.payload = r1cs_variable_assignment<FieldT>(va.begin() + pos, va.begin() + pos + len);
    return result;
}

template<typename FieldT, typename protoboardT>
r1cs_pcd_witness<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_witness() const
{
    const r1cs_variable_assignment<FieldT> va = pb.full_variable_assignment();
    size_t pos = 1 + outgoing_message_payload_length + 1; // constant term + output msg + arity
    for (size_t i = 0; i < max_arity; ++i)
    {
        pos += 1 + incoming_message_payload_lengths[i]; // type + payload
    }
    pos += local_data_length;

    const size_t len = witness_length;
    return r1cs_variable_assignment<FieldT>(va.begin() + pos, va.begin() + pos + len);
}

template<typename FieldT, typename protoboardT>
r1cs_pcd_compliance_predicate_primary_input<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_primary_input() const
{
    r1cs_pcd_compliance_predicate_primary_input<FieldT> result;
    result.outgoing_message = get_outgoing_message();
    return result;
}

template<typename FieldT, typename protoboardT>
r1cs_pcd_compliance_predicate_auxiliary_input<FieldT> compliance_predicate_handler<FieldT, protoboardT>::get_auxiliary_input() const
{
    r1cs_pcd_compliance_predicate_auxiliary_input<FieldT> result;

    const size_t arity = get_arity();

    for (size_t msg_idx = 0; msg_idx < arity; ++msg_idx)
    {
        result.incoming_messages.emplace_back(get_incoming_message(msg_idx));
    }
    result.local_data = get_local_data();
    result.witness = get_witness();

    return result;
}

} // libsnark

#endif // CP_HANDLER_TCC_
