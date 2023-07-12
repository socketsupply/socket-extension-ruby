#!/usr/bin/env bash

target="$(echo "$1" | sed 's/--platform=//g')"

if [[ "$target" =~ "ios-simulator" ]]; then
  target="ios-simulator"
elif [[ "$target" =~ "ios" ]]; then
  target="ios"
elif [[ "$target" =~ "android" ]]; then
  target="$target"*
else
  target="host"
fi

objects=($(find "$PWD"/mruby/build/$target -name *.o 2>/dev/null))
libraries=($(find "$PWD"/mruby/build/$target -name *.a 2>/dev/null))

for object in "${objects[@]}"; do
  if [[ "$object" =~ "-bin-" ]] || [[ "$object" =~ "mrbc" ]]; then
    continue
  fi

  echo "$object"
done

for library in "${libraries[@]}"; do
  if [[ "$library" =~ "-bin-" ]] || [[ "$library" =~ "mrbc" ]]; then
    continue
  fi

  echo "$library"
done
