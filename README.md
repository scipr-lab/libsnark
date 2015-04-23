libsnark: a C++ library for zkSNARK proofs
================================================================================

--------------------------------------------------------------------------------
Authors
--------------------------------------------------------------------------------

The libsnark library is developed by the [SCIPR Lab] project and contributors
and is released under the MIT License (see the [LICENSE] file).

Copyright (c) 2012-2014 SCIPR Lab and contributors (see [AUTHORS] file).

--------------------------------------------------------------------------------
[TOC]

<!---
  NOTE: the file you are reading is in Markdown format, which is is fairly readable
  directly, but can be converted into an HTML file with much nicer formatting.
  To do so, run "make doc" (this requires the python-markdown package) and view
  the resulting file README.html.
-->

--------------------------------------------------------------------------------
Overview
--------------------------------------------------------------------------------

This library implements __zkSNARK__ schemes, which are a cryptographic method
for proving/verifying, in zero knowledge, the integrity of computations.

A computation can be expressed as an NP statement, in forms such as the following:

- "The C program _foo_, when executed, returns exit code 0 if given the input _bar_ and some auxiliary input _qux_."
- "The arithmetic circuit _foo_ accepts the partial assignment _bar_, when extended into some full assignment _qux_."
- "The set of constraints _foo_ is satisfiable by the partial assignment _bar_, when extended into some full assignment _qux_."

A prover who knows the witness for the NP statement (i.e., a satisfying input/assignment) can produce a short proof attesting to the truth of the NP statement. This proof can be verified by anyone, and enjoys the following properties.

-   __Zero knowledge:__
    the verifier learns nothing from the proof beside the truth of the statement.
-   __Succinctness:__
    the proof is short and easy to verify.
-   __Non-interactivity:__
    the proof is a string (i.e. it does not require back-and-forth interaction between the prover and the verifier).
-   __Soundness:__
    the proof is computationally sound (i.e., it is infeasible to fake a proof of a false NP statement). Such a proof system is also called an _argument_.
-   __Proof of knowledge:__
    the proof attests not just that the NP statement is true, but also that the
    prover knows why (e.g., knows a valid _qux_).

These properties are summarized by the _zkSNARK_ acronym, which stands for _Zero-Knowledge Succinct Non-interactive ARgument of Knowledge_ (though zkSNARKs are also knows as
_succinct non-interactive computationally-sound zero-knowledge proofs of knowledge_).
For formal definitions and theoretical discussions about these, see
\[BCCT12], \[BCIOP13], and the references therein.

The libsnark library currently provides a C++ implementation of:

1.  A zkSNARK for the NP-complete language "R1CS" (_rank-1 constraint systems_), which is a language that is similar to arithmetic circuit satisfiability.
2.  Gadget libraries (gadgetlib1 and gadgetlib2) for constructing R1CS instances out of modular "gadget" classes.

Future releases of libsnark will add many examples of R1CS instances, including those for checking execution of TinyRAM machine code, as explained in \[BCTV14] and \[BCGTV13]; in turn, such machine code can be obtained, e.g., by compiling from C.

The zkSNARK construction implemented by libsnark follows, extends, and optimizes the approach described in \[BCTV14], itself an extension of \[BCGTV13], following the approach of \[BCIOP13] and \[GGPR13]. An alternative implementation of the basic approach is the _Pinocchio_ system of \[PGHR13]. See these references for discussions of efficiency aspects that arise in practical use of such constructions, as well as security and trust considerations.

This scheme is a _preprocessing zkSNARK_ (_ppzkSNARK_): before proofs can be created and verified, one needs to first decide on a size/circuit/system representing the NP statements to be proved, and run a _generator_ algorithm to create corresponding public parameters (a long proving key and a short verification key).

Using the library involves the following high-level steps:

1.  Express the statements to be proved as a R1CS, by writing C++
    code that creates these constraints with the help of gadgetlib1 or gadgetlib2.
    Link this code together with libsnark.
2.  Use libsnark's generator algorithm to create the public parameters for this R1CS
    (once and for all).
3.  Use libsnark's prover algorithm to create proofs of true statements about
    the satisfiability of the R1CS.
4.  Use libsnark's verifier algorithm to check proofs for alleged statements.


--------------------------------------------------------------------------------
The NP-complete language R1CS
--------------------------------------------------------------------------------

The ppzkSNARK supports proving/verifying membership in a specific NP-complete
language: R1CS (*rank-1 constraint systems*). An instance of the language is
specified by a set of equations over a prime field F, and each equation looks like:
                   < A, (1,X) > * < B , (1,X) > = < C, (1,X) >
where A,B,C are vectors over F, and X is a vector of variables.

In particular, arithmetic (as well as boolean) circuits are easily reducible to
this language by converting each gate into a rank-1 constraint. See \[BCGTV13]
Appendix E (and "System of Rank 1 Quadratic Equations") for more details about this.


--------------------------------------------------------------------------------
Elliptic curve choices
--------------------------------------------------------------------------------

The ppzkSNARK can be instantiated with different parameter choices, depending on
which elliptic curve is used. The libsnark library currently provides three
options:

* "edwards":
   an instantiation based on an Edwards curve, providing 80 bits of security.

* "bn128":
   an instantiation based on a Barreto-Naehrig curve, providing 128
   bits of security. The underlying curve implementation is
   \[ate-pairing], which has incorporated our patch that changes the
   BN curve to one suitable for SNARK applications.

    *   This implementation uses dynamically-generated machine code for the curve
        arithmetic. Some modern systems disallow execution of code on the heap, and
        will thus block this implementation.

        For example, on Fedora 20 at its default settings, you will get the error
        `zmInit ERR:can't protect` when running this code. To solve this,
        run `sudo setsebool -P allow_execheap 1` to allow execution,
        or use `make CURVE=ALT_BN128` instead.

* "alt_bn128":
   an alternative to "bn128", somewhat slower but avoids dynamic code generation.

Note that bn128 requires an x86-64 CPU while the other curve choices
should be architecture-independent; see [portability](#portability).


--------------------------------------------------------------------------------
Gadget libraries
--------------------------------------------------------------------------------

The libsnark library currently provides two libraries for conveniently constructing
R1CS instances out of reusable "gadgets". Both libraries provide a way to construct
gadgets on other gadgets as well as additional explicit equations. In this way,
complex R1CS instances can be built bottom up.

### gadgetlib1

This library is a minimalistic library that only seeks to support basic
functionality for the construction of R1CS instances. Moreover, its design is
based on templates (as does the ppzkSNARK code) to efficiently support
working on multiple elliptic curves simultaneously.

### gadgetlib2

This library provides support for constructing systems of polynomial equations
and, in particular, also R1CS instances. It is better documented and easier to use
than gadgetlib1, and its interface does not use templates.

### Choice and stability

We advise new uses of libsnark to use gadgetlib2, unless the template features of
gadgetlib1 are specifically required. In the future, we plan to bring the template
features to gadgetlib2 as well. Note that (consequentially) the constraint library
interface and its implementation are in flux, and future versions of libsnark are
likely to break compatibility.


--------------------------------------------------------------------------------
Security
--------------------------------------------------------------------------------

The theoretical security of the underlying mathematical constructions, and the
requisite assumptions, are analyzed in detailed in the aforementioned research
papers.

**
This code is a research-quality proof of concept, and has not
yet undergone extensive review or testing. It is thus not suitable,
as is, for use in critical or production systems.
**

Known issues include the following:

* The ppzkSNARK's generator and prover exhibit data-dependent running times
  and memory usage. These form timing and cache-contention side channels,
  which may be an issue in some applications.

* Randomness is retrieved from /dev/urandom, but this should be
  changed to a carefully considered (depending on system and threat
  model) external, high-quality randomness source when creating
  long-term proving/verification keys.


--------------------------------------------------------------------------------
Build instructions
--------------------------------------------------------------------------------

The libsnark library relies on the following:

- C++ build environment
- GMP for certain bit-integer arithmetic
- libprocps for reporting memory usage
- GTest for some of the unit tests

So far we have tested these only on Linux, though we have been able to make the library work,
with some features disabled (such as memory profiling or GTest tests), on Windows via Cygwin
and on Mac OS X. (If you succeed in achieving more complete ports of the library, please
let us know!) See also the notes on [portability](#portability) below.

For example, on a fresh install of Ubuntu 14.04, install the following packages:

    $ sudo apt-get install build-essential git libgmp3-dev libprocps3-dev libgtest-dev python-markdown

Or, on Fedora 20:

    $ sudo yum install gcc-c++ make git gmp-devel procps-ng-devel gtest-devel python-markdown

Run the following, to fetch \[ate-pairing] from its Github repo and
compile it. (Required only when using the default bn128 curve.)

    $ ./prepare-depends.sh

If not using bn128 option the depinst directory, referred to in Makefile, needs to be created manually:

    $ mkdir -p depinst/include depinst/src

Then, to compile the library, tests, profiling harness and documentation, run:

    $ make

To create just the HTML documentation, run

    $ make doc

and then view the resulting `README.html` (which contains the very text you are reading now).

To create Doxygen documentation summarizing all files, classes and functions,
with some (currently sparse) comments, install the `doxygen` and `graphviz` packages, then run

    $ make doxy

(this may take a few minutes). Then view the resulting [`doxygen/index.html`](doxygen/index.html).

To build the shared object library `libsnark.so`, run:

    $ make lib

To build the static library `libsnark.a`, run:

    $ make lib STATIC=1

As for Cygwin, it should suffice to install g++ and libgmp using the graphical installer and use:

    $ make NO_PROCPS=1 NO_GTEST=1 NO_DOCS=1

As for Mac OS X, it should suffice to install GMP from MacPorts (`port install gmp`) and use:

    $ make NO_PROCPS=1 NO_GTEST=1 NO_DOCS=1

MacPorts does not write its libraries into standard system folders, so you might need to explicitly provide the paths to the header files and libraries by appending `CXXFLAGS=-I/opt/local/include LDFLAGS=-L/opt/local/lib` to the line above.
Similarly, to pass the paths to ate-pairing you would run `INC_DIR=-I/opt/local/include LIB_DIR=-L/opt/local/lib ./prepare-depends.sh` instead of `./prepare-depends.sh` above.

--------------------------------------------------------------------------------
Tutorials
--------------------------------------------------------------------------------

libsnark includes a tutorial, and some usage examples, for the high-level API.

* `src/gadgetlib2/examples` contains a tutorial for using gadgetlib2 to express NP statements as constraint systems. It introduces basic terminology, design overview, and recommended programming style. It also shows how to invoke ppzkSNARKs on such constraint systems. The main file, `tutorial.cpp`, builds into a standalone executable.

* `src/gadgetlib1/examples1` contains a simple example for constructing a constraint system using gadgetlib1. gadgetlib1 was the predecessor to gadgetlib2 and shares similar design methodologies.

* `r1cs_ppzksnark/examples/demo_r1cs_ppzksnark.cpp` constructs a simple constraint system and runs the ppzksnark. See below for how to run it.


--------------------------------------------------------------------------------
Executing profiling example
--------------------------------------------------------------------------------

The command

     $ src/r1cs_ppzksnark/examples/demo_r1cs_ppzksnark 1000 10 Fr

exercises the ppzkSNARK (first generator, then prover, then verifier) on an R1CS instance with 1000 equations and an input consisting of 10 field elements.

(If you get the error `zmInit ERR:can't protect`, see the discussion [above](#elliptic-curve-choices).)

The command

     $ src/r1cs_ppzksnark/examples/demo_r1cs_ppzksnark 1000 10 bytes

does the same but now the input consists of 10 bytes.


--------------------------------------------------------------------------------
Build options
--------------------------------------------------------------------------------

The following flags change the behavior of the compiled code:

*    define `BINARY_OUTPUT`

     In serialization, output raw binary data (instead of decimal, when not set).

*   `make CURVE=choice` / define `CURVE_choice` (where `choice` is one of: ALT_BN128, BN128, EDWARDS, MNT4, MNT6)

     Set the default curve to one of the above (see [elliptic curve choices](#elliptic-curve-choices)).

*   `make DEBUG=1` / define `DEBUG`

    Print additional information for debugging purposes.

*   `make LOWMEM=1` / define `LOWMEM`

    Limit the size of multi-exponentiation tables, for low-memory platforms.

*   `make NO_DOCS=1`

     Do not generate HTML documentation, e.g. on platforms where Markdown is not easily available.

*   `make NO_PROCPS=1`

     Do not link against libprocps. This disables memory profiling.

*   `make NO_GTEST=1`

     Do not link against GTest. The tutorial and test suite of gadgetlib2 tutorial won't be compiled if this option is enabled.

*   `make MULTICORE=1`

     Enable parallelized execution of the ppzkSNARK generator and prover, using OpenMP.

*   define `NO_PT_COMPRESSION`

    Do not use point compression.
    This gives much faster serialization times, at the expense of ~2x larger
    sizes for serialized keys and proofs.

*   define `MONTGOMERY_OUTPUT` (on by default)

    Serialize Fp elements as their Montgomery representations. If this
    option is not set then Fp elements are serialized as their
    equivalence classes, which is slower but produces human-readable
    output.

*   `make PROFILE_OP_COUNTS=1` / define `PROFILE_OP_COUNTS`

    Collect counts for field and curve operations inside static variables
    of the corresponding algebraic objects. This option works for all
  curves except bn128.

*   define `USE_ASM` (on by default)

    Use unrolled assembly routines for F[p] arithmetic and faster heap in
    multi-exponentiation. (When not set, use GMP's `mpn_*` routines instead.)

*   `make PERFORMANCE=1`

    Enables various compiler optimizations for the current CPU, and disables debugging aids.

Not all combinations are tested together or supported by every part of the codebase.


--------------------------------------------------------------------------------
Portability
--------------------------------------------------------------------------------

libsnark is written in fairly standard C++11.

However, having been developed on Linux on x86-64 CPUs, libsnark has some limitations
with respect to portability. Specifically:

1. libsnark's algebraic data structures assume little-endian byte order.

2. Profiling routines use `clock_gettime` and `readproc` calls, which are Linux-specific.

3. Random-number generation is done by reading from `/dev/urandom`, which is
   specific to Unix-like systems.

4. libsnark binary serialization routines (see `BINARY_OUTPUT` above) assume
   a fixed machine word size (i.e. sizeof(mp_limb_t) for GMP's limb data type).
   Objects serialized in binary on a 64-bit system cannot be de-serialized on
   a 32-bit system, and vice versa.
   (The decimal serialization routines have no such limitation.)

5. libsnark requires a C++ compiler with good C++11 support. It has been
   tested with g++ 4.7, g++ 4.8, and clang 3.4.

6. On x86-64, we by default use highly optimized assembly implementations for some
   operations (see `USE_ASM` above). On other architectures we fall back to a
   portable C++ implementation, which is slower.

Tested configurations include:

* Debian jessie with g++ 4.7 on x86-64
* Debian jessie with clang 3.4 on x86-64
* Fedora 20 with g++ 4.8.2 on x86-64
* Ubuntu 14.04 LTS with g++ 4.8 on x86-64
* Ubuntu 14.04 LTS with g++ 4.8 on x86-32, for EDWARDS and ALT_BN128 curve choices
* Debian wheezy with g++ 4.7 on ARM little endian (Debian armel port) inside QEMU, for EDWARDS and ALT_BN128 curve choices
* Windows 7 with g++ 4.8.3 under Cygwin 1.7.30 on x86-64 with NO_PROCPS=1, NO_GTEST=1 and NO_DOCS=1, for EDWARDS and ALT_BN128 curve choices
* Mac OS X 10.9.4 (Mavericks) with Apple LLVM version 5.1 (based on LLVM 3.4svn) on x86-64 with NO_PROCPS=1, NO_GTEST=1 and NO_DOCS=1


--------------------------------------------------------------------------------
Directory structure
--------------------------------------------------------------------------------

The directory structure of the libsnark library is as follows:

* src/ --- main C++ source code, containing the following modules:
    * algebra/ --- fields and elliptic curve groups
    * common/ --- miscellaneous utilities
    * encoding/ --- cryptographic encoding of group elements
    * gadgetlib1/ --- gadgetlib1, a library to construct R1CS instances
        * gadgets/ --- basic gadgets for gadgetlib1
    * gadgetlib2/ --- gadgetlib2, a library to construct R1CS instances
    * qap/ --- quadratic arithmetic program
        * domains/ --- support for fast interpolation/evaluation, by providing
          FFTs and Lagrange-coefficient computations for various domains
    * r1cs/ --- interfaces for R1CS instances
    * r1cs_ppzksnark/ --- preprocessing zk-SNARK for R1CS instances

    Some of these module directories have the following subdirectories:

    * ...
        * examples/ --- example code and tutorials for this module
        * tests/ --- unit tests for this module

    In particular, the top-level API examples are at `src/r1cs_ppzksnark/examples/` and `src/gadgetlib2/examples/`.

* depsrc/ --- created by `prepare_depends.sh` for retrieved sourcecode and local builds of external code
  (currently: \[ate-pairing], and its dependency xbyak).

* depinst/ --- created by `prepare_depends.sh` and `Makefile`
  for local installation of locally-compiled dependencies.

* doxygen/ --- created by `make doxy` and contains a Doxygen summary of all files, classes etc. in libsnark.


--------------------------------------------------------------------------------
Further considerations
--------------------------------------------------------------------------------

### Multiexponentiation window size

The ppzkSNARK's generator has to solve a fixed-base multi-exponentiation
problem.  We use a window-based method in which the optimal window size depends
on the size of the multiexponentiation instance *and* the platform.

On our benchmarking platform (a 3.40 GHz Intel Core i7-4770 CPU), we have
computed for each curve optimal windows, provided as
"fixed_base_exp_window_table" initialization sequences, for each curve; see
`X_init.cpp` for X=edwards,bn128,alt_bn128.

Performance on other platforms may not be optimal (but probably not be far off).
Future releases of the libsnark library will include a tool that generates
optimal window sizes.


--------------------------------------------------------------------------------
References
--------------------------------------------------------------------------------

\[BCCT12] [
  _From extractable collision resistance to succinct non-Interactive arguments of knowledge, and back again_
](http://eprint.iacr.org/2011/443)
  Nir Bitansky, Ran Canetti, Alessandro Chiesa, Eran Tromer
  ITCS 2012


\[BCGTV13] [
  _SNARKs for C: Verifying Program Executions Succinctly and in Zero Knowledge_
](http://eprint.iacr.org/2013/507),
  Eli Ben-Sasson and Alessandro Chiesa and Daniel Genkin and Eran Tromer, Madars Virza,
  CRYPTO 2013

\[BCIOP13] [
  _Succinct Non-Interactive Arguments via Linear Interactive Proofs_
](http://eprint.iacr.org/2012/718)
  Nir Bitansky, Alessandro Chiesa, Yuval Ishai, Rafail Ostrovsky, Omer Paneth
  TCC 2013

\[BCTV14] [
  _Succinct Non-Interactive Zero Knowledge for a von Neumann Architecture_
](http://eprint.iacr.org/2013/879)
  Eli Ben-Sasson and Alessandro Chiesa and Eran Tromer, Madars Virza
  USENIX Security 2014

\[GGPR13] [
  _Quadratic span programs and succinct NIZKs without PCPs_
](http://eprint.iacr.org/2012/215)
  Rosario Gennaro, Craig Gentry, Bryan Parno, Mariana Raykova
  EUROCRYPT 2013

\[ate-pairing] [
  _High-Speed Software Implementation of the Optimal Ate Pairing over Barreto-Naehrig Curves_
](https://github.com/herumi/ate-pairing)
  MITSUNARI Shigeo, TERUYA Tadanori

\[PGHR13] [
  _Pinocchio: Nearly Practical Verifiable Computation_
](http://eprint.iacr.org/2013/279)
  Bryan Parno, Craig Gentry, Jon Howell, Mariana Raykova
  IEEE S&P 2013


[SCIPR Lab]: http://www.scipr-lab.org/ (Succinct Computational Integrity and Privacy Research Lab)

[LICENSE]: LICENSE (LICENSE file in top directory of libsnark distribution)

[AUTHORS]: AUTHORS (AUTHORS file in top directory of libsnark distribution)
