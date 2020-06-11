# TinyRAM examples

This directory contains the following, each explained in sections below:
- `process_assembly`: a simple TinyRAM assembly pre-processor (written in Python), which resolves labels and parses the header line to output `program-architecture_params.txt`;
- `run_demo_arithmetization`, `run_demo_ram_ppzksnark`, and `run_ram_ppzksnark_gpv`: scripts for running TinyRAM demonstrations;
- multiple example assembly programs `answer0`, `answer1`, `knapsack`, and `knapsack-indirect`.

## Structure of example directories

An example in directory `program` will contain the following files:
- `program.s` -- the main TinyRAM assembly file;
- `program-computation_bounds.txt` -- a text file containing three space-separated integers: input size bound, program size bound, and time bound; and
- `program-primary_input.txt`, `program-auxiliary_input.txt` -- two text files containing space-separated integers, the contents of primary input and auxiliary input tapes, respectively.

`process_assembly` takes a `.s` assembly code, resolves labels and outputs `program-processed_assembly.txt`, an internal representation of TinyRAM assembly (for easier parsing from libsnark C++ code). It also outputs `program-architecture_params.txt`, a text file containing two space-separated integers `w` (word size) and `k` (register count), specifying TinyRAM machine.

## Example programs

We currently provide the following four programs: two are minimal examples (`answer0`, `answer1`, which always accept / always reject, respectively) and two are zero-knowledge proofs for variant of 0/1 knapsack problem.

The instance for our 0/1 knapsack problem is a list of `k` integers `a_1`, ..., `a_k` together with a target value `target`. The knapsack instance is satisfiable if there exists a subset of `a_i`'s that sum up to `target`.

For example, an instance having `a_i`'s be `2, 3, 5, 7` and `target` of `10`, is satisfiable (10 = 3 + 7), whereas the same `a_i`'s and `target` = `11` is unsatisfiable.

The two examples we provide have markedly different performance, depending on whether the knapsack solution is sparse (the satisfying subset is small compared to the entire set) or not.

Namely, `knapsack.s` assembly has the auxiliary input be the size-`k` bit vector where each bit `b_i` denotes whether or not the corresponding integer `a_i` is to be included. That is, for `2, 3, 5, 7` and `10` example the auxiliary input is `0, 1, 0, 1` (include `3` and `7`, do not include `2` and `5`). This runs in time `O(k)`.

In contrast, `knapsack-indirect.s` assembly has the auxiliary input be the size-`l` array of indices `idx_1`, ..., `idx_l`, and checks the sum `a_{idx_1} + ... + a_{idx_l}` against the target. That is, for  `2, 3, 5, 7` and `10` example the auxiliary input is `2, 4` (select second and fourth element). This runs in time `O(l)`.

We remark that efficiency of `knapsack-indirect` is directly attributable to our routing-network-based capability to perform arbitrary (unknown at the program compile time/SNARK key generation time) memory accesses, and yields to a `O(l log l)`-size circuit for the underlying SNARK proving system. Had we used a linear-sweep to simulate arbitrary memory accesses, instead of routing networks, the resulting SNARK circuit would be of size `O(k * l)`, as each unpredictable memory access would cost `O(k)`. (This explains the name: `knapsack-indirect` uses indirect addressing, whereas `knapsack`'s memory accesses are substantially uniform.) This improvement kicks in at small examples as well: for the above toy example `knapsack` takes 48 TinyRAM cycles, whereas `knapsack-indirect` accepts in just 28 cycles.

## Scripts for running demonstrations

The demonstrations can be exercised by running either of three scripts `run_demo_arithmetization`, `run_demo_ram_ppzksnark`, and `run_ram_ppzksnark_gpv`. All of them assume that libsnark executables were built in `../build` directory, but this path is easy to change.

In more detail,
- running `./run_demo_arithmetization program` runs the "TinyRAM universal circuit reduction" component of our system, but does not exercise the SNARK cryptography. This is most useful for debugging programs as it is fast and outputs full disassembly of execution traces.
- running `./run_demo_ram_ppzksnark` runs the "TinyRAM universal circuit reduction" component of our system, and runs the SNARK cryptography in a single process, passing proving/verification keys and proofs between functions. This is establishes end-to-end functionality of proving and verifying correctness of arbitrary TinyRAM computations.
- running `./run_ram_ppzksnark_gpv`. This is the same as running `./run_demo_ram_ppzksnark`, except the generator, prover, and verifier are separate processes that communicate with files containing serialized proving/verification keys and proofs. This is mostly useful for testing serialization, as it is (due to serialization cost) rather more time-consuming that `./run_demo_ram_ppzksnark`.
