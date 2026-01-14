#!/usr/bin/env bash
# bash
set -euo pipefail

# Configurable values
SDL3_URL="https://github.com/libsdl-org/SDL.git"
SDL3_PATH="external/SDL3"
SDL3_BRANCH="main"
EXTRA_SUBMODULES=(
  "external/SDL3_image:https://github.com/libsdl-org/SDL_image.git"
  "external/SDL3_ttf:https://github.com/libsdl-org/SDL_ttf.git"
)

# Ensure we are in a git repo
if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "Not a git repository" >&2
  exit 1
fi

echo "Backing up .gitmodules to .gitmodules.bak"
cp -f .gitmodules .gitmodules.bak || true

# Find submodule sections and remove any that mention SDL2 in name/path/url
echo "Removing SDL2 submodules (if any)..."
mapfile -t SUBMODULE_SECTIONS < <(git config -f .gitmodules --name-only --get-regexp '^submodule\.' 2>/dev/null || true)
for entry in "${SUBMODULE_SECTIONS[@]}"; do
  # entry looks like "submodule.\"external/xyz\".path" or "submodule.external/xyz.path"
  # Normalize to extract the submodule key between first dot and the final dot
  # Use parameter expansion to strip prefix and suffix
  section="${entry#submodule.}"
  section="${section%%.*}"
  # Remove surrounding quotes if present
  section="${section#\"}"
  section="${section%\"}"
  path="$(git config -f .gitmodules --get "submodule.${section}.path" || true)"
  url="$(git config -f .gitmodules --get "submodule.${section}.url" || true)"
  # Check for SDL2 mention
  if [[ "${section}" == *SDL2* || "${path}" == *SDL2* || "${url}" == *SDL2* ]]; then
    echo "Cleaning submodule section '${section}' (path='${path}', url='${url}')"
    # Deinit from the git metadata (safe-guard)
    if [[ -n "$path" ]]; then
      git submodule deinit -f -- "$path" 2>/dev/null || true
      git rm -f --cached "$path" 2>/dev/null || true
      rm -rf "$path"
    fi
    # Remove from .gitmodules and local git config
    git config -f .gitmodules --remove-section "submodule.${section}" 2>/dev/null || true
    git config --remove-section "submodule.${section}" 2>/dev/null || true
  fi
done

# Commit removal if .gitmodules changed
if ! git diff --quiet -- .gitmodules 2>/dev/null; then
  git add .gitmodules
  git commit -m "Remove SDL2 submodule entries" || true
fi

# Ensure working tree doesn't contain leftover SDL3 directory interfering with add
if [[ -d "${SDL3_PATH}" ]]; then
  echo "Removing existing directory ${SDL3_PATH} to re-add as submodule"
  rm -rf "${SDL3_PATH}"
fi

# Add SDL3 submodule (if not present in .gitmodules)
if ! git config -f .gitmodules --get "submodule.${SDL3_PATH}.path" >/dev/null 2>&1; then
  echo "Adding SDL3 submodule at ${SDL3_PATH} -> ${SDL3_URL}"
  git submodule add "${SDL3_URL}" "${SDL3_PATH}"
else
  echo "SDL3 submodule already present in .gitmodules; ensuring correct url"
  git config -f .gitmodules "submodule.${SDL3_PATH}.url" "${SDL3_URL}"
  git add .gitmodules
fi

# Ensure branch setting is sane (avoid 'origin/SDL3' mistakes)
git config -f .gitmodules "submodule.${SDL3_PATH}.branch" "${SDL3_BRANCH}"
git add .gitmodules

# Add/ensure extra SDL3_* submodules
for spec in "${EXTRA_SUBMODULES[@]}"; do
  IFS=":" read -r path url <<< "$spec"
  if ! git config -f .gitmodules --get "submodule.${path}.path" >/dev/null 2>&1; then
    echo "Adding submodule ${path} -> ${url}"
    git submodule add "${url}" "${path}"
  else
    echo "Submodule ${path} already present; ensuring correct url"
    git config -f .gitmodules "submodule.${path}.url" "${url}"
    git add .gitmodules
  fi
done

# Initialize and update new submodules
echo "Initializing and updating submodules..."
git submodule sync --recursive
git submodule update --init --recursive

# Commit final .gitmodules and index changes
git add .gitmodules
git add -A
if git diff --cached --quiet; then
  echo "No changes to commit."
else
  git commit -m "Replace SDL2 submodules with SDL3 and ensure SDL3_image/SDL3_ttf"
fi

echo "Done. If you still see checkout errors, run: git submodule update --init --recursive"
