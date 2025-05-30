#!/bin/bash
start=`date +%s.%N`
./bin/Nebulite task TaskFiles/benchmark_gravity.txt
end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Benchmark Passed. Took $runtime s"

