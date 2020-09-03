#!/usr/bin/env bash

set -e # exit with error if any of this fails
cp -r ../include ../coverage_include
for filename in $(find ../coverage_include -name "*.hpp")
do
  echo $filename
  ../third-party/force-cover/force_cover $filename -- -I. -I../coverage_include --language c++ -std=c++17 -DEMP_TRACK_MEM -DUIT_AUDIT_OCCUPANCY -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/../../../../include/c++/8 -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/../../../../include/x86_64-linux-gnu/c++/8 -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/../../../../include/c++/8/backward -I/usr/include/clang/7/include -I/usr/local/include -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/include -I/usr/include/x86_64-linux-gnu -I/usr/include -I../third-party/Empirical/source -I/usr/include/mpi/ | xargs -0 echo > $filename.temp
  mv $filename.temp $filename

done
