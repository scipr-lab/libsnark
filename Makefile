#********************************************************************************
# Makefile for the libsnark library.
#********************************************************************************
#* @author     This file is part of libsnark, developed by SCIPR Lab
#*             and contributors (see AUTHORS).
#* @copyright  MIT license (see LICENSE file)
#*******************************************************************************/

# To override these, use "make OPTFLAGS=..." etc.
CURVE = BN128
OPTFLAGS = -O2 -march=native -mtune=native
FEATUREFLAGS = -DUSE_ASM -DMONTGOMERY_OUTPUT

# Initialize this using "CXXFLAGS=... make". The makefile appends to that.
CXXFLAGS += -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-comment -Wfatal-errors $(OPTFLAGS) $(FEATUREFLAGS) -DCURVE_$(CURVE)

DEPSRC = depsrc
DEPINST = depinst

LDFLAGS += -L$(DEPINST)/lib -Wl,-rpath $(DEPINST)/lib
LDLIBS += -lgmpxx -lgmp -lboost_program_options
# OpenSSL and its dependencies (needed explicitly for static builds):
LDLIBS += -lcrypto -ldl -lz
CXXFLAGS += -I$(DEPINST)/include -Isrc
# Sentinel file to check existence of this directory (since directories don't work as a Make dependency):
DEPINST_EXISTS = $(DEPINST)/.exists

COMPILE_GTEST :=
ifneq ($(NO_GTEST),1)
	GTESTDIR=/usr/src/gtest
# Compile GTest from sourcecode if we can (e.g., Ubuntu). Otherwise use precompiled one (e.g., Fedora).
# See https://code.google.com/p/googletest/wiki/FAQ#Why_is_it_not_recommended_to_install_a_pre-compiled_copy_of_Goog .
	COMPILE_GTEST :=$(shell test -d $(GTESTDIR) && echo -n 1)
	GTEST_LDLIBS += -lgtest -lpthread
endif

ifneq ($(NO_SUPERCOP),1)
	SUPERCOP_LDLIBS += -lsupercop
endif

LIB_SRCS = \
	src/algebra/curves/alt_bn128/alt_bn128_g1.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_g2.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_init.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_pairing.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_pp.cpp \
	src/algebra/curves/edwards/edwards_g1.cpp \
	src/algebra/curves/edwards/edwards_g2.cpp \
	src/algebra/curves/edwards/edwards_init.cpp \
	src/algebra/curves/edwards/edwards_pairing.cpp \
	src/algebra/curves/edwards/edwards_pp.cpp \
	src/algebra/curves/mnt/mnt4/mnt4_g1.cpp \
	src/algebra/curves/mnt/mnt4/mnt4_g2.cpp \
	src/algebra/curves/mnt/mnt4/mnt4_init.cpp \
	src/algebra/curves/mnt/mnt4/mnt4_pairing.cpp \
	src/algebra/curves/mnt/mnt4/mnt4_pp.cpp \
	src/algebra/curves/mnt/mnt46_common.cpp \
	src/algebra/curves/mnt/mnt6/mnt6_g1.cpp \
	src/algebra/curves/mnt/mnt6/mnt6_g2.cpp \
	src/algebra/curves/mnt/mnt6/mnt6_init.cpp \
	src/algebra/curves/mnt/mnt6/mnt6_pairing.cpp \
	src/algebra/curves/mnt/mnt6/mnt6_pp.cpp \
	src/common/data_structures/integer_permutation.cpp \
	src/common/data_structures/set_commitment.cpp \
	src/common/default_types/r1cs_ppzkpcd_pp.cpp \
	src/common/default_types/tinyram_ppzksnark_pp.cpp \
	src/common/default_types/r1cs_ppzkadsnark_pp.cpp \
	src/common/default_types/tinyram_zksnark_pp.cpp \
	src/common/profiling.cpp \
	src/common/routing_algorithms/as_waksman_routing_algorithm.cpp \
	src/common/routing_algorithms/benes_routing_algorithm.cpp \
	src/common/utils.cpp \
	src/gadgetlib1/constraint_profiling.cpp \
	src/gadgetlib2/adapters.cpp \
	src/gadgetlib2/constraint.cpp \
	src/gadgetlib2/examples/simple_example.cpp \
	src/gadgetlib2/gadget.cpp \
	src/gadgetlib2/infrastructure.cpp \
	src/gadgetlib2/integration.cpp \
	src/gadgetlib2/pp.cpp \
	src/gadgetlib2/protoboard.cpp \
	src/gadgetlib2/variable.cpp \
	src/relations/circuit_satisfaction_problems/tbcs/examples/tbcs_examples.cpp \
	src/relations/circuit_satisfaction_problems/tbcs/tbcs.cpp \
	src/relations/ram_computations/memory/examples/memory_contents_examples.cpp \
	src/relations/ram_computations/memory/memory_store_trace.cpp \
	src/relations/ram_computations/memory/ra_memory.cpp \
	src/relations/ram_computations/rams/fooram/fooram_aux.cpp \
	src/relations/ram_computations/rams/tinyram/tinyram_aux.cpp

ifeq ($(CURVE),BN128)
	LIB_SRCS += \
	        src/algebra/curves/bn128/bn128_g1.cpp \
		src/algebra/curves/bn128/bn128_g2.cpp \
		src/algebra/curves/bn128/bn128_gt.cpp \
		src/algebra/curves/bn128/bn128_init.cpp \
		src/algebra/curves/bn128/bn128_pairing.cpp \
		src/algebra/curves/bn128/bn128_pp.cpp

	CXXFLAGS += -DBN_SUPPORT_SNARK
	LDLIBS += -lzm
endif

EXECUTABLES = \
	src/algebra/curves/tests/test_bilinearity \
	src/algebra/curves/tests/test_groups \
	src/algebra/fields/tests/test_fields \
	src/common/routing_algorithms/profiling/profile_routing_algorithms \
	src/common/routing_algorithms/tests/test_routing_algorithms \
	src/gadgetlib1/gadgets/cpu_checkers/fooram/examples/test_fooram \
	src/gadgetlib1/gadgets/hashes/knapsack/tests/test_knapsack_gadget \
	src/gadgetlib1/gadgets/hashes/sha256/tests/test_sha256_gadget \
	src/gadgetlib1/gadgets/merkle_tree/tests/test_merkle_tree_gadgets \
	src/gadgetlib1/gadgets/routing/profiling/profile_routing_gadgets \
	src/gadgetlib1/gadgets/set_commitment/tests/test_set_commitment_gadget \
	src/gadgetlib1/gadgets/verifiers/tests/test_r1cs_ppzksnark_verifier_gadget \
	src/reductions/ram_to_r1cs/examples/demo_arithmetization \
	src/relations/arithmetic_programs/qap/tests/test_qap \
	src/relations/arithmetic_programs/ssp/tests/test_ssp \
	src/zk_proof_systems/pcd/r1cs_pcd/r1cs_mp_ppzkpcd/profiling/profile_r1cs_mp_ppzkpcd \
	src/zk_proof_systems/pcd/r1cs_pcd/r1cs_mp_ppzkpcd/tests/test_r1cs_mp_ppzkpcd \
	src/zk_proof_systems/pcd/r1cs_pcd/r1cs_sp_ppzkpcd/profiling/profile_r1cs_sp_ppzkpcd \
	src/zk_proof_systems/pcd/r1cs_pcd/r1cs_sp_ppzkpcd/tests/test_r1cs_sp_ppzkpcd \
	src/zk_proof_systems/ppzksnark/bacs_ppzksnark/profiling/profile_bacs_ppzksnark \
	src/zk_proof_systems/ppzksnark/bacs_ppzksnark/tests/test_bacs_ppzksnark \
	src/zk_proof_systems/ppzksnark/r1cs_ppzksnark/profiling/profile_r1cs_ppzksnark \
	src/zk_proof_systems/ppzksnark/r1cs_ppzksnark/tests/test_r1cs_ppzksnark \
	src/zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark \
	src/zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark_generator \
	src/zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark_prover \
	src/zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark_verifier \
	src/zk_proof_systems/ppzksnark/ram_ppzksnark/profiling/profile_ram_ppzksnark \
	src/zk_proof_systems/ppzksnark/ram_ppzksnark/tests/test_ram_ppzksnark \
	src/zk_proof_systems/ppzksnark/tbcs_ppzksnark/profiling/profile_tbcs_ppzksnark \
	src/zk_proof_systems/ppzksnark/tbcs_ppzksnark/tests/test_tbcs_ppzksnark \
	src/zk_proof_systems/ppzksnark/uscs_ppzksnark/profiling/profile_uscs_ppzksnark \
	src/zk_proof_systems/ppzksnark/uscs_ppzksnark/tests/test_uscs_ppzksnark \
	src/zk_proof_systems/zksnark/ram_zksnark/profiling/profile_ram_zksnark \
	src/zk_proof_systems/zksnark/ram_zksnark/tests/test_ram_zksnark


EXECUTABLES_WITH_GTEST = \
	src/gadgetlib2/examples/tutorial \
	src/gadgetlib2/tests/gadgetlib2_test

EXECUTABLES_WITH_SUPERCOP = \
	src/zk_proof_systems/ppzkadsnark/r1cs_ppzkadsnark/examples/demo_r1cs_ppzkadsnark

DOCS = README.html

# For documentation of the following options, see README.md .

ifeq ($(NO_PROCPS),1)
	CXXFLAGS += -DNO_PROCPS
else
	LDLIBS += -lprocps
endif

ifeq ($(LOWMEM),1)
	CXXFLAGS += -DLOWMEM
endif

ifeq ($(PROFILE_OP_COUNTS),1)
	STATIC = 1
	CXXFLAGS += -DPROFILE_OP_COUNTS
endif

ifeq ($(STATIC),1)
	CXXFLAGS += -static -DSTATIC
else
	CXXFLAGS += -fPIC
endif

ifeq ($(MULTICORE),1)
	CXXFLAGS += -DMULTICORE -fopenmp
endif

ifeq ($(CPPDEBUG),1)
        CXXFLAGS += -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
        DEBUG = 1
endif

ifeq ($(DEBUG),1)
        CXXFLAGS += -DDEBUG -ggdb3
endif

ifeq ($(PERFORMANCE),1)
        OPTFLAGS = -O3 -march=native -mtune=native
        CXXFLAGS += -DNDEBUG
        # Enable link-time optimization:
        CXXFLAGS += -flto -fuse-linker-plugin
        LDFLAGS += -flto
endif

LIB_OBJS  =$(patsubst %.cpp,%.o,$(LIB_SRCS))
EXEC_OBJS =$(patsubst %,%.o,$(EXECUTABLES) $(EXECUTABLES_WITH_GTEST) $(EXECUTABLES_WITH_SUPERCOP))

all: \
     $(if $(NO_GTEST),,$(EXECUTABLES_WITH_GTEST)) \
     $(if $(NO_SUPERCOP),,$(EXECUTABLES_WITH_SUPERCOP)) \
     $(EXECUTABLES) \
     $(if $(NO_DOCS),,doc)

doc: $(DOCS)

$(DEPINST_EXISTS):
	# Create placeholder directories for installed dependencies. Some make settings (including the default) require actually running ./prepare-depends to populate this directory.
	mkdir -p $(DEPINST)/lib $(DEPINST)/include
	touch $@

# In order to detect changes to #include dependencies. -MMD below generates a .d file for each .o file. Include the .d file.
-include $(patsubst %.o,%.d, $(LIB_OBJS) $(EXEC_OBJS) )

$(LIB_OBJS) $(EXEC_OBJS): %.o: %.cpp
	$(CXX) -o $@   $< -c -MMD $(CXXFLAGS)

LIBGTESTA = $(DEPINST)/lib/libgtest.a

$(LIBGTESTA): $(GTESTDIR)/src/gtest-all.cc $(DEPINST_EXISTS)
	$(CXX) -o $(DEPINST)/lib/gtest-all.o   -I $(GTESTDIR) -c -isystem $(GTESTDIR)/include $< $(CXXFLAGS)
	$(AR) -rv $(LIBGTESTA) $(DEPINST)/lib/gtest-all.o

src/gadgetlib2/tests/gadgetlib2_test: \
	src/gadgetlib2/tests/adapters_UTEST.cpp \
	src/gadgetlib2/tests/constraint_UTEST.cpp \
	src/gadgetlib2/tests/gadget_UTEST.cpp \
	src/gadgetlib2/tests/integration_UTEST.cpp \
	src/gadgetlib2/tests/protoboard_UTEST.cpp \
	src/gadgetlib2/tests/variable_UTEST.cpp

$(EXECUTABLES): %: %.o $(LIB_OBJS) $(DEPINST_EXISTS)
	$(CXX) -o $@   $@.o $(LIB_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(EXECUTABLES_WITH_GTEST): %: %.o $(LIB_OBJS) $(if $(COMPILE_GTEST),$(LIBGTESTA)) $(DEPINST_EXISTS)
	$(CXX) -o $@   $@.o $(LIB_OBJS) $(CXXFLAGS) $(LDFLAGS) $(GTEST_LDLIBS) $(LDLIBS)

$(EXECUTABLES_WITH_SUPERCOP): %: %.o $(LIB_OBJS) $(DEPINST_EXISTS)
	$(CXX) -o $@   $@.o $(LIB_OBJS) $(CXXFLAGS) $(LDFLAGS) $(SUPERCOP_LDLIBS) $(LDLIBS)

libsnark.a: $(LIB_OBJS)
	$(AR) crs $@ $^

libsnark.so: $(LIB_OBJS)
	$(CXX) -o $@   $^ -shared $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

ifeq ($(STATIC),1)
LIB_FILE =libsnark.a
else
LIB_FILE =libsnark.so
endif

lib: $(LIB_FILE)

$(DOCS): %.html: %.md
	markdown_py -f $@ $^ -x toc -x extra --noisy
#	TODO: Would be nice to enable "-x smartypants" but Ubuntu 12.04 doesn't support that.
#	TODO: switch to redcarpet, to produce same output as GitHub's processing of README.md. But what about TOC?

ifeq ($(PREFIX),)
install:
	$(error Please provide PREFIX. E.g. make install PREFIX=/usr)
else
HEADERS_SRC=$(shell find src -name '*.hpp' -o -name '*.tcc')
HEADERS_DEST=$(patsubst src/%,$(PREFIX)/include/libsnark/%,$(HEADERS_SRC))

$(HEADERS_DEST): $(PREFIX)/include/libsnark/%: src/%
	mkdir -p $(shell dirname $@)
	cp $< $@

install: lib $(HEADERS_DEST) $(DEPINST_EXISTS)
	mkdir -p $(PREFIX)/lib
	cp $(LIB_FILE) $(PREFIX)/lib/$(LIB_FILE)
	cp -rv $(DEPINST)/lib $(PREFIX)
	cp -rv $(DEPINST)/include $(PREFIX)
#TODO: Avoid copying the .exists sentinel into $(PREFIX)/lib/
endif

doxy:
	doxygen doxygen.conf

# Clean generated files, except locally-compiled dependencies
clean:
	$(RM) \
		$(LIB_OBJS) $(EXEC_OBJS) \
		$(EXECUTABLES) $(EXECUTABLES_WITH_GTEST) $(EXECUTABLES_WITH_SUPERCOP) \
		$(DOCS) \
		${patsubst %.o,%.d,${LIB_OBJS} ${EXEC_OBJS}} \
		libsnark.so libsnark.a \
	$(RM) -fr doxygen/ \
	$(RM) $(LIBGTESTA) $(DEPINST)/lib/gtest-all.o

# Clean all, including locally-compiled dependencies
clean-all: clean
	$(RM) -fr $(DEPSRC) $(DEPINST)

.PHONY: all clean clean-all doc doxy lib
