/** @file
 *****************************************************************************

 Implementation of interfaces for a compliance predicate for RAM.

 See ram_compliance_predicate.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef RAM_COMPLIANCE_PREDICATE_TCC_
#define RAM_COMPLIANCE_PREDICATE_TCC_

namespace libsnark {

template<typename ramT>
void ram_message<ramT>::print(const ram_architecture_params<ramT> &ap) const
{
    ram_protoboard<ramT> pb(ap);
    ram_compliance_message_vars<ramT> vars(pb, "vars");
    vars.deserialize(this->contents);
    vars.print();
}

template<typename ramT>
class ram_compliance_message_vars {
public:
    typedef ram_base_field<ramT> FieldT;

    ram_protoboard<ramT> &pb;
    pb_variable_array<FieldT> timestamp;
    pb_variable_array<FieldT> root_initial;
    pb_variable_array<FieldT> root;
    pb_variable_array<FieldT> pc_addr;
    pb_variable_array<FieldT> cpu_state;
    pb_variable_array<FieldT> pc_addr_initial;
    pb_variable_array<FieldT> cpu_state_initial;
    pb_variable<FieldT> has_accepted;

    pb_variable_array<FieldT> all_vars;
private:
    void print_vec(const char *prefix, const pb_variable_array<FieldT> &va) const;
public:
    const size_t addr_size;
    const size_t value_size;
    const size_t cpu_state_size;
    const size_t digest_size;

    ram_compliance_message_vars(ram_protoboard<ramT> &pb,
                                const std::string &annotation_prefix);
    std::vector<FieldT> serialize() const;
    void deserialize(const std::vector<FieldT> &v);
    void print() const;
    static size_t size_in_bits(const ram_architecture_params<ramT> &ap);
};

template<typename ramT>
ram_compliance_message_vars<ramT>::ram_compliance_message_vars(ram_protoboard<ramT> &pb,
                                                               const std::string &annotation_prefix) :
    pb(pb),
    addr_size(pb.ap.address_size()),
    value_size(pb.ap.value_size()),
    cpu_state_size(pb.ap.cpu_state_size()),
    digest_size(CRH_with_bit_out_gadget<FieldT>::get_digest_len())
{
    timestamp.allocate(this->pb, ramT::timestamp_length, FMT(annotation_prefix, " timestamp"));
    root_initial.allocate(this->pb, digest_size, FMT(annotation_prefix, " root_initial"));
    root.allocate(this->pb, digest_size, FMT(annotation_prefix, " root"));
    pc_addr.allocate(this->pb, addr_size, FMT(annotation_prefix, " pc_addr"));
    cpu_state.allocate(this->pb, cpu_state_size, FMT(annotation_prefix, " cpu_state"));
    pc_addr_initial.allocate(this->pb, addr_size, FMT(annotation_prefix, " pc_addr_initial"));
    cpu_state_initial.allocate(this->pb, cpu_state_size, FMT(annotation_prefix, " cpu_state_initial"));
    has_accepted.allocate(this->pb, FMT(annotation_prefix, " has_accepted"));

    all_vars.insert(all_vars.end(), timestamp.begin(), timestamp.end());
    all_vars.insert(all_vars.end(), root_initial.begin(), root_initial.end());
    all_vars.insert(all_vars.end(), root.begin(), root.end());
    all_vars.insert(all_vars.end(), pc_addr.begin(), pc_addr.end());
    all_vars.insert(all_vars.end(), cpu_state.begin(), cpu_state.end());
    all_vars.insert(all_vars.end(), pc_addr_initial.begin(), pc_addr_initial.end());
    all_vars.insert(all_vars.end(), cpu_state_initial.begin(), cpu_state_initial.end());
    all_vars.insert(all_vars.end(), has_accepted);
}

template<typename ramT>
std::vector<ram_base_field<ramT> > ram_compliance_message_vars<ramT>::serialize() const
{
    return all_vars.get_vals(pb);
}

template<typename ramT>
void ram_compliance_message_vars<ramT>::deserialize(const std::vector<FieldT> &v)
{
    all_vars.fill_with_bits(pb, v);
}

template<typename ramT>
void ram_compliance_message_vars<ramT>::print_vec(const char *prefix, const pb_variable_array<FieldT> &va) const
{
    printf("bin(%s) = ", prefix);
    for (auto it = va.rbegin(); it != va.rend(); ++it)
    {
        printf("%ld", pb.val(*it).as_ulong());
    }
    printf("\n");
}

template<typename ramT>
void ram_compliance_message_vars<ramT>::print() const
{
    print_vec("timestamp", timestamp);
    print_vec("root_initial", root_initial);
    print_vec("root", root);
    print_vec("pc_addr", pc_addr);
    print_vec("cpu_state", cpu_state);
    print_vec("pc_addr_initial", pc_addr_initial);
    print_vec("cpu_state_initial", cpu_state_initial);
}

template<typename ramT>
size_t ram_compliance_message_vars<ramT>::size_in_bits(const ram_architecture_params<ramT> &ap)
{
    ram_protoboard<ramT> pb(ap);
    ram_compliance_message_vars<ramT> vars(pb, "vars");
    return vars.all_vars.size();
}

/*
  We need to perform the following checks:

  Always:
  next.root_initial = cur.root_initial
  next.pc_addr_init = cur.pc_addr_initial
  next.cpu_state_initial = cur.cpu_state_initial

  If is_is_base_case = 1: (base case)
  that cur.timestamp = 0, cur.cpu_state = cpu_state_init, cur.pc_addr = pc_addr_initial, cur.has_accepted = 0
  that cur.root = cur.root_initial

  If do_halt = 0: (regular case)
  that instruction fetch was correctly executed
  next.timestamp = cur.timestamp + 1
  that CPU accepted on (cur, temp)
  that load-then-store was correctly handled
  that next.root = temp.root, next.cpu_state = temp.cpu_state, next.pc_addr = temp.pc_addr

  If do_halt = 1: (final case)
  that cur.has_accepted = 1
  that next.root = 0, next.cpu_state = 0, next.pc_addr = 0
  that next.timestamp = cur.timestamp and next.has_accepted = cur.has_accepted
*/

template<typename ramT>
ram_compliance_predicate_handler<ramT>::ram_compliance_predicate_handler(const ram_architecture_params<ramT> &ap)
    :
    compliance_predicate_handler<ram_base_field<ramT>, ram_protoboard<ramT> >(ram_protoboard<ramT>(ap)),
    ap(ap),
    addr_size(ap.address_size()),
    value_size(ap.value_size()),
    digest_size(CRH_with_bit_out_gadget<FieldT>::get_digest_len())
{
    // TODO: assert that message has fields of lengths consistent with num_addresses/value_size (as a method for ram_message)
    // choose a constant for timestamp_len
    // check that value_size <= digest_size; digest_size is not assumed to fit in chunk size (more precisely, it is handled correctly in the other gadgets).
    // check if others fit (timestamp_length, value_size, addr_size)

    // the variables allocated are: next, cur, local data (nil for us), is_base_case, witness

    const size_t chunk_size = FieldT::size_in_bits() - 1;

    const size_t message_length_in_bits = ram_compliance_message_vars<ramT>::size_in_bits(this->pb.ap);
    message_length = div_ceil(message_length_in_bits, chunk_size);

    next_type.allocate(this->pb, "next_type");
    next_packed.allocate(this->pb, message_length, "next_packed");
    arity.allocate(this->pb, "arity");
    cur_type.allocate(this->pb, "cur_type");
    cur_packed.allocate(this->pb, message_length, "cur_packed");

    is_base_case.allocate(this->pb, "is_base_case");

    next.reset(new ram_compliance_message_vars<ramT>(this->pb, "next"));
    cur.reset(new ram_compliance_message_vars<ramT>(this->pb, "cur"));

    unpack_next.reset(new multipacking_gadget<FieldT>(this->pb, next->all_vars, next_packed, chunk_size, "unpack_next"));
    unpack_cur.reset(new multipacking_gadget<FieldT>(this->pb, cur->all_vars, cur_packed, chunk_size, "cur_next"));

    // work-around for bad linear combination handling
    zero.allocate(this->pb, "zero"); // will go away when we properly support linear terms

    temp_next_pc_addr.allocate(this->pb, addr_size, "temp_next_pc_addr");
    temp_next_cpu_state.allocate(this->pb, cur->cpu_state_size, "temp_next_cpu_state");

    /*
      Always:
      next.root_initial = cur.root_initial
      next.pc_addr_init = cur.pc_addr_initial
      next.cpu_state_initial = cur.cpu_state_initial
    */
    copy_root_initial.reset(new bit_vector_copy_gadget<FieldT>(this->pb, cur->root_initial, next->root_initial, ONE, chunk_size, "copy_root_initial"));
    copy_pc_addr_initial.reset(new bit_vector_copy_gadget<FieldT>(this->pb, cur->pc_addr_initial, next->pc_addr_initial, ONE, chunk_size, "copy_pc_addr_initial"));
    copy_cpu_state_initial.reset(new bit_vector_copy_gadget<FieldT>(this->pb, cur->cpu_state_initial, next->cpu_state_initial, ONE, chunk_size, "copy_cpu_state_initial"));

    /*
      If is_base_case = 1: (base case)
      that cur.timestamp = 0, cur.cpu_state = 0, cur.pc_addr = 0, cur.has_accepted = 0
      that cur.root = cur.root_initial
    */
    packed_cur_timestamp.allocate(this->pb, "packed_cur_timestamp");
    pack_cur_timestamp.reset(new packing_gadget<FieldT>(this->pb, cur->timestamp, packed_cur_timestamp, "pack_cur_timestamp"));

    zero_cpu_state = pb_variable_array<FieldT>(cur->cpu_state.size(), zero);
    zero_pc_addr = pb_variable_array<FieldT>(cur->pc_addr.size(), zero);

    initialize_cur_cpu_state.reset(new bit_vector_copy_gadget<FieldT>(this->pb, cur->cpu_state_initial, cur->cpu_state, is_base_case, chunk_size, "initialize_cur_cpu_state"));
    initialize_prev_pc_addr.reset(new bit_vector_copy_gadget<FieldT>(this->pb, cur->pc_addr_initial, cur->pc_addr, is_base_case, chunk_size, "initialize_prev_pc_addr"));

    initialize_root.reset(new bit_vector_copy_gadget<FieldT>(this->pb, cur->root_initial, cur->root, is_base_case, chunk_size, "initialize_root"));
    /*
      If do_halt = 0: (regular case)
      that instruction fetch was correctly executed
      next.timestamp = cur.timestamp + 1
      that CPU accepted on (cur, next)
      that load-then-store was correctly handled
    */
    is_not_halt_case.allocate(this->pb, "is_not_halt_case");
    // for performing instruction fetch
    prev_pc_val.allocate(this->pb, value_size, "prev_pc_val");
    prev_pc_val_digest.reset(new digest_variable<FieldT>(this->pb, digest_size, prev_pc_val, zero, "prev_pc_val_digest"));
    cur_root_digest.reset(new digest_variable<FieldT>(this->pb, digest_size, cur->root, zero, "cur_root_digest"));
    instruction_fetch_merkle_proof.reset(new merkle_authentication_path_variable<FieldT>(this->pb, addr_size, "instruction_fetch_merkle_proof"));
    instruction_fetch.reset(new memory_load_gadget<FieldT>(this->pb, addr_size,
                                                           cur->pc_addr,
                                                           *prev_pc_val_digest,
                                                           *cur_root_digest,
                                                           *instruction_fetch_merkle_proof,
                                                           ONE,
                                                           "instruction_fetch"));

    // for next.timestamp = cur.timestamp + 1
    packed_next_timestamp.allocate(this->pb, "packed_next_timestamp");
    pack_next_timestamp.reset(new packing_gadget<FieldT>(this->pb, next->timestamp, packed_next_timestamp, "pack_next_timestamp"));

    // that CPU accepted on (cur, temp)
    ls_addr.allocate(this->pb, addr_size, "ls_addr");
    ls_prev_val.allocate(this->pb, value_size, "ls_prev_val");
    ls_next_val.allocate(this->pb, value_size, "ls_next_val");
    cpu_checker.reset(new ram_cpu_checker<ramT>(this->pb, cur->pc_addr, prev_pc_val, cur->cpu_state,
                                                ls_addr, ls_prev_val, ls_next_val,
                                                temp_next_cpu_state, temp_next_pc_addr, next->has_accepted,
                                                "cpu_checker"));

    // that load-then-store was correctly handled
    ls_prev_val_digest.reset(new digest_variable<FieldT>(this->pb, digest_size, ls_prev_val, zero, "ls_prev_val_digest"));
    ls_next_val_digest.reset(new digest_variable<FieldT>(this->pb, digest_size, ls_next_val, zero, "ls_next_val_digest"));
    next_root_digest.reset(new digest_variable<FieldT>(this->pb, digest_size, next->root, zero, "next_root_digest"));
    load_merkle_proof.reset(new merkle_authentication_path_variable<FieldT>(this->pb, addr_size, "load_merkle_proof"));
    store_merkle_proof.reset(new merkle_authentication_path_variable<FieldT>(this->pb, addr_size, "store_merkle_proof"));
    load_store_checker.reset(new memory_load_store_gadget<FieldT>(this->pb, addr_size, ls_addr,
                                                                  *ls_prev_val_digest, *cur_root_digest, *load_merkle_proof,
                                                                  *ls_next_val_digest, *next_root_digest, *store_merkle_proof, is_not_halt_case,
                                                                  "load_store_checker"));
    /*
      If do_halt = 1: (final case)
      that cur.has_accepted = 1
      that next.root = 0, next.cpu_state = 0, next.pc_addr = 0
      that next.timestamp = cur.timestamp and next.has_accepted = cur.has_accepted
    */
    do_halt.allocate(this->pb, "do_halt");
    zero_root = pb_variable_array<FieldT>(next->root.size(), zero);
    clear_next_root.reset(new bit_vector_copy_gadget<FieldT>(this->pb, zero_root, next->root, do_halt, chunk_size, "clear_next_root"));
    clear_next_pc_addr.reset(new bit_vector_copy_gadget<FieldT>(this->pb, zero_pc_addr, next->pc_addr, do_halt, chunk_size, "clear_next_pc_addr"));
    clear_next_cpu_state.reset(new bit_vector_copy_gadget<FieldT>(this->pb, zero_cpu_state, next->cpu_state, do_halt, chunk_size, "clear_cpu_state"));

    copy_temp_next_pc_addr.reset(new bit_vector_copy_gadget<FieldT>(this->pb, temp_next_pc_addr, next->pc_addr, is_not_halt_case, chunk_size, "copy_temp_next_pc_addr"));
    copy_temp_next_cpu_state.reset(new bit_vector_copy_gadget<FieldT>(this->pb, temp_next_cpu_state, next->cpu_state, is_not_halt_case, chunk_size, "copy_temp_next_cpu_state"));

    /* set parameters */
    this->pb.set_input_sizes(message_length + 1); /* +1 accounts for type */

    this->name = 1;
    this->type = 1;
    this->outgoing_message_payload_length = message_length;
    this->max_arity = 1;
    this->incoming_message_payload_lengths.resize(this->max_arity, message_length);
    this->local_data_length = 0;
    this->witness_length = this->pb.num_variables() - (2 * (message_length + 1) + 1); /* additional 1 for arity */
    this->relies_on_same_type_inputs = true;
}

template<typename ramT>
void ram_compliance_predicate_handler<ramT>::generate_r1cs_constraints()
{
    print_indent(); printf("* Message size: %zu\n", next->all_vars.size());
    print_indent(); printf("* Address size: %zu\n", addr_size);
    print_indent(); printf("* CPU state size: %zu\n", next->cpu_state_size);
    print_indent(); printf("* Digest size: %zu\n", next->digest_size);

    PROFILE_CONSTRAINTS(this->pb, "handle next_type, arity and cur_type")
    {
        generate_r1cs_equals_const_constraint<FieldT>(this->pb, next_type, FieldT::one(), "next_type");
        generate_r1cs_equals_const_constraint<FieldT>(this->pb, arity, FieldT::one(), "arity");
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(is_base_case, cur_type, 0), "nonzero_cur_type_implies_base_case_0");
        generate_boolean_r1cs_constraint<FieldT>(this->pb, cur_type, "cur_type_boolean");
        generate_boolean_r1cs_constraint<FieldT>(this->pb, is_base_case, "is_base_case_boolean");
    }

    PROFILE_CONSTRAINTS(this->pb, "unpack messages")
    {
        unpack_next->generate_r1cs_constraints(true);
        unpack_cur->generate_r1cs_constraints(true);
    }

    // work-around for bad linear combination handling
    generate_r1cs_equals_const_constraint<FieldT>(this->pb, zero, FieldT::zero(), " zero");

    /* recall that Booleanity of PCD messages has already been enforced by the PCD machine, which is explains the absence of Booleanity checks */
    /*
      We need to perform the following checks:

      Always:
      next.root_initial = cur.root_initial
      next.pc_addr_init = cur.pc_addr_initial
      next.cpu_state_initial = cur.cpu_state_initial
    */
    PROFILE_CONSTRAINTS(this->pb, "copy root_initial")
    {
        copy_root_initial->generate_r1cs_constraints(false, false);
    }

    PROFILE_CONSTRAINTS(this->pb, "copy pc_addr_initial and cpu_state_initial")
    {
        copy_pc_addr_initial->generate_r1cs_constraints(false, false);
        copy_cpu_state_initial->generate_r1cs_constraints(false, false);
    }

    /*
      If is_base_case = 1: (base case)
      that cur.timestamp = 0, cur.cpu_state = 0, cur.pc_addr = 0, cur.has_accepted = 0
      that cur.root = cur.root_initial
    */
    pack_cur_timestamp->generate_r1cs_constraints(false);
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(is_base_case, packed_cur_timestamp, 0), "clear_ts_on_is_base_case");
    PROFILE_CONSTRAINTS(this->pb, "copy cur_cpu_state and prev_pc_addr")
    {
        initialize_cur_cpu_state->generate_r1cs_constraints(false, false);
        initialize_prev_pc_addr->generate_r1cs_constraints(false, false);
    }
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(is_base_case, cur->has_accepted, 0), "is_base_case_is_not_accepting");
    PROFILE_CONSTRAINTS(this->pb, "initialize root")
    {
        initialize_root->generate_r1cs_constraints(false, false);
    }

    /*
      If do_halt = 0: (regular case)
      that instruction fetch was correctly executed
      next.timestamp = cur.timestamp + 1
      that CPU accepted on (cur, next)
      that load-then-store was correctly handled
      that next.root = temp.root, next.cpu_state = temp.cpu_state, next.pc_addr = temp.pc_addr
    */
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, 1 - do_halt, is_not_halt_case), "is_not_halt_case");
    PROFILE_CONSTRAINTS(this->pb, "instruction fetch")
    {
        instruction_fetch_merkle_proof->generate_r1cs_constraints();
        instruction_fetch->generate_r1cs_constraints();
    }
    pack_next_timestamp->generate_r1cs_constraints(false);
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(is_not_halt_case, (packed_cur_timestamp + 1) - packed_next_timestamp, 0), "increment_timestamp");
    PROFILE_CONSTRAINTS(this->pb, "CPU checker")
    {
        cpu_checker->generate_r1cs_constraints();
    }
    PROFILE_CONSTRAINTS(this->pb, "load/store checker")
    {
        // See comment in merkle_tree_check_update_gadget::generate_r1cs_witness() for why we don't need to call store_merkle_proof->generate_r1cs_constraints()
        load_merkle_proof->generate_r1cs_constraints();
        load_store_checker->generate_r1cs_constraints();
    }

    PROFILE_CONSTRAINTS(this->pb, "copy temp_next_pc_addr and temp_next_cpu_state")
    {
        copy_temp_next_pc_addr->generate_r1cs_constraints(true, false);
        copy_temp_next_cpu_state->generate_r1cs_constraints(true, false);
    }

    /*
      If do_halt = 1: (final case)
      that cur.has_accepted = 1
      that next.root = 0, next.cpu_state = 0, next.pc_addr = 0
      that next.timestamp = cur.timestamp and next.has_accepted = cur.has_accepted
    */
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(do_halt, 1 - cur->has_accepted, 0), "final_case_must_be_accepting");

    PROFILE_CONSTRAINTS(this->pb, "clear next root")
    {
        clear_next_root->generate_r1cs_constraints(false, false);
    }

    PROFILE_CONSTRAINTS(this->pb, "clear next_pc_addr and next_cpu_state")
    {
        clear_next_pc_addr->generate_r1cs_constraints(false, false);
        clear_next_cpu_state->generate_r1cs_constraints(false, false);
    }

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(do_halt,  packed_cur_timestamp - packed_next_timestamp, 0), "equal_ts_on_halt");

    const size_t accounted = PRINT_CONSTRAINT_PROFILING();
    const size_t total = this->pb.num_constraints();
    print_indent(); printf("* Unaccounted constraints: %zu\n", total - accounted);
    print_indent(); printf("* Number of constraints in ram_compliance_predicate: %zu\n", total);
}

template<typename ramT>
void ram_compliance_predicate_handler<ramT>::generate_r1cs_witness(const r1cs_pcd_message<FieldT> &msg,
                                                                   const bool want_halt,
                                                                   delegated_ra_memory<CRH_with_bit_out_gadget<FieldT> > &mem,
                                                                   typename ram_input_tape<ramT>::const_iterator &aux_it,
                                                                   const typename ram_input_tape<ramT>::const_iterator &aux_end)
{
    assert(mem.num_addresses == 1ul << addr_size); // check value_size and num_addresses too

    this->pb.clear_values();

    this->pb.val(cur_type) = FieldT(msg.type);
    cur_packed.fill_with_field_elements(this->pb, msg.payload);
    unpack_cur->generate_r1cs_witness_from_packed();

    this->pb.val(next_type) = FieldT::one();
    this->pb.val(arity) = FieldT::one();
    this->pb.val(is_base_case) = this->pb.val(cur_type) == FieldT::zero() ? FieldT::one() : FieldT::zero();

    this->pb.val(zero) = FieldT::zero();
    /*
      Always:
      next.root_initial = cur.root_initial
      next.pc_addr_init = cur.pc_addr_initial
      next.cpu_state_initial = cur.cpu_state_initial
    */
    copy_root_initial->generate_r1cs_witness();
    for (size_t i = 0 ; i < next->root_initial.size(); ++i)
    {
        assert(this->pb.val(cur->root_initial[i]) == this->pb.val(next->root_initial[i]));
    }

    copy_pc_addr_initial->generate_r1cs_witness();
    copy_cpu_state_initial->generate_r1cs_witness();

    /*
      If is_base_case = 1: (base case)
      that cur.timestamp = 0, cur.cpu_state = 0, cur.pc_addr = 0, cur.has_accepted = 0
      that cur.root = cur.root_initial
    */
    const bool base_case = (msg.type == 0);
    this->pb.val(is_base_case) = base_case ? FieldT::one() : FieldT::zero();

    initialize_cur_cpu_state->generate_r1cs_witness();
    initialize_prev_pc_addr->generate_r1cs_witness();

    if (base_case)
    {
        this->pb.val(packed_cur_timestamp) = FieldT::zero();
        this->pb.val(cur->has_accepted) = FieldT::zero();
        pack_cur_timestamp->generate_r1cs_witness_from_packed();
    }
    else
    {
        pack_cur_timestamp->generate_r1cs_witness_from_bits();
    }

    initialize_root->generate_r1cs_witness();

    /*
      If do_halt = 0: (regular case)
      that instruction fetch was correctly executed
      next.timestamp = cur.timestamp + 1
      that CPU accepted on (cur, temp)
      that load-then-store was correctly handled
    */
    this->pb.val(do_halt) = want_halt ? FieldT::one() : FieldT::zero();
    this->pb.val(is_not_halt_case) = FieldT::one() - this->pb.val(do_halt);

    // that instruction fetch was correctly executed
    const size_t int_pc_addr = convert_bit_vector_to_field_element<FieldT>(cur->pc_addr.get_bits(this->pb)).as_ulong();
    const size_t int_pc_val = mem.get_value(int_pc_addr);
#ifdef DEBUG
    printf("pc_addr (in units) = %zu, pc_val = %zu (0x%08zx)\n", int_pc_addr, int_pc_val, int_pc_val);
#endif
    bit_vector pc_val_bv = int_list_to_bits({ int_pc_val }, value_size);
    std::reverse(pc_val_bv.begin(), pc_val_bv.end());

    prev_pc_val.fill_with_bits(this->pb, pc_val_bv);
    const merkle_authentication_path pc_path = mem.get_path(int_pc_addr);
    instruction_fetch_merkle_proof->generate_r1cs_witness(int_pc_addr, pc_path);
    instruction_fetch->generate_r1cs_witness();

    // next.timestamp = cur.timestamp + 1 (or cur.timestamp if do_halt)
    this->pb.val(packed_next_timestamp) = this->pb.val(packed_cur_timestamp) + (want_halt ? FieldT::zero() : FieldT::one());
    pack_next_timestamp->generate_r1cs_witness_from_packed();

    // that CPU accepted on (cur, temp)
    // Step 1: Get address and old witnesses for delegated memory.
    cpu_checker->generate_r1cs_witness_address();
    const size_t int_ls_addr = ls_addr.get_field_element_from_bits(this->pb).as_ulong();
    const size_t int_ls_prev_val = mem.get_value(int_ls_addr);
    const merkle_authentication_path prev_path = mem.get_path(int_ls_addr);
    ls_prev_val.fill_with_bits_of_ulong(this->pb, int_ls_prev_val);
    assert(ls_prev_val.get_field_element_from_bits(this->pb) == FieldT(int_ls_prev_val, true));
    // Step 2: Execute CPU checker and delegated memory
    cpu_checker->generate_r1cs_witness_other(aux_it, aux_end);
#ifdef DEBUG
    printf("Debugging information from transition function:\n");
    cpu_checker->dump();
#endif
    const size_t int_ls_next_val = ls_next_val.get_field_element_from_bits(this->pb).as_ulong();
    mem.set_value(int_ls_addr, int_ls_next_val);
#ifdef DEBUG
    printf("Memory location %zu changed from %zu (0x%08zx) to %zu (0x%08zx)\n", int_ls_addr, int_ls_prev_val, int_ls_prev_val, int_ls_next_val, int_ls_next_val);
#endif
    // Step 3: Satisfy load_store_checker
    load_merkle_proof->generate_r1cs_witness(int_ls_addr, prev_path);
    load_store_checker->generate_r1cs_witness();

    /*
      If do_halt = 1: (final case)
      that cur.has_accepted = 1
      that next.root = 0, next.cpu_state = 0, next.pc_addr = 0
      that next.timestamp = cur.timestamp and next.has_accepted = cur.has_accepted
    */

    // Order matters here: both witness maps touch next_root, but the
    // one that does not set values must be executed the last, so its
    // auxiliary variables are filled in correctly according to values
    // actually set by the other witness map.
    if (!want_halt)
    {
        copy_temp_next_pc_addr->generate_r1cs_witness();
        copy_temp_next_cpu_state->generate_r1cs_witness();

        clear_next_root->generate_r1cs_witness();
        clear_next_pc_addr->generate_r1cs_witness();
        clear_next_cpu_state->generate_r1cs_witness();
    }
    else
    {
        clear_next_root->generate_r1cs_witness();
        clear_next_pc_addr->generate_r1cs_witness();
        clear_next_cpu_state->generate_r1cs_witness();

        copy_temp_next_pc_addr->generate_r1cs_witness();
        copy_temp_next_cpu_state->generate_r1cs_witness();
    }

#ifdef DEBUG
    printf("next.has_accepted: ");
    this->pb.val(next->has_accepted).print();
#endif

    unpack_next->generate_r1cs_witness_from_bits();
}

template<typename ramT>
size_t ram_compliance_predicate_handler<ramT>::message_size(const ram_architecture_params<ramT> &ap)
{
    const size_t chunk_size = FieldT::size_in_bits() - 1;
    const size_t message_length_in_bits = ram_compliance_message_vars<ramT>::size_in_bits(ap);
    const size_t message_length = div_ceil(message_length_in_bits, chunk_size);
    return message_length;
}

template<typename ramT>
ram_message<ramT> ram_compliance_predicate_handler<ramT>::get_base_case_message(const ram_architecture_params<ramT> &ap,
                                                                                const ram_boot_trace<ramT> &primary_input)
{
    enter_block("Call to ram_compliance_predicate_handler::get_base_case_message");
    const size_t num_addresses = 1ul << ap.address_size();
    const size_t value_size = ap.value_size();
    delegated_ra_memory<CRH_with_bit_out_gadget<FieldT> > mem(num_addresses, value_size, primary_input.as_memory_contents());

    ram_protoboard<ramT> pb(ap);
    ram_compliance_message_vars<ramT> msg(pb, "msg");
    msg.root_initial.fill_with_bits(pb, mem.get_root());
    msg.root.fill_with_bits(pb, mem.get_root());

    const size_t initial_pc_addr = ap.initial_pc_addr();

    msg.pc_addr_initial.fill_with_bits(pb, convert_field_element_to_bit_vector<FieldT>(FieldT(initial_pc_addr), ap.address_size()));
    msg.pc_addr.fill_with_bits(pb, convert_field_element_to_bit_vector<FieldT>(FieldT(initial_pc_addr), ap.address_size()));

    ram_cpu_state<ramT> initial_state_val;

    ram_message<ramT> result;
    result.type = 0;
    result.payload = pack_bit_vector_into_field_element_vector<FieldT>(msg.all_vars.get_bits(pb));

    leave_block("Call to ram_compliance_predicate_handler::get_base_case_message");
    return result;
}

template<typename ramT>
ram_message<ramT> ram_compliance_predicate_handler<ramT>::get_final_case_msg(const ram_architecture_params<ramT> &ap,
                                                                             const ram_boot_trace<ramT> &primary_input,
                                                                             const size_t time_bound)
{
    enter_block("Call to ram_compliance_predicate_handler::get_final_case_msg");
    const size_t num_addresses = 1ul << ap.address_size();
    const size_t value_size = ap.value_size();
    delegated_ra_memory<CRH_with_bit_out_gadget<FieldT> > mem(num_addresses, value_size, primary_input.as_memory_contents());

    ram_protoboard<ramT> pb(ap);
    ram_compliance_message_vars<ramT> msg(pb, "msg");
    msg.root_initial.fill_with_bits(pb, mem.get_root());

    const size_t initial_pc_addr = ap.initial_pc_addr();

    msg.pc_addr_initial.fill_with_bits(pb, convert_field_element_to_bit_vector<FieldT>(FieldT(initial_pc_addr), ap.address_size()));

    ram_cpu_state<ramT> initial_state_val;

    bit_vector ts_bits = convert_field_element_to_bit_vector<FieldT>(FieldT(time_bound), ramT::timestamp_length);
    msg.timestamp.fill_with_bits(pb, ts_bits);
    pb.val(msg.has_accepted) = FieldT::one();

    ram_message<ramT> result;
    result.type = 1;
    result.payload = pack_bit_vector_into_field_element_vector<FieldT>(msg.all_vars.get_bits(pb));

    leave_block("Call to ram_compliance_predicate_handler::get_final_case_msg");

    return result;
}

} // libsnark

#endif // RAM_COMPLIANCE_PREDICATE_TCC_
