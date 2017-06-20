/**
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/
#include "../../../../../common/default_types/r1cs_ppzkpcd_pp.hpp"
#include "../r1cs_sp_ppzkpcd.hpp"
#include "../examples/run_r1cs_sp_ppzkpcd.hpp"

using namespace libsnark;

template<typename PCD_ppT>
void profile_tally(const size_t arity, const size_t max_layer)
{
    const size_t wordsize = 32;
    const bool test_serialization = true;
    const bool bit = run_r1cs_sp_ppzkpcd_tally_example<PCD_ppT>(wordsize, arity, max_layer, test_serialization);
    assert(bit);
}

int main(void)
{
    typedef default_r1cs_ppzkpcd_pp PCD_pp;

    libff::start_profiling();
    PCD_pp::init_public_params();

    const size_t arity = 2;
    const size_t max_layer = 2;

    profile_tally<PCD_pp>(arity, max_layer);
}
