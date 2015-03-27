/** @file
 *****************************************************************************

 Declaration of interfaces for a compliance predicate handler.

 A compliance predicate handler is a base class for creating compliance predicates.
 It relies on classes declared in gadgetlib1.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef CP_HANDLER_HPP_
#define CP_HANDLER_HPP_

#include "zk_proof_systems/pcd/r1cs_pcd/compliance_predicate/compliance_predicate.hpp"
#include "gadgetlib1/protoboard.hpp"

namespace libsnark {

/***************************** Message variable ******************************/

/**
 * A variable to represent an r1cs_pcd_message.
 */
template<typename FieldT>
class r1cs_pcd_message_variable : public gadget<FieldT> {
public:

    pb_variable<FieldT> type;
    pb_variable_array<FieldT> payload;

    pb_variable_array<FieldT> all_vars;

    size_t payload_size;

    r1cs_pcd_message_variable(protoboard<FieldT> &pb,
                              const size_t payload_size,
                              const std::string &annotation_prefix);

    void generate_r1cs_witness(const r1cs_pcd_message<FieldT> &msg);

    r1cs_pcd_message<FieldT> get_message() const;

};

/*************************** Local data variable *****************************/

/**
 * A variable to represent an r1cs_pcd_local_data.
 */
template<typename FieldT>
class r1cs_pcd_local_data_variable : public gadget<FieldT> {
public:

    pb_variable_array<FieldT> all_vars;

    r1cs_pcd_local_data_variable(protoboard<FieldT> &pb,
                                 const std::string &annotation_prefix);

    void generate_r1cs_witness(const r1cs_pcd_local_data<FieldT> &ld);

};

/*********************** Compliance predicate handler ************************/

/**
 * A base class for creating compliance predicates.
 */
template<typename FieldT, typename protoboardT>
class compliance_predicate_handler {
protected:
    protoboardT pb;

    size_t name;
    size_t type;
    size_t outgoing_message_payload_length;
    size_t max_arity;
    std::vector<size_t> incoming_message_payload_lengths;
    size_t local_data_length;
    size_t witness_length;
    bool relies_on_same_type_inputs;
    std::set<size_t> accepted_input_types;
public:
    compliance_predicate_handler(const protoboardT &pb);

    void generate_r1cs_constraints();
    void generate_r1cs_witness(const std::vector<r1cs_pcd_message<FieldT> > &input_messages,
                               const r1cs_pcd_local_data<FieldT> &local_data,
                               const bool is_base_case);

    r1cs_pcd_compliance_predicate<FieldT> get_compliance_predicate() const;
    r1cs_variable_assignment<FieldT> get_full_variable_assignment() const;

    r1cs_pcd_message<FieldT> get_outgoing_message() const;
    size_t get_arity() const;
    r1cs_pcd_message<FieldT> get_incoming_message(const size_t message_idx) const;
    r1cs_pcd_local_data<FieldT> get_local_data() const;
    r1cs_variable_assignment<FieldT> get_witness() const;

    r1cs_pcd_compliance_predicate_primary_input<FieldT> get_primary_input() const;
    r1cs_pcd_compliance_predicate_auxiliary_input<FieldT> get_auxiliary_input() const;
};

} // libsnark

#include "zk_proof_systems/pcd/r1cs_pcd/compliance_predicate/cp_handler.tcc"

#endif // CP_HANDLER_HPP_
