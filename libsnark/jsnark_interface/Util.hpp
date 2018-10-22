#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <libff/common/default_types/ec_pp.hpp>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;


typedef libff::Fr<libff::default_ec_pp> FieldT;


void readIds(char* str, std::vector<unsigned int>& vec);
FieldT readFieldElementFromHex(char* str);


#endif
