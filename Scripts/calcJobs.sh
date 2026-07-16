#!/usr/bin/env bash
set -euo pipefail

# Ensures at least 4GiB of memory is available for each job, and limits the number of jobs to the number of CPU cores.
# Using just nproc can cause out of memory issues.

# Number of logical CPU cores
cores=$(nproc)

# Available memory in KiB
mem_kib=$(awk '/MemAvailable:/ {print $2}' /proc/meminfo)

# Require 4 GiB per job
mem_per_job_kib=$((4 * 1024 * 1024))

# Maximum jobs allowed by memory
mem_jobs=$((mem_kib / mem_per_job_kib))

# Always allow at least one job
(( mem_jobs < 1 )) && mem_jobs=1

# Use the smaller of CPU cores and memory limit
jobs=$(( cores < mem_jobs ? cores : mem_jobs ))

echo "$jobs"