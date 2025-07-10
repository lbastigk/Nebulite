#!/bin/bash

task="TaskFiles/Benchmarks/gravity.txt"
task="TaskFiles/Tests/spawn_memleak.txt"

ulimit -n 32768
valgrind --tool=massif --massif-out-file=massif_output.out ./bin/Nebulite_Debug task "$task"

massif-visualizer massif_output.out
