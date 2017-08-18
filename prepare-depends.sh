#!/bin/sh
# This script fetches, builds and locally installs external dependencies.

set -x -e

DEPSRC=./depsrc
DEPINST=./depinst

# rm -fr $DEPINST
mkdir -p $DEPINST
mkdir -p $DEPSRC

# ate-pairing library, and its dependency, xbyak (needed for BN128 curve)
cd $DEPSRC
[ ! -d xbyak ] && git clone git://github.com/herumi/xbyak.git
[ ! -d ate-pairing ] && git clone git://github.com/herumi/ate-pairing.git
cd ate-pairing
make -j SUPPORT_SNARK=1
cd ../..
cp -rv $DEPSRC/ate-pairing/include $DEPINST/
cp -rv $DEPSRC/ate-pairing/lib $DEPINST/

# mcl library, and its dependency, xbyak and cybozulib (needed for MCL_BN128 curve)
cd $DEPSRC
[ ! -d xbyak ] && git clone git://github.com/herumi/xbyak.git
[ ! -d cybozulib ] && git clone git://github.com/herumi/cybozulib.git
[ ! -d mcl ] && git clone git://github.com/herumi/mcl.git
cd mcl
make -j
cd ../..
cp -rv $DEPSRC/mcl/include/mcl $DEPINST/include/
cp -rv $DEPSRC/cybozulib/include/cybozu $DEPINST/include/
cp -rv $DEPSRC/mcl/lib $DEPINST/

# SUPERCOP library (optimized crypto implementations, used by ADSNARK)
cd $DEPSRC
[ ! -d libsnark-supercop ] && git clone git://github.com/mbbarbosa/libsnark-supercop.git
cd libsnark-supercop
sh "do"
cd ../..
mkdir -p $DEPINST/include/supercop
cp -v $DEPSRC/libsnark-supercop/include/* $DEPINST/include/supercop
cp -rv $DEPSRC/libsnark-supercop/lib $DEPINST/
