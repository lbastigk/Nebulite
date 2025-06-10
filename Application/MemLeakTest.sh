#!/bin/bash
valgrind --tool=massif --massif-out-file=massif_output.out ./bin/Nebulite_Debug task TaskFiles/Benchmarks/gravity.txt
massif-visualizer massif_output.out
