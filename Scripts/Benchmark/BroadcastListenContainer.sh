#!/bin/bash

# Tests the current broadcast-listen-container with small and large benchmarks

BENCHMARK_COUNT=10

BENCHMARK_SMALL_CMD=(./bin/Nebulite task TaskFiles/Benchmarks/spawn_constantly.nebs)
BENCHMARK_LARGE_CMD=(./bin/Nebulite task TaskFiles/Benchmarks/gravity_XL.nebs)

BENCHMARK_SMALL_CLASSIC_CMD=(./bin/Nebulite task TaskFiles/Benchmarks/spawn_constantly_classic.nebs)
BENCHMARK_LARGE_CLASSIC_CMD=(./bin/Nebulite task TaskFiles/Benchmarks/gravity_XL_classic.nebs)

# Generate binary
make delete-binaries
make linux-release

# ensure binary exists
if [ ! -x "${BENCHMARK_SMALL_CMD[0]}" ]; then
  echo "Binary not found or not executable: ${BENCHMARK_SMALL_CMD[0]}"
  exit 1
fi

TIME_SMALL=0.0
TIME_LARGE=0.0

TIME_SMALL_CLASSIC=0.0
TIME_LARGE_CLASSIC=0.0

for i in $(seq 1 "$BENCHMARK_COUNT"); do
    echo "Running small benchmark iteration $i..."
    val_small=$("${BENCHMARK_SMALL_CMD[@]}" | grep 'Benchmark took' | awk '{print $3}')
    val_small=${val_small:-0}
    TIME_SMALL=$(echo "$TIME_SMALL + $val_small" | bc -l)
    echo "Total time after iteration $i: $TIME_SMALL s"

    echo "Running large benchmark iteration $i..."
    val_large=$("${BENCHMARK_LARGE_CMD[@]}" | grep 'Average frame time:' | awk '{print $4}')
    val_large=${val_large:-0}
    TIME_LARGE=$(echo "$TIME_LARGE + $val_large" | bc -l)
    echo "Total frame time after iteration $i: $TIME_LARGE s"

    echo "Running small classic benchmark iteration $i..."
    val_small_classic=$("${BENCHMARK_SMALL_CLASSIC_CMD[@]}" | grep 'Benchmark took' | awk '{print $3}')
    val_small_classic=${val_small_classic:-0}
    TIME_SMALL_CLASSIC=$(echo "$TIME_SMALL_CLASSIC + $val_small_classic" | bc -l)
    echo "Total time for classic after iteration $i: $TIME_SMALL_CLASSIC s"

    echo "Running large classic benchmark iteration $i..."
    val_large_classic=$("${BENCHMARK_LARGE_CLASSIC_CMD[@]}" | grep 'Average frame time:' | awk '{print $4}')
    val_large_classic=${val_large_classic:-0}
    TIME_LARGE_CLASSIC=$(echo "$TIME_LARGE_CLASSIC + $val_large_classic" | bc -l)
    echo "Total frame time for classic after iteration $i: $TIME_LARGE_CLASSIC s"
done

avg_small=$(echo "scale=6; $TIME_SMALL / $BENCHMARK_COUNT" | bc -l)
avg_large=$(echo "scale=6; $TIME_LARGE / $BENCHMARK_COUNT" | bc -l)
avg_small_classic=$(echo "scale=6; $TIME_SMALL_CLASSIC / $BENCHMARK_COUNT" | bc -l)
avg_large_classic=$(echo "scale=6; $TIME_LARGE_CLASSIC / $BENCHMARK_COUNT" | bc -l)

printf "// Static Rulesets:\n"
printf "// Average total time for small benchmark: %.6f s\n" "$avg_small"
printf "// Average frame time for large benchmark: %.6f s\n" "$avg_large"
printf "// JSON Rulesets:\n"
printf "// Average total time for small benchmark: %.6f s\n" "$avg_small_classic"
printf "// Average frame time for large benchmark: %.6f s\n" "$avg_large_classic"
