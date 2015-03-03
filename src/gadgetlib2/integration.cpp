/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "gadgetlib2/adapters.hpp"
#include "gadgetlib2/integration.hpp"

namespace libsnark {

linear_combination<Fr<default_ec_pp> > convert_gadgetlib2_linear_combination(const gadgetlib2::GadgetLibAdapter::linear_combination_t &lc)
{
    typedef Fr<default_ec_pp> FieldT;
    typedef gadgetlib2::GadgetLibAdapter GLA;

    linear_combination<FieldT> result = lc.second * variable<FieldT>(0);
    for (const GLA::linear_term_t &lt : lc.first)
    {
        result = result + lt.second * variable<FieldT>(lt.first+1);
    }

    return result;
}

r1cs_constraint_system<Fr<default_ec_pp> > get_constraint_system_from_gadgetlib2(const gadgetlib2::Protoboard &pb)
{
    typedef Fr<default_ec_pp> FieldT;
    typedef gadgetlib2::GadgetLibAdapter GLA;

    r1cs_constraint_system<FieldT> result;
    const GLA adapter;

    GLA::protoboard_t converted_pb = adapter.convert(pb);
    for (const GLA::constraint_t &constr : converted_pb.first)
    {
        result.constraints.emplace_back(r1cs_constraint<FieldT>(convert_gadgetlib2_linear_combination(std::get<0>(constr)),
                                                                convert_gadgetlib2_linear_combination(std::get<1>(constr)),
                                                                convert_gadgetlib2_linear_combination(std::get<2>(constr))));
    }
    //The numbers of variables is the highest index created.
    //TODO: If there are multiple protoboard, or variables not assigned to a protoboard, then getNextFreeIndex() is *not* the number of variables! See also in get_variable_assignment_from_gadgetlib2.

    // TODO: fix integration
    // result.num_variables = GLA::getNextFreeIndex();
    return result;
}

r1cs_variable_assignment<Fr<default_ec_pp> > get_variable_assignment_from_gadgetlib2(const gadgetlib2::Protoboard &pb)
{
    typedef Fr<default_ec_pp> FieldT;
    typedef gadgetlib2::GadgetLibAdapter GLA;

    const size_t num_vars = pb.numVars();

    r1cs_variable_assignment<FieldT> result(num_vars, FieldT::zero());
    const GLA adapter;

    GLA::protoboard_t converted_pb = adapter.convert(pb);
    for (size_t i = 0; i < num_vars; ++i)
    {
        auto it = converted_pb.second.find(i);
        if (it != converted_pb.second.end())
        {
            result[i] = it->second;
        }
    }

    return result;
}

}
