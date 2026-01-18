#!/usr/bin/env bash

BIN="./bin/Nebulite"
ARGS="'--headless task TaskFiles/VerticalSlice/levelDescription.nebs ; wait 1000 ; exit'"

COUNT=0

while true; do
    COUNT=$((COUNT + 1))
    echo "=== Run #$COUNT ==="

    GDB_OUT=$(
        gdb -q -batch \
            -ex "set pagination off" \
            -ex "handle SIGSEGV stop print pass" \
            -ex "handle SIGABRT stop print pass" \
            -ex "handle SIGFPE stop print pass" \
            -ex "handle SIGILL stop print pass" \
            -ex "run $ARGS" \
            -ex "bt" \
            --args "$BIN" 2>&1
    )

    echo "$GDB_OUT"

    if echo "$GDB_OUT" | grep -q "Program received signal"; then
        echo "💥 Crash detected on run #$COUNT"
        break
    fi
done