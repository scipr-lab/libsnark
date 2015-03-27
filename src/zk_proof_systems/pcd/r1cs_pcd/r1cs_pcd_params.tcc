#ifndef R1CS_PCD_PARAMS_TCC_
#define R1CS_PCD_PARAMS_TCC_

namespace libsnark {

template<typename FieldT>
r1cs_primary_input<FieldT> r1cs_pcd_compliance_predicate_primary_input<FieldT>::as_r1cs_primary_input() const
{
    r1cs_primary_input<FieldT> result;

    result.emplace_back(FieldT(outgoing_message.type));
    result.insert(result.end(), outgoing_message.payload.begin(), outgoing_message.payload.end());

    return result;
}

template<typename FieldT>
r1cs_auxiliary_input<FieldT> r1cs_pcd_compliance_predicate_auxiliary_input<FieldT>::as_r1cs_auxiliary_input(const std::vector<size_t> &incoming_message_payload_lengths) const
{
    r1cs_auxiliary_input<FieldT> result;
    result.emplace_back(FieldT(incoming_messages.size()));

    const size_t max_arity = incoming_message_payload_lengths.size();

    for (size_t i = 0; i < max_arity; ++i)
    {
        if (i < incoming_messages.size())
        {
            result.emplace_back(FieldT(incoming_messages[i].type));
            result.insert(result.end(), incoming_messages[i].payload.begin(), incoming_messages[i].payload.end());
        }
        else
        {
            result.resize(result.size() + incoming_message_payload_lengths[i] + 1, FieldT::zero());
        }
    }

    result.insert(result.end(), local_data.payload.begin(), local_data.payload.end());
    result.insert(result.end(), witness.begin(), witness.end());

    return result;
}

} // libsnark

#endif // R1CS_PCD_PARAMS_TCC_
