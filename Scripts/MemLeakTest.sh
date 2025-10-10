#!/bin/bash

rm -rf massif_output.out ; ulimit -n 32768 && valgrind --tool=massif --massif-out-file=massif_output.out ./bin/Nebulite_Debug task TaskFiles/Tests/vscode_debug.nebs && massif-visualizer massif_output.out
