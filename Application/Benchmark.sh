#!/bin/bash

runs=10           # Number of runs to average
total=0

echo ""
echo "----------------------------------------"
echo "Benchmarking Nebulite with $runs runs..."
echo ""
echo ""
for ((i=1; i<=runs; i++)); do
    start=$(date +%s.%N)
    ./bin/Nebulite task TaskFiles/benchmark_gravity.txt >> /dev/null
    end=$(date +%s.%N)

    runtime=$(echo "$end - $start" | bc -l)
    printf " - Run #%02i: %s seconds \n" $i $runtime
    total=$(echo "$total + $runtime" | bc -l)
done

average=$(echo "$total / $runs" | bc -l)
echo ""
echo "[RESULT] Average runtime over $runs runs: $average seconds"
