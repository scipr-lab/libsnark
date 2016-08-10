#include <iostream>

#include <gadgetlib2/examples/simple_example.hpp>
#include <zk_proof_systems/ppzksnark/r1cs_ppzksnark/examples/run_r1cs_ppzksnark.hpp>

int main() {
    const libsnark::r1cs_example<libsnark::Fr<libsnark::default_ec_pp> > example = libsnark::gen_r1cs_example_from_gadgetlib2_protoboard(100);
    const bool test_serialization = true;
    const bool bit = libsnark::run_r1cs_ppzksnark<libsnark::default_ec_pp>(example, test_serialization);
    std::cout << (bit ? "Success" : "Failure") << std::endl;
}
