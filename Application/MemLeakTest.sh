#!/bin/bash
valgrind --tool=massif --massif-out-file=massif_output.out ./bin/Nebulite env-load ./Resources/Levels/gravity.json
massif-visualizer massif_output.out
