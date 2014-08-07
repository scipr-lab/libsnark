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
cd ate-pairing
make -j SUPPORT_SNARK=1
cd ..
cd ..
cp -rv $DEPSRC/ate-pairing/include $DEPINST/
cp -rv $DEPSRC/ate-pairing/lib $DEPINST/
