#!/bin/bash

rm -rf massif_output.out

taskfile="TaskFiles/Benchmarks/gravity.nebs"
taskfile="TaskFiles/Tests/spawn_memleak.nebs"

ulimit -n 32768
valgrind --tool=massif --massif-out-file=massif_output.out ./bin/Nebulite_Debug task "$taskfile"

massif-visualizer massif_output.out
