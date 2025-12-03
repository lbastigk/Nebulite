#!/usr/bin/env bash
set -euo pipefail

# commit-test-pipeline: run full pipeline if sources changed
WATCH_DIRS=(src Scripts TaskFiles include)
ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
TMP_DIR="$ROOT_DIR/tmp"
LATEST_FILE="$TMP_DIR/.latest"
LAST_MOD_FILE="$TMP_DIR/.lastModified"
ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"

mkdir -p "$TMP_DIR"

echo "Checking sources for changes in: ${WATCH_DIRS[*]}"
echo "Repository root: $ROOT_DIR"
echo "Finding last modified time..."

# build absolute watch paths under repo root
FOUND=()
for d in "${WATCH_DIRS[@]}"; do
  path="$ROOT_DIR/$d"
  [ -d "$path" ] && FOUND+=("$path")
done

if [ "${#FOUND[@]}" -eq 0 ]; then
  echo "No watch directories found: ${WATCH_DIRS[*]}"
  printf "" > "$LATEST_FILE"
else
  find "${FOUND[@]}" -type f -exec stat -c '%Y %n' {} + 2>/dev/null | sort -nr | head -n 1 > "$LATEST_FILE" || true
fi

if [ -s "$LATEST_FILE" ]; then
  latest="$(cut -d' ' -f1 "$LATEST_FILE")"
else
  latest=0
fi

if [ -f "$LAST_MOD_FILE" ]; then
  last="$(cat "$LAST_MOD_FILE")"
else
  last=0
fi

# detect top-level Makefile name and build make args
MAKEFILE_CAND=(MAKEFILE Makefile makefile)
MAKEFILE_ARG=()
for m in "${MAKEFILE_CAND[@]}"; do
  if [ -f "$ROOT_DIR/$m" ]; then
    MAKEFILE_ARG=(-f "$ROOT_DIR/$m")
    echo "Using makefile: $ROOT_DIR/$m"
    break
  fi
done

if [ "$last" -eq 0 ]; then
  echo "No previous timestamp file found. Running full pipeline..."
  make -C "$ROOT_DIR" "${MAKEFILE_ARG[@]}" build-and-test-native
else
  if [ "$latest" -gt "$last" ]; then
    echo "Changes detected. Running full pipeline in $ROOT_DIR"
    make -C "$ROOT_DIR" "${MAKEFILE_ARG[@]}" build-and-test-native
  else
    echo "No changes detected. Skipping tests."
  fi
fi

if [ -s "$LATEST_FILE" ]; then
  # print last modified file path relative to repo root
  cut -d' ' -f2- "$LATEST_FILE" | sed "s|^$ROOT_DIR/||" | xargs -I{} echo "Last modified file: {}"
else
  echo "Last modified file: (none)"
fi

printf '%s\n' "$latest" > "$LAST_MOD_FILE"
