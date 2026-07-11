##############################################################
# Makefile for building and testing Nebulite
##############################################################


############################################
# Base CMake Presets Build Targets
############################################

linux-release:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

linux-debug:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

linux-coverage:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

linux-profiling:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

windows-release:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

windows-debug:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

macos-release:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

macos-debug:
	@echo "Building preset: $@"
	@cmake --preset $@
	@cmake --build --preset $@ -j$(nproc)

############################################
# TODO: move available/native/all to python script

############################################
# Gather native and available Presets
############################################

# Find native build preset for this machine,
# not including coverage or profiling builds
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	NATIVE_PRESET := linux-release linux-debug
else ifeq ($(UNAME_S),Darwin)
	NATIVE_PRESET := macos-release macos-debug
else ifeq ($(OS),Windows_NT)
	NATIVE_PRESET := windows-release windows-debug
else
	NATIVE_PRESET := linux-release linux-debug
endif

# Find available presets for this machine based on OS and compilers available
# We consider NATIVE_PRESET as well as any cross-compilation presets possible
# Currently, we only check if wine and mingw are available for Windows cross-compilation
AVAILABLE_PRESETS := $(NATIVE_PRESET)
ifeq ($(UNAME_S),Linux)
# check for wine and mingw availability
# if available, add windows presets
ifneq ($(shell command -v wine 2> /dev/null),)
ifneq ($(shell command -v x86_64-w64-mingw32-g++ 2> /dev/null),)
	# add windows presets
	AVAILABLE_PRESETS += windows-release windows-debug
endif
endif
endif

# Define target "all" as building all available presets
all: $(AVAILABLE_PRESETS)

############################################
# Basic useful targets
############################################

.PHONY: all install-deps resources test run clean $(NATIVE_PRESET)

install-deps:
	@./Scripts/Installation/pythonPackages.sh

resources:
	@./Scripts/AssetCreation/create_resources_directory.sh

test:
	@python Scripts/TestingSuite.py --stop --verbose

clean:
	@rm -rf tmp/ bin/ || true

message-available-presets:
	@echo "Available presets for this machine: $(AVAILABLE_PRESETS)"

coverage-report:
	@echo "Generating coverage report..."
	@python Scripts/TestingSuite.py --coverage --verbose

delete-binaries:
	@rm -rf bin/* || true


############################################
# Build
############################################

# Fast build targets for native and available presets

build-native: $(NATIVE_PRESET)

build-available: $(AVAILABLE_PRESETS) message-available-presets

build-coverage: linux-coverage coverage-report


############################################
# Documentation
############################################

docs:
	@echo "Generating documentation for keywords..."
	@python Scripts/MakeCommandDocumentation.py
	@python Languages/nebs/nebulite-script-vscode/scripts/extract_keywords.py

standards:
	@echo "Generating standard files..."
	@./Scripts/MakeStandards.sh

############################################
# Full testing pipelines
############################################

# Coverage build + report

build-and-coverage-report: build-coverage coverage-report

build-and-test-native: delete-binaries build-native test

build-and-test-available: delete-binaries build-available test


############################################
# Profiling
############################################

profiling: linux-profiling
	@echo "Running profiling.nebs"
	@sudo -S sysctl -w kernel.perf_event_paranoid=-1 ; sudo sysctl -w kernel.kptr_restrict=0 && perf record -F 99 -g -- ./bin/Nebulite_Profiling task TaskFiles/Debugging/profiling.nebs ; hotspot perf.data

############################################
# Memory Checking
############################################

memory-check-gui:
	@echo "Running memory check with Valgrind..."
	@rm -rf massif_output.out ; ulimit -n 32768 && valgrind --tool=massif --massif-out-file=massif_output.out --max-snapshots=500 --detailed-freq=1 ./bin/Nebulite_Debug task TaskFiles/Debugging/memoryleak.nebs && massif-visualizer massif_output.out
	@echo "Memory check completed."

memory-check-cli:
	@echo "Running memory check with Valgrind..."
	@ulimit -n 32768 && valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./bin/Nebulite_Debug task TaskFiles/Debugging/memoryleak.nebs
	@echo "Memory check completed. See valgrind_output.txt for details."

############################################
# Release Packaging
############################################

release:
	@Scripts/createRelease.sh

############################################
# Commit testing pipelines
############################################

commit-test-pipeline:
	@Scripts/commitTestPipeline.sh
