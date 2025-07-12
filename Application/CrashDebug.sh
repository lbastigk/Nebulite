#!/bin/bash

# Raise file descriptor and core dump limits
ulimit -n 1024
ulimit -c unlimited

# Define task
taskfile="TaskFiles/Benchmarks/gravity.txt"
#taskfile="TaskFiles/Tests/spawn_memleak.txt"

# Set core dump directory and pattern
CORE_DIR="/tmp"
PATTERN="$CORE_DIR/core.%e.%p.%t"

# Apply kernel setting for core dump pattern
echo "Setting core pattern to $PATTERN"
sudo sysctl -w kernel.core_pattern="$PATTERN"

# Clean up old core files
rm -f $CORE_DIR/core.*

# Parse flags
UNTIL_CRASH=false
if [[ "$1" == "--until-crash" ]]; then
    UNTIL_CRASH=true
    echo "Running in loop until a crash is detected..."
fi

run_once() {
    echo "Running Nebulite_Debug with task: $taskname"
    ./bin/Nebulite_Debug task "$taskfile"
}

# Main loop
attempt=0
while true; do
    ((attempt++))
    echo "=== Attempt $attempt ==="

    run_once

    sleep 1  # Give time for core file to be written

    CORE_FILE=$(ls -t $CORE_DIR/core.* 2>/dev/null | head -n 1)

    if [ -f "$CORE_FILE" ]; then
        echo "Crash detected. Generating stack trace from $CORE_FILE..."
        gdb -batch -ex "set pagination off" -ex "thread apply all bt full" -ex "quit" ./bin/Nebulite_Debug "$CORE_FILE" > crash_stacktrace.txt
        echo "Stack trace saved to crash_stacktrace.txt"
        head -n 40 crash_stacktrace.txt
        break
    fi

    if ! $UNTIL_CRASH; then
        echo "Program exited normally — no crash detected."
        break
    fi

    echo "No crash. Retrying..."
    sleep 0.5
done


####################
# Old version

exit




valgrind \
  --tool=callgrind \
  --error-limit=no \
  --num-callers=50 \
  ./bin/Nebulite_Debug "$taskname"

