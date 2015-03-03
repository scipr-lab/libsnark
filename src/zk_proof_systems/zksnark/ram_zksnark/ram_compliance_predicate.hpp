/** @file
 *****************************************************************************

 Declaration of interfaces for a compliance predicate for RAM.

 The implementation follows, extends, and optimizes the approach described
 in \[BCTV14].

 Essentially, the RAM's CPU, which is expressed as an R1CS constraint system,
 is augmented to obtain another R1CS constraint ssytem that implements a RAM
 compliance predicate. This predicate is responsible for checking:
 (1) transitions from a CPU state to the next;
 (2) correct load/stores; and
 (3) corner cases such as the first and last steps of the machine.
 The first can be done by suitably embedding the RAM's CPU in the constraint
 system. The second can be done by verifying authentication paths for the values
 of memory. The third mostly consists of bookkeepng (with some subtleties arising
 from the need to not break zero knowledge).

 The laying out of R1CS constraints is done via gadgetlib1 (a minimalistic
 library for writing R1CS constraint systems).

 References:

 \[BCTV14]:
 "Scalable Zero Knowledge via Cycles of Elliptic Curves",
 Eli Ben-Sasson, Alessandro Chiesa, Eran Tromer, Madars Virza,
 CRYPTO 2014,
 <http://eprint.iacr.org/2014/595>

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef RAM_COMPLIANCE_PREDICATE_HPP_
#define RAM_COMPLIANCE_PREDICATE_HPP_

#include "relations/ram_computations/memory/delegated_ra_memory.hpp"
#include "gadgetlib1/gadgets/delegated_ra_memory/memory_load_gadget.hpp"
#include "gadgetlib1/gadgets/delegated_ra_memory/memory_load_store_gadget.hpp"
#include "relations/ram_computations/rams/ram_params.hpp"
#include "zk_proof_systems/pcd/r1cs_pcd/compliance_predicate/compliance_predicate.hpp"
#include "zk_proof_systems/pcd/r1cs_pcd/compliance_predicate/cp_handler.hpp"

namespace libsnark {

/*
  XXX
  message (FieldT values), message_vars (variables).
  could:
  - have all_vars in message_vars for serialization
  - print() method of message would construct message_vars, serialize into and then semantically access
 */

/**
 * A RAM message specializes the generic PCD message, in order to
 * obtain a more user-friendly print method.
 */
template<typename ramT>
class ram_message : public r1cs_pcd_message<ram_base_field<ramT> > {
public:
    void print(const ram_architecture_params<ramT> &ap) const;
};

/**
 * Forward declaration. (The implementation is in the tcc file.)
 */
template<typename ramT>
class ram_compliance_message_vars;

/**
 * A RAM compliance predicate.
 */
template<typename ramT>
class ram_compliance_predicate_handler : public compliance_predicate_handler<ram_base_field<ramT>, ram_protoboard<ramT> > {
protected:

    ram_architecture_params<ramT> ap;

public:

    typedef ram_base_field<ramT> FieldT;

    pb_variable<FieldT> next_type;
    pb_variable_array<FieldT> next_packed;
    pb_variable<FieldT> arity;
    pb_variable<FieldT> cur_type;
    pb_variable_array<FieldT> cur_packed;

    std::shared_ptr<ram_compliance_message_vars<ramT> > next;
    std::shared_ptr<ram_compliance_message_vars<ramT> > cur;

    std::shared_ptr<multipacking_gadget<FieldT> > unpack_next;
    std::shared_ptr<multipacking_gadget<FieldT> > unpack_cur;

private:

    pb_variable<FieldT> zero; // TODO: promote linear combinations to first class objects
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > copy_root_initial;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > copy_pc_addr_initial;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > copy_cpu_state_initial;

    pb_variable<FieldT> is_base_case;
    pb_variable<FieldT> is_not_halt_case;

    pb_variable<FieldT> packed_cur_timestamp;
    std::shared_ptr<packing_gadget<FieldT> > pack_cur_timestamp;
    pb_variable<FieldT> packed_next_timestamp;
    std::shared_ptr<packing_gadget<FieldT> > pack_next_timestamp;

    pb_variable_array<FieldT> zero_cpu_state;
    pb_variable_array<FieldT> zero_pc_addr;
    pb_variable_array<FieldT> zero_root;

    std::shared_ptr<bit_vector_copy_gadget<FieldT> > initialize_cur_cpu_state;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > initialize_prev_pc_addr;

    std::shared_ptr<bit_vector_copy_gadget<FieldT> > initialize_root;

    pb_variable_array<FieldT> prev_pc_val;
    std::shared_ptr<digest_variable<FieldT> > prev_pc_val_digest;
    std::shared_ptr<digest_variable<FieldT> > cur_root_digest;
    std::shared_ptr<memory_load_gadget<FieldT> > instruction_fetch;

    std::shared_ptr<digest_variable<FieldT> > temp_next_root_digest;

    pb_variable_array<FieldT> ls_addr;
    pb_variable_array<FieldT> ls_prev_val;
    pb_variable_array<FieldT> ls_next_val;
    std::shared_ptr<digest_variable<FieldT> > ls_prev_val_digest;
    std::shared_ptr<digest_variable<FieldT> > ls_next_val_digest;
    std::shared_ptr<memory_load_store_gadget<FieldT> > load_store_checker;

    pb_variable_array<FieldT> temp_next_root;
    pb_variable_array<FieldT> temp_next_pc_addr;
    pb_variable_array<FieldT> temp_next_cpu_state;
    pb_variable<FieldT> temp_next_has_accepted;
    std::shared_ptr<ram_cpu_checker<ramT> > cpu_checker;

    pb_variable<FieldT> do_halt;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > clear_next_root;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > clear_next_pc_addr;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > clear_next_cpu_state;

    std::shared_ptr<bit_vector_copy_gadget<FieldT> > copy_temp_next_root;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > copy_temp_next_pc_addr;
    std::shared_ptr<bit_vector_copy_gadget<FieldT> > copy_temp_next_cpu_state;

public:
    const size_t addr_size;
    const size_t value_size;
    const size_t digest_size;

    size_t message_length;

    ram_compliance_predicate_handler(const ram_architecture_params<ramT> &ap);

    void generate_r1cs_constraints();
    void generate_r1cs_witness(const r1cs_pcd_message<FieldT> &msg,
                               const bool halt_case,
                               delegated_ra_memory<CRH_with_bit_out_gadget<FieldT> > &mem,
                               typename ram_input_tape<ramT>::const_iterator &aux_it,
                               const typename ram_input_tape<ramT>::const_iterator &aux_end);

    static size_t message_size(const ram_architecture_params<ramT> &ap);
    static ram_message<ramT> get_base_case_message(const ram_architecture_params<ramT> &ap,
                                                   const ram_boot_trace<ramT> &primary_input);
    static ram_message<ramT> get_final_case_msg(const ram_architecture_params<ramT> &ap,
                                                const ram_boot_trace<ramT> &primary_input,
                                                const size_t time_bound);
};

} // libsnark

#include "zk_proof_systems/zksnark/ram_zksnark/ram_compliance_predicate.tcc"

#endif // RAM_COMPLIANCE_PREDICATE_HPP_
