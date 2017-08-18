/** @file
 *****************************************************************************
 Test program that exercises the ppzkSNARK (first generator, then
 prover, then verifier) on a synthetic R1CS instance.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/
#include <cassert>
#include <cstdio>

#include "common/default_types/r1cs_ppzksnark_pp.hpp"
#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "relations/constraint_satisfaction_problems/r1cs/examples/r1cs_examples.hpp"
#include "zk_proof_systems/ppzksnark/r1cs_ppzksnark/examples/run_r1cs_ppzksnark.hpp"

using namespace libsnark;

//****Ariel code-testing batch verifier****///
template<typename ppT>
void test_r1cs_ppzksnark_batch_verifier(size_t num_constraints,
                         size_t input_size)
{
    //silly test that takes the same proof/primary input multiple times
    enter_block("Test R1CS ppzkSNARK batch verifier");
    r1cs_example<Fr<ppT> > example = generate_r1cs_example_with_binary_input<Fr<ppT> >(num_constraints, input_size);
    r1cs_ppzksnark_keypair<ppT> keypair = r1cs_ppzksnark_generator<ppT>(example.constraint_system);
    r1cs_ppzksnark_proof<ppT> proof = r1cs_ppzksnark_prover<ppT>(keypair.pk, example.primary_input, example.auxiliary_input);
    
    enter_block("In test_r1cs_ppzksnark_batch_verifier after generating example and proof");
    
    r1cs_ppzksnark_processed_batch_verification_key<ppT> pvk = r1cs_ppzksnark_batch_verifier_process_vk<ppT>(keypair.vk);
    
    batch_verification_accumulator<ppT> acc;
    const bool test_serialization = true;
    for(auto i=0; i<10;i++)
    {
        //r1cs_example<Fr<ppT> > example = generate_r1cs_example_with_binary_input<Fr<ppT> >(num_constraints, input_size);
        add_proof_in_batch_verifier_test<ppT>(acc,proof,keypair.vk,example.primary_input);
        
        
    }

    const bool bit = r1cs_ppzksnark_batch_verifier<ppT>(pvk, acc,example.primary_input,proof);
    assert(bit);
    leave_block("In test_r1cs_ppzksnark_batch_verifier after generating example and proof");
    
    leave_block("Test R1CS ppzkSNARK batch verifier");
}


///****End Ariel code ******//////


template<typename ppT>
void test_r1cs_ppzksnark(size_t num_constraints,
                         size_t input_size)
{
    print_header("(enter) Test R1CS ppzkSNARK");

    const bool test_serialization = true;
    r1cs_example<Fr<ppT> > example = generate_r1cs_example_with_binary_input<Fr<ppT> >(num_constraints, input_size);
    const bool bit = run_r1cs_ppzksnark<ppT>(example, test_serialization);
    assert(bit);

    print_header("(leave) Test R1CS ppzkSNARK");
}

int main()
{
    default_r1cs_ppzksnark_pp::init_public_params();
    start_profiling();

    //test_r1cs_ppzksnark<default_r1cs_ppzksnark_pp>(1000, 100);
    //ariel change
    test_r1cs_ppzksnark_batch_verifier<default_r1cs_ppzksnark_pp>(1000,100);
}
