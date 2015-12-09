#ifndef UTIL_HPP_
#define UTIL_HPP_

//#include <gadgetlib2/gadget.hpp>
//#include "relations/constraint_satisfaction_problems/r1cs/examples/r1cs_examples.hpp"
//#include "gadgetlib2/examples/simple_example.hpp"
//#include "zk_proof_systems/ppzksnark/r1cs_ppzksnark/examples/run_r1cs_ppzksnark.hpp"
//#include "zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
//#include "gadgetlib2/integration.hpp"
#include "gadgetlib2/pp.hpp"
#include "gmp.h"

//#include "pp.hpp"

//#include "common/profiling.hpp"
using namespace libsnark;
using namespace gadgetlib2;

#include <iostream>
#include <sstream>
#include <vector>
using namespace std;


typedef Fr<libsnark::default_ec_pp> FieldT;


void readIds(char* str, std::vector<unsigned int>& vec);
FieldT readFieldElementFromHex(char* str);


#endif
