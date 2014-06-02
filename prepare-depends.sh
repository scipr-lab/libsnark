#!/bin/sh
# This script fetches, builds and locally installes the external ate-pairing library (and its dependency, xbyak)

set -x -e

DEPSRC=./depsrc
DEPINST=./depinst

# rm -fr $DEPINST
mkdir -p $DEPINST
DEPINST=`readlink -f $DEPINST`  # remember absolute path

mkdir -p $DEPSRC
cd $DEPSRC
[ ! -d xbyak ] && git clone git://github.com/herumi/xbyak.git
[ ! -d ate-pairing ] && git clone git://github.com/herumi/ate-pairing.git
cd ate-pairing
git checkout .  # undo our patches if already applied
git apply ../../patches/ate-pairing-change-curve.diff
# make clean
make -j
cp -rv include $DEPINST/
cp -rv lib $DEPINST/
