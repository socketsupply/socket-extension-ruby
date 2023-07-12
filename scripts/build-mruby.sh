#!/usr/bin/env bash

SSC_PLATFORM_TARGET="$(echo "$1" | sed 's/--platform=//g')"
MRUBY_CONFIG="$PWD/build_config.rb"

git submodule init
git submodule update --depth 1

eval "$(ssc env)"

export SSC_PLATFORM_TARGET
export ANDROID_HOME
export JAVA_HOME

pushd .
cd mruby
rake MRUBY_CONFIG="$MRUBY_CONFIG"
popd

