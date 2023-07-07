#!/usr/bin/env bash

target="$1"
MRUBY_CONFIG="$PWD/build_config.rb"

pushd .

git submodule init
git submodule update

if [[ "$target" =~ "ios" ]]; then
  CC="$(xcrun -sdk iphoneos -f clang)"
  LD="$(xcrun -sdk iphoneos -f clang)"
  CFLAGS="-isysroot $(xcrun -sdk iphoneos -show-sdk-path) -target arm64-apple-ios -arch arm64 -D_XOPEN_SOURCE"
  LDFLAGS="$CFLAGS"
fi

export CC
export LD
export CFLAGS
export LDFLAGS

cd mruby
#rake clean
rake MRUBY_CONFIG="$MRUBY_CONFIG"

popd
