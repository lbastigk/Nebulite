#!/bin/bash

rm -rf massif_output.out

taskfile="TaskFiles/Benchmarks/gravity.txt"
taskfile="TaskFiles/Tests/spawn_memleak.txt"

ulimit -n 32768
valgrind --tool=massif --massif-out-file=massif_output.out ./bin/Nebulite_Debug task "$taskfile"

massif-visualizer massif_output.out
