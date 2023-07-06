#!/usr/bin/env bash

objects=($(find mruby/build/host -name *.o 2>/dev/null))
libraries=($(find mruby/build/host -name *.a 2>/dev/null))

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
