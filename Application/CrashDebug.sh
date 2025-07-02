#!/bin/bash
ulimit -n 1024  # Set max open files to 1024
taskname="task TaskFiles/Benchmarks/gravity.txt"
valgrind \
  --tool=callgrind \
  --error-limit=no \
  --num-callers=50 \
  ./bin/Nebulite_Debug "$taskname"

