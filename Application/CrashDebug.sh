#!/bin/bash

ulimit -n 1024  # Set max open files to 1024

taskname="task TaskFiles/Tests/crash_test.txt"
taskname="task TaskFiles/Benchmarks/benchmark_gravity.txt"
taskname='eval echo $(global.time.t)'


valgrind \
  --tool=callgrind \
  --error-limit=no \
  --num-callers=50 \
  ./bin/Nebulite_Debug "$taskname"

