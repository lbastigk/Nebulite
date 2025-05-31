#!/bin/bash
valgrind --tool=massif --massif-out-file=massif_output.out ./bin/Nebulite task TaskFiles/Benchmarks/gravity_classic.txt
massif-visualizer massif_output.out
