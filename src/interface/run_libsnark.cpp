/*
 * run_libsnark.cpp
 *
 *      Author: Ahmed Kosba
 */

#include "../interface/CircuitReader.hpp"
#include "gadgetlib2/examples/simple_example.hpp"
#include "gadgetlib2/adapters.hpp"
#include "gadgetlib2/gadget.hpp"
#include "gadgetlib2/integration.hpp"

int main(int argc, char **argv) {

	start_profiling();
	GadgetLibAdapter::resetVariableIndex();
	gadgetlib2::initPublicParamsFromDefaultPp();
	gadgetlib2::GadgetLibAdapter::resetVariableIndex();
	ProtoboardPtr pb = gadgetlib2::Protoboard::create(gadgetlib2::R1P);

	CircuitReader reader(argv[1], argv[2], pb);
	r1cs_constraint_system<FieldT> cs = get_constraint_system_from_gadgetlib2(
			*pb);
	const r1cs_variable_assignment<FieldT> full_assignment =
			get_variable_assignment_from_gadgetlib2(*pb);
	cs.primary_input_size = reader.getNumInputs() + reader.getNumOutputs();
	cs.auxiliary_input_size = full_assignment.size() - cs.num_inputs();

	// extract primary and auxiliary input
	const r1cs_primary_input<FieldT> primary_input(full_assignment.begin(),
			full_assignment.begin() + cs.num_inputs());
	const r1cs_auxiliary_input<FieldT> auxiliary_input(
			full_assignment.begin() + cs.num_inputs(), full_assignment.end());

	vector<FieldT>::const_iterator first = full_assignment.begin();
	vector<FieldT>::const_iterator last = full_assignment.begin()
			+ reader.getNumInputs() + reader.getNumOutputs();
	vector<FieldT> newVec(first, last);
	cout << "Printing client I/O assignment:: " << endl;
	for (int i = 0; i < reader.getNumInputs() + reader.getNumOutputs(); i++) {
		cout << primary_input[i] << endl;
	}
	cout << "Length " << cs.num_inputs() << endl;
	assert(cs.is_valid());
	assert(cs.is_satisfied(primary_input, auxiliary_input));
	r1cs_example<FieldT> example(cs, primary_input, auxiliary_input);
	const bool test_serialization = false;
	const bool bit = libsnark::run_r1cs_ppzksnark<libsnark::default_ec_pp>(
			example, test_serialization);
	assert(bit);
	return 0;
}

