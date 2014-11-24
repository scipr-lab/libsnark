/** @file
 *****************************************************************************
 Profiling program that exercises the ppzkSNARK (first generator, then prover,
 then verifier) on a synthetic USCS instance.

 The command

     $ src/uscs_ppzksnark/examples/profile_uscs_ppzksnark 1000 10

 exercises the ppzkSNARK (first generator, then prover, then verifier) on an USCS instance with 1000 equations and an input consisting of 10 bits.

 (If you get the error `zmInit ERR:can't protect`, see the discussion [above](#elliptic-curve-choices).)

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <algorithm>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <vector>

#include "common/types.hpp"
#include "common/profiling.hpp"
#include "common/utils.hpp"
#include "relations/constraint_satisfaction_problems/uscs/examples/uscs_examples.hpp"
#include "zk_proof_systems/ppzksnark/uscs_ppzksnark/examples/run_uscs_ppzksnark.hpp"

using namespace libsnark;

int main(int argc, const char * argv[])
{
    init_public_params<default_pp>();
    start_profiling();

    if (argc == 2 && strcmp(argv[1], "-v") == 0)
    {
        print_compilation_info();
        return 0;
    }

    if (argc != 3)
    {
        printf("usage: %s num_constraints input_size\n", argv[0]);
        return 1;
    }

    const int num_constraints = atoi(argv[1]);
    const int input_size = atoi(argv[2]);

    enter_block("Generate USCS example");
    uscs_example<Fr<default_pp> > example = generate_uscs_example_with_field_input<Fr<default_pp> >(num_constraints, input_size);
    leave_block("Generate USCS example");

    print_header("(enter) Profile USCS ppzkSNARK");
    const bool test_serialization = false;
    run_uscs_ppzksnark<default_pp>(example, test_serialization);
    print_header("(leave) Profile USCS ppzkSNARK");
}
