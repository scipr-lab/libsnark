# Set this to the directory where you installed libsnark.
# If you install to link_example (`make install PREFIX=link_example`),
# the default prefix will work.
INSTALL_PREFIX=.

all:
	$(CXX) main.cpp -o main -std=c++11 -L$(INSTALL_PREFIX)/lib -lsnark -lsupercop -lgmp -lgmpxx -I$(INSTALL_PREFIX)/include -I$(INSTALL_PREFIX)/include/libsnark -DCURVE_BN128

run:
	LD_LIBRARY_PATH=$(INSTALL_PREFIX)/lib ./main
