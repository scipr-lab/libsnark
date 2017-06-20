/** @file
 *****************************************************************************

 Functions to profile the algorithms that route on Benes and AS-Waksman networks.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <algorithm>

#include <libff/common/profiling.hpp>
#include "../as_waksman_routing_algorithm.hpp"
#include "../benes_routing_algorithm.hpp"

using namespace libsnark;

void profile_benes_algorithm(const size_t n)
{
    printf("* Size: %zu\n", n);

    assert(n == 1ul<<libff::log2(n));

    libff::enter_block("Generate permutation");
    integer_permutation permutation(n);
    permutation.random_shuffle();
    libff::leave_block("Generate permutation");

    libff::enter_block("Generate Benes routing assignment");
    const benes_routing routing = get_benes_routing(permutation);
    libff::leave_block("Generate Benes routing assignment");
}

void profile_as_waksman_algorithm(const size_t n)
{
    printf("* Size: %zu\n", n);

    libff::enter_block("Generate permutation");
    integer_permutation permutation(n);
    permutation.random_shuffle();
    libff::leave_block("Generate permutation");

    libff::enter_block("Generate AS-Waksman routing assignment");
    const as_waksman_routing routing = get_as_waksman_routing(permutation);
    libff::leave_block("Generate AS-Waksman routing assignment");
}

int main()
{
    libff::start_profiling();

    for (size_t n = 1ul<<10; n <= 1ul<<20; n <<= 1)
    {
        profile_benes_algorithm(n);
    }

    for (size_t n = 1ul<<10; n <= 1ul<<20; n <<= 1)
    {
        profile_as_waksman_algorithm(n);
    }
}
