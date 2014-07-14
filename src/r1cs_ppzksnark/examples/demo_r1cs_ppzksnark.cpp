/** @file
 *****************************************************************************
 Profiling program that exercises the ppzkSNARK (first generator, then prover,
 then verifier) on a synthetic R1CS instance.

 The command

     $ src/r1cs_ppzksnark/examples/profile_r1cs_ppzksnark 1000 10 Fr

 exercises the ppzkSNARK (first generator, then prover, then verifier) on an R1CS instance with 1000 equations and an input consisting of 10 field elements.

 (If you get the error `zmInit ERR:can't protect`, see the discussion [above](#elliptic-curve-choices).)

 The command

     $ src/r1cs_ppzksnark/examples/profile_r1cs_ppzksnark 1000 10 bytes

 does the same but now the input consists of 10 bytes.

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
#include "r1cs/examples/r1cs_examples.hpp"
#include "r1cs_ppzksnark/examples/run_r1cs_ppzksnark.hpp"

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

    if (argc != 3 && argc != 4)
    {
        printf("usage: %s num_constraints input_size [Fr|bytes]\n", argv[0]);
        return 1;
    }
    const int num_constraints = atoi(argv[1]);
    int input_size = atoi(argv[2]);
    if (argc == 4)
    {
        assert(strcmp(argv[3], "Fr") == 0 || strcmp(argv[3], "bytes") == 0);
        if (strcmp(argv[3], "bytes") == 0)
        {
            input_size = div_ceil(8 * input_size, Fr<default_pp>::num_bits - 1);
        }
    }

    enter_block("Generate R1CS example");
    r1cs_example<Fr<default_pp> > example = gen_r1cs_example_Fr_input<Fr<default_pp> >(num_constraints, input_size);
    leave_block("Generate R1CS example");

    print_header("(enter) Profile R1CS ppzkSNARK");
    const bool test_serialization = false;
    run_r1cs_ppzksnark<default_pp>(example, test_serialization);
    print_header("(leave) Profile R1CS ppzkSNARK");
}
