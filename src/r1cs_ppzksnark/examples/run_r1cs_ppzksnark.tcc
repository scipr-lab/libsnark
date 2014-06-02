/** @file
 ********************************************************************************
 Implementation of high-level example code that runs the ppzkSNARK
 (generator, prover, and verifier) for a given R1CS example.
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef RUN_R1CS_PPZKSNARK_TCC_
#define RUN_R1CS_PPZKSNARK_TCC_

#include "r1cs_ppzksnark/r1cs_ppzksnark.hpp"

#include <sstream>

#include "common/profiling.hpp"

/* 
    This is an example of all stages of running a SNARK.
    Note that in a real life scenario, we would have 3 different entities (mangled into one in this
    test):
    (1) Generator: Creates the constraints and runs SNARK generator to create a proving key and a
    verification key.
    (2) Prover  : Creates the witness and runs the SNARK prover using the proving key.
    (3) Verifier: Runs the SNARK verifier using the verification key and the proof.
*/

namespace libsnark {

template<typename ppT>
bool run_r1cs_ppzksnark(const r1cs_example<Fr<ppT> > &example,
                        const bool test_serialization)
{
    enter_block("Call to run_r1cs_ppzksnark");

    print_header("R1CS ppzkSNARK Generator");
    r1cs_ppzksnark_keypair<ppT> keypair = r1cs_ppzksnark_generator<ppT>(example.constraint_system);
    printf("\n"); print_indent(); print_mem("after generator");

    print_header("Preprocess verification key");
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(keypair.vk);

    if (test_serialization)
    {
        enter_block("Test serialization of keys");
        keypair.pk = reserialize<r1cs_ppzksnark_proving_key<ppT> >(keypair.pk);
        keypair.vk = reserialize<r1cs_ppzksnark_verification_key<ppT> >(keypair.vk);
        pvk = reserialize<r1cs_ppzksnark_processed_verification_key<ppT> >(pvk);
        leave_block("Test serialization of keys");
    }

    print_header("R1CS ppzkSNARK Prover");
    r1cs_ppzksnark_proof<ppT> proof = r1cs_ppzksnark_prover<ppT>(keypair.pk, example.witness);
    printf("\n"); print_indent(); print_mem("after prover");

    if (test_serialization)
    {
        enter_block("Test serialization of proof");
        proof = reserialize<r1cs_ppzksnark_proof<ppT> >(proof);
        leave_block("Test serialization of proof");
    }

    print_header("R1CS ppzkSNARK Verifier");
    bool ans = r1cs_ppzksnark_verifier_strong_IC<ppT>(keypair.vk, example.input, proof);
    printf("\n"); print_indent(); print_mem("after verifier");
    printf("* The verification result is: %s\n", (ans ? "PASS" : "FAIL"));

    print_header("R1CS ppzkSNARK Online Verifier");
    bool ans2 = r1cs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, example.input, proof);
    assert(ans == ans2);

    leave_block("Call to run_r1cs_ppzksnark");

    return ans;
}

} // libsnark
#endif // RUN_R1CS_PPZKSNARK_TCC_
