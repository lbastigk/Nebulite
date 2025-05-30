#!/bin/bash

runs=10           # Number of runs to average
total=0

for ((i=1; i<=runs; i++)); do
    start=$(date +%s.%N)
    ./bin/Nebulite task TaskFiles/benchmark_gravity.txt
    end=$(date +%s.%N)

    runtime=$(echo "$end - $start" | bc -l)
    echo "Run #$i: $runtime s"
    total=$(echo "$total + $runtime" | bc -l)
done

average=$(echo "$total / $runs" | bc -l)
echo "Average runtime over $runs runs: $average s"
