/*
 * CircuitReader.cpp
 *
 *      Author: Ahmed Kosba
 */

#include "../interface/CircuitReader.hpp"

CircuitReader::CircuitReader(char* arithFilepath, char* inputsFilepath,
		ProtoboardPtr pb) {

	this->pb = pb;
	numWires = 0;
	numInputs = numNizkInputs = numOutputs = 0;

	parseAndEval(arithFilepath, inputsFilepath);
	constructCircuit(arithFilepath);
	mapValuesToProtoboard();

	wireLinearCombinations.clear();
	wireValues.clear();
	variables.clear();
	variableMap.clear();
	zeropMap.clear();
	zeroPwires.clear();
}

void CircuitReader::parseAndEval(char* arithFilepath, char* inputsFilepath) {

	enter_block("Parsing and Evaluating the circuit");

	ifstream arithfs(arithFilepath, ifstream::in);
	ifstream inputfs(inputsFilepath, ifstream::in);
	string line;

	if (!arithfs.good()) {
		printf("Unable to open circuit file %s \n", arithFilepath);
		exit(-1);
	}

	getline(arithfs, line);
	int ret = sscanf(line.c_str(), "total %u", &numWires);

	if (ret != 1) {
		printf("File Format Does not Match\n");
		exit(-1);
	}

	wireValues.resize(numWires);
	wireUseCounters.resize(numWires);
	wireLinearCombinations.resize(numWires);

	if (!inputfs.good()) {
		printf("Unable to open input file %s \n", inputsFilepath);
		exit(-1);
	} else {
		char* inputStr;
		while (getline(inputfs, line)) {
			if (line.length() == 0) {
				continue;
			}
			Wire wireId;
			inputStr = new char[line.size()];
			if (2 == sscanf(line.c_str(), "%u %s", &wireId, inputStr)) {
				wireValues[wireId] = readFieldElementFromHex(inputStr);
			} else {
				printf("Error in Input\n");
				exit(-1);
			}
			delete[] inputStr;
		}
		inputfs.close();
	}

	char type[200];
	char* inputStr;
	char* outputStr;
	unsigned int numGateInputs, numGateOutputs;

	Wire wireId;

	FieldT oneElement = FieldT::one();
	FieldT zeroElement = FieldT::zero();
	FieldT negOneElement = FieldT(-1);

	long long evalTime;
	long long begin, end;
	evalTime = 0;

	// Parse the circuit: few lines were imported from Pinocchio's code.

	while (getline(arithfs, line)) {
		if (line.length() == 0) {
			continue;
		}
		inputStr = new char[line.size()];
		outputStr = new char[line.size()];

		if (line[0] == '#') {
			continue;
		} else if (1 == sscanf(line.c_str(), "input %u", &wireId)) {
			numInputs++;
			inputWireIds.push_back(wireId);
		} else if (1 == sscanf(line.c_str(), "nizkinput %u", &wireId)) {
			numNizkInputs++;
			nizkWireIds.push_back(wireId);
		} else if (1 == sscanf(line.c_str(), "output %u", &wireId)) {
			numOutputs++;
			outputWireIds.push_back(wireId);
			wireUseCounters[wireId]++;
		} else if (5
				== sscanf(line.c_str(), "%s in %u <%[^>]> out %u <%[^>]>", type,
						&numGateInputs, inputStr, &numGateOutputs, outputStr)) {

			istringstream iss_i(inputStr, istringstream::in);
			std::vector<FieldT> inValues;
			std::vector<Wire> outWires;
			Wire inWireId;
			while (iss_i >> inWireId) {
				wireUseCounters[inWireId]++;
				inValues.push_back(wireValues[inWireId]);
			}
			readIds(outputStr, outWires);

			short opcode;
			FieldT constant;
			if (strcmp(type, "add") == 0) {
				opcode = ADD_OPCODE;
			} else if (strcmp(type, "mul") == 0) {
				opcode = MUL_OPCODE;
			} else if (strcmp(type, "xor") == 0) {
				opcode = XOR_OPCODE;
			} else if (strcmp(type, "or") == 0) {
				opcode = OR_OPCODE;
			} else if (strcmp(type, "assert") == 0) {
				wireUseCounters[outWires[0]]++;
				opcode = CONSTRAINT_OPCODE;
			} else if (strcmp(type, "pack") == 0) {
				opcode = PACK_OPCODE;
			} else if (strcmp(type, "zerop") == 0) {
				opcode = NONZEROCHECK_OPCODE;
			} else if (strcmp(type, "split") == 0) {
				opcode = SPLIT_OPCODE;
			} else if (strstr(type, "const-mul-neg-")) {
				opcode = MULCONST_OPCODE;
				char* constStr = type + sizeof("const-mul-neg-") - 1;
				constant = readFieldElementFromHex(constStr) * negOneElement;
			} else if (strstr(type, "const-mul-")) {
				opcode = MULCONST_OPCODE;
				char* constStr = type + sizeof("const-mul-") - 1;
				constant = readFieldElementFromHex(constStr);
			} else {
				printf("Error: unrecognized line: %s\n", line.c_str());
				assert(0);
			}

			begin = get_nsec_time();
			if (opcode == ADD_OPCODE) {
				FieldT sum;
				for (auto &v : inValues)
					sum += v;
				wireValues[outWires[0]] = sum;
			} else if (opcode == MUL_OPCODE) {
				wireValues[outWires[0]] = inValues[0] * inValues[1];
			} else if (opcode == XOR_OPCODE) {
				wireValues[outWires[0]] =
						(inValues[0] == inValues[1]) ? zeroElement : oneElement;
			} else if (opcode == OR_OPCODE) {
				wireValues[outWires[0]] =
						(inValues[0] == zeroElement
								&& inValues[1] == zeroElement) ?
								zeroElement : oneElement;
			} else if (opcode == NONZEROCHECK_OPCODE) {
				wireValues[outWires[1]] =
						(inValues[0] == zeroElement) ? zeroElement : oneElement;
			} else if (opcode == PACK_OPCODE) {
				FieldT sum, coeff;
				FieldT two = oneElement;
				for (auto &v : inValues) {
					sum += two * v;
					two += two;
				}
				wireValues[outWires[0]] = sum;
			} else if (opcode == SPLIT_OPCODE) {
				int size = outWires.size();
				FElem inVal = inValues[0];
				for (int i = 0; i < size; i++) {
					wireValues[outWires[i]] = inVal.getBit(i, R1P);
				}
			} else if (opcode == MULCONST_OPCODE) {
				wireValues[outWires[0]] = constant * inValues[0];
			}
			end = get_nsec_time();
			evalTime += (end - begin);
		} else {
			printf("Error: unrecognized line: %s\n", line.c_str());
			assert(0);
		}
		delete[] inputStr;
		delete[] outputStr;
	}
	arithfs.close();

	end = clock();
	printf("\t Evaluation Done in %lf seconds \n", (double) (evalTime) * 1e-9);
	leave_block("Parsing and Evaluating the circuit");
}

void CircuitReader::constructCircuit(char* arithFilepath) {

	struct proc_t usage1, usage2;
	cout << "Translating Constraints ... " << endl;
	look_up_our_self(&usage1);
	unsigned int i;

	currentVariableIdx = currentLinearCombinationIdx = 0;
	for (i = 0; i < numInputs; i++) {
		variables.push_back(make_shared<Variable>("input"));
		variableMap[inputWireIds[i]] = currentVariableIdx;
		currentVariableIdx++;
	}
	for (i = 0; i < numOutputs; i++) {
		variables.push_back(make_shared<Variable>("output"));
		variableMap[outputWireIds[i]] = currentVariableIdx;
		currentVariableIdx++;
	}
	for (i = 0; i < numNizkInputs; i++) {
		variables.push_back(make_shared<Variable>("nizk input"));
		variableMap[nizkWireIds[i]] = currentVariableIdx;
		currentVariableIdx++;
	}

	char type[200];
	char* inputStr;
	char* outputStr;
	string line;
	unsigned int numGateInputs, numGateOutputs;

	ifstream ifs2(arithFilepath, ifstream::in);

	if (!ifs2.good()) {
		printf("Unable to open circuit file:\n");
		exit(5);
	}

	// Parse the circuit: few lines were imported from Pinocchio's code.

	getline(ifs2, line);
	sscanf(line.c_str(), "total %d", &numWires);

	int lineCount = 0;
	while (getline(ifs2, line)) {
		lineCount++;
//		if (lineCount % 100000 == 0) {
//			printf("At Line:: %d\n", lineCount);
//		}

		if (line.length() == 0) {
			continue;
		}
		inputStr = new char[line.size()];
		outputStr = new char[line.size()];

		if (5
				== sscanf(line.c_str(), "%s in %d <%[^>]> out %d <%[^>]>", type,
						&numGateInputs, inputStr, &numGateOutputs, outputStr)) {
			if (strcmp(type, "add") == 0) {
				assert(numGateOutputs == 1);
				handleAddition(inputStr, outputStr);
			} else if (strcmp(type, "mul") == 0) {
				assert(numGateInputs == 2 && numGateOutputs == 1);
				addMulConstraint(inputStr, outputStr);
			} else if (strcmp(type, "xor") == 0) {
				assert(numGateInputs == 2 && numGateOutputs == 1);
				addXorConstraint(inputStr, outputStr);
			} else if (strcmp(type, "or") == 0) {
				assert(numGateInputs == 2 && numGateOutputs == 1);
				addOrConstraint(inputStr, outputStr);
			} else if (strcmp(type, "assert") == 0) {
				assert(numGateInputs == 2 && numGateOutputs == 1);
				addAssertionConstraint(inputStr, outputStr);
			} else if (strstr(type, "const-mul-neg-")) {
				assert(numGateInputs == 1 && numGateOutputs == 1);
				handleMulNegConst(type, inputStr, outputStr);
			} else if (strstr(type, "const-mul-")) {
				assert(numGateInputs == 1 && numGateOutputs == 1);
				handleMulConst(type, inputStr, outputStr);
			} else if (strcmp(type, "zerop") == 0) {
				assert(numGateInputs == 1 && numGateOutputs == 2);
				addNonzeroCheckConstraint(inputStr, outputStr);
			} else if (strstr(type, "split")) {
				assert(numGateInputs == 1);
				addSplitConstraint(inputStr, outputStr, numGateOutputs);
			} else if (strstr(type, "pack")) {
				assert(numGateOutputs == 1);
				addPackConstraint(inputStr, outputStr, numGateInputs);
			}
		} else {
//			assert(0);
		}
		delete[] inputStr;
		delete[] outputStr;
		clean();
	}

	ifs2.close();

	printf("\tConstraint translation done\n");
	look_up_our_self(&usage2);
	unsigned long diff = usage2.vsize - usage1.vsize;
	printf("\tMemory usage for constraint translation: %lu MB\n", diff >> 20);

}

void CircuitReader::mapValuesToProtoboard() {

	int zeropGateIndex = 0;
	for (WireMap::iterator iter = variableMap.begin();
			iter != variableMap.end(); ++iter) {
		Wire wireId = iter->first;
		pb->val(*variables[variableMap[wireId]]) = wireValues[wireId];
		if (zeropMap.find(wireId) != zeropMap.end()) {
			LinearCombination l = *zeroPwires[zeropGateIndex++];
			if (pb->val(l) == 0) {
				pb->val(*variables[zeropMap[wireId]]) = 0;
			} else {
				pb->val(*variables[zeropMap[wireId]]) = pb->val(l).inverse(
						pb->fieldType_);
			}
		}
	}
	if (!pb->isSatisfied(PrintOptions::DBG_PRINT_IF_NOT_SATISFIED)) {
		printf("Protoboard Not Satisfied");
		assert(0);
	}
	printf("Assignment of values done .. \n");

}

int CircuitReader::find(Wire wireId, LinearCombinationPtr& lc,
		bool intentionToEdit) {

	LinearCombinationPtr p = wireLinearCombinations[wireId];
	if (p) {
		wireUseCounters[wireId]--;
		if (wireUseCounters[wireId] == 0) {
			toClean.push_back(wireId);
			lc = p;
		} else {
			if (intentionToEdit) {
				lc = make_shared<LinearCombination>(*p);
			} else {
				lc = p;
			}
		}
		return 1;
	} else {
		wireUseCounters[wireId]--;
		lc = make_shared<LinearCombination>(
				LinearCombination(*variables[variableMap[wireId]]));
		if (wireUseCounters[wireId] == 0) {
			toClean.push_back(wireId);
		}
		return 2;
	}
}

void CircuitReader::clean() {

	for (Wire wireId : toClean) {
		wireLinearCombinations[wireId].reset();
	}
	toClean.clear();
}

void CircuitReader::addMulConstraint(char* inputStr, char* outputStr) {

	Wire outputWireId, inWireId1, inWireId2;

	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId1;
	iss_i >> inWireId2;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;

	LinearCombinationPtr l1, l2;
	find(inWireId1, l1);
	find(inWireId2, l2);

	if (variableMap.find(outputWireId) == variableMap.end()) {
		variables.push_back(make_shared<Variable>("mul out"));
		variableMap[outputWireId] = currentVariableIdx;
		pb->addRank1Constraint(*l1, *l2, *variables[currentVariableIdx],
				"Mul ..");
		currentVariableIdx++;
	} else {
		pb->addRank1Constraint(*l1, *l2, *variables[variableMap[outputWireId]],
				"Mul ..");
	}
}

void CircuitReader::addXorConstraint(char* inputStr, char* outputStr) {

	Wire outputWireId, inWireId1, inWireId2;

	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId1;
	iss_i >> inWireId2;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;

	LinearCombinationPtr lp1, lp2;
	find(inWireId1, lp1);
	find(inWireId2, lp2);
	LinearCombination l1, l2;
	l1 = *lp1;
	l2 = *lp2;
	if (variableMap.find(outputWireId) == variableMap.end()) {
		variables.push_back(make_shared<Variable>("xor out"));
		variableMap[outputWireId] = currentVariableIdx;
		pb->addRank1Constraint(2 * l1, l2,
				l1 + l2 - *variables[currentVariableIdx], "XOR ..");
		currentVariableIdx++;
	} else {
		pb->addRank1Constraint(2 * l1, l2,
				l1 + l2 - *variables[variableMap[outputWireId]], "XOR ..");
	}
}

void CircuitReader::addOrConstraint(char* inputStr, char* outputStr) {

	Wire outputWireId, inWireId1, inWireId2;

	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId1;
	iss_i >> inWireId2;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;

	LinearCombinationPtr lp1, lp2;
	find(inWireId1, lp1);
	find(inWireId2, lp2);
	LinearCombination l1, l2;
	l1 = *lp1;
	l2 = *lp2;
	if (variableMap.find(outputWireId) == variableMap.end()) {
		variables.push_back(make_shared<Variable>("or out"));
		variableMap[outputWireId] = currentVariableIdx;
		pb->addRank1Constraint(l1, l2, l1 + l2 - *variables[currentVariableIdx],
				"OR ..");
		currentVariableIdx++;
	} else {
		pb->addRank1Constraint(l1, l2,
				l1 + l2 - *variables[variableMap[outputWireId]], "OR ..");
	}
}

void CircuitReader::addAssertionConstraint(char* inputStr, char* outputStr) {

	Wire outputWireId, inWireId1, inWireId2;

	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId1;
	iss_i >> inWireId2;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;

	LinearCombinationPtr lp1, lp2, lp3;
	find(inWireId1, lp1);
	find(inWireId2, lp2);
	find(outputWireId, lp3);

	LinearCombination l1, l2, l3;
	l1 = *lp1;
	l2 = *lp2;
	l3 = *lp3;
	pb->addRank1Constraint(l1, l2, l3, "Assertion ..");

}

void CircuitReader::addSplitConstraint(char* inputStr, char* outputStr,
		unsigned short n) {

	Wire inWireId;
	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId;

	LinearCombinationPtr l;
	find(inWireId, l);

	istringstream iss_o(outputStr, istringstream::in);

	LinearCombination sum;
	FElem two_i = Fr<default_ec_pp>("1");

	for (int i = 0; i < n; i++) {
		Wire bitWireId;
		iss_o >> bitWireId;
		variables.push_back(make_shared<Variable>("bit out"));
		variableMap[bitWireId] = currentVariableIdx;
		VariablePtr vptr = variables[currentVariableIdx];
		pb->enforceBooleanity(*vptr);
		sum += LinearTerm(*vptr, two_i);
		two_i += two_i;
		currentVariableIdx++;
	}

	pb->addRank1Constraint(*l, 1, sum, "Split Constraint");
}

void CircuitReader::addPackConstraint(char* inputStr, char* outputStr,
		unsigned short n) {

	Wire outputWireId;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;

	istringstream iss_i(inputStr, istringstream::in);
	LinearCombination sum;
	FElem two_i = Fr<default_ec_pp>("1");
	for (int i = 0; i < n; i++) {
		Wire bitWireId;
		iss_i >> bitWireId;
		LinearCombinationPtr l;
		find(bitWireId, l);
		sum += two_i * (*l);
		two_i += two_i;
	}

	VariablePtr vptr;
	if (variableMap.find(outputWireId) == variableMap.end()) {
		variables.push_back(make_shared<Variable>("pack out"));
		variableMap[outputWireId] = currentVariableIdx;
		vptr = variables[currentVariableIdx];
		currentVariableIdx++;
	} else {

		vptr = variables[variableMap[outputWireId]];
	}

	pb->addRank1Constraint(*vptr, 1, sum, "Pack Constraint");

}

void CircuitReader::addNonzeroCheckConstraint(char* inputStr, char* outputStr) {

	Variable auxConditionInverse_;
	Wire outputWireId, inWireId;

	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;
	iss_o >> outputWireId;
	LinearCombinationPtr l;

	find(inWireId, l);
	VariablePtr vptr;
	if (variableMap.find(outputWireId) == variableMap.end()) {
		variables.push_back(make_shared<Variable>("zerop out"));
		variableMap[outputWireId] = currentVariableIdx;
		vptr = variables[currentVariableIdx];
		currentVariableIdx++;
	} else {
		vptr = variables[variableMap[outputWireId]];
	}
	variables.push_back(make_shared<Variable>("zerop aux"));
	pb->addRank1Constraint(*l, 1 - *vptr, 0, "condition * not(output) = 0");
	pb->addRank1Constraint(*l, *variables[currentVariableIdx], *vptr,
			"condition * auxConditionInverse = output");

	zeroPwires.push_back(l);
	zeropMap[outputWireId] = currentVariableIdx;
	currentVariableIdx++;

}

void CircuitReader::handleAddition(char* inputStr, char* outputStr) {

	Wire inWireId, outputWireId;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;

	istringstream iss_i(inputStr, istringstream::in);
	LinearCombinationPtr s, l;
	iss_i >> inWireId;
	find(inWireId, l, true);
	s = l;
	while (iss_i >> inWireId) {
		find(inWireId, l);
		*s += *l;
	}
	wireLinearCombinations[outputWireId] = s;
}

void CircuitReader::handleMulConst(char* type, char* inputStr,
		char* outputStr) {

	char* constStr = type + sizeof("const-mul-") - 1;
	Wire outputWireId, inWireId;

	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;
	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId;
	LinearCombinationPtr l;
	find(inWireId, l, true);
	wireLinearCombinations[outputWireId] = l;
	*(wireLinearCombinations[outputWireId]) *= readFieldElementFromHex(
			constStr);
}

void CircuitReader::handleMulNegConst(char* type, char* inputStr,
		char* outputStr) {

	char* constStr = type + sizeof("const-mul-neg-") - 1;
	Wire outputWireId, inWireId;
	istringstream iss_o(outputStr, istringstream::in);
	iss_o >> outputWireId;
	istringstream iss_i(inputStr, istringstream::in);
	iss_i >> inWireId;

	LinearCombinationPtr l;
	find(inWireId, l, true);

	wireLinearCombinations[outputWireId] = l;
	*(wireLinearCombinations[outputWireId]) *= readFieldElementFromHex(
			constStr);
	*(wireLinearCombinations[outputWireId]) *= FieldT(-1); //TODO: make shared FieldT constants

}
