#!/bin/sh
# This script fetches, builds and locally installs the external ate-pairing library (and its dependency, xbyak)

set -x -e

DEPSRC=./depsrc
DEPINST=./depinst

# rm -fr $DEPINST
mkdir -p $DEPINST
mkdir -p $DEPSRC

cd $DEPSRC
[ ! -d xbyak ] && git clone git://github.com/herumi/xbyak.git
[ ! -d ate-pairing ] && git clone git://github.com/herumi/ate-pairing.git
[ ! -d libsnark-supercop ] && git clone git://github.com/mbbarbosa/libsnark-supercop.git
cd ate-pairing
make -j SUPPORT_SNARK=1
cd ..
cd libsnark-supercop
sh "do"
cd ..
cd ..
cp -rv $DEPSRC/ate-pairing/include $DEPINST/
cp -rv $DEPSRC/ate-pairing/lib $DEPINST/
mkdir -p $DEPINST/include/supercop
cp -v $DEPSRC/libsnark-supercop/include/* $DEPINST/include/supercop
cp -rv $DEPSRC/libsnark-supercop/lib $DEPINST/
