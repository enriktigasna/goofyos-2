#!/bin/bash

PWD_DIR="$(pwd)"
UTILS_DIR="$(dirname "$(realpath "$0")")"
ROOT=$UTILS_DIR/../root

echo "[*] Creating root if it doesn't exist.."
mkdir $ROOT
rm -r $ROOT/*

echo "[*] Building init.."
cd $UTILS_DIR/init
make

echo "[*] Copying over binaries.."
mkdir $ROOT/sbin
cp $UTILS_DIR/init/init $ROOT/sbin/init

cd $PWD_DIR