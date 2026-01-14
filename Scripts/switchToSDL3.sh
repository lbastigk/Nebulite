#!/usr/bin/env bash
# bash
set -euo pipefail

# simple safety checks
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || { echo "Error: not a git repository"; exit 1; }
CUR_BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "Current branch: $CUR_BRANCH"

# backup
TS=$(date +%s)
cp -v .gitmodules ".gitmodules.bak.${TS}" 2>/dev/null || true
cp -v .git/config ".git/config.bak.${TS}" 2>/dev/null || true

# list of old SDL2 submodule paths to remove
OLD_PATHS=(
  "external/SDL2"
  "external/SDL2_image"
  "external/SDL2_ttf"
)

echo "Removing old SDL2 submodules (if present)..."
for p in "${OLD_PATHS[@]}"; do
  if git config -f .gitmodules --get "submodule.$p.url" >/dev/null 2>&1 || [ -d "$p" ] || git submodule status -- "$p" >/dev/null 2>&1; then
    echo " - Removing submodule $p"
    git submodule deinit -f -- "$p" 2>/dev/null || true
    git rm -f "$p" 2>/dev/null || rm -rf "$p"
    rm -rf ".git/modules/$p"
    git config -f .gitmodules --remove-section "submodule.$p" 2>/dev/null || true
    git config --remove-section "submodule.$p" 2>/dev/null || true
  else
    echo " - Skipping $p (not present)"
  fi
done

# stage .gitmodules changes if any
git add .gitmodules 2>/dev/null || true
if ! git diff --cached --quiet -- .gitmodules 2>/dev/null; then
  git commit -m "Remove SDL2 submodules" || true
fi

# new SDL3 submodules to add: array of "url path branch"
NEW_SUBS=(
  "https://github.com/libsdl-org/SDL.git external/SDL3 SDL3"
  "https://github.com/libsdl-org/SDL_image.git external/SDL3_image SDL3"
  "https://github.com/libsdl-org/SDL_ttf.git external/SDL3_ttf SDL3"
)

echo "Adding SDL3 submodules..."
for spec in "${NEW_SUBS[@]}"; do
  read -r url path branch <<<"$spec"
  if [ -d "$path" ] || git config -f .gitmodules --get "submodule.$path.url" >/dev/null 2>&1; then
    echo " - $path already present, attempting to set branch $branch"
    if [ -d "$path/.git" ]; then
      git -C "$path" fetch --all --tags || true
      if git -C "$path" ls-remote --exit-code --heads origin "$branch" >/dev/null 2>&1; then
        git -C "$path" checkout -B "$branch" "origin/$branch" || git -C "$path" checkout "$branch" || true
        git -C "$path" branch --set-upstream-to="origin/$branch" "$branch" >/dev/null 2>&1 || true
      else
        echo "   - remote does not have branch $branch; leaving submodule on default branch"
      fi
    fi
    continue
  fi

  echo " - Adding $path from $url (will try to checkout branch: $branch)"
  git submodule add "$url" "$path"

  # make sure the submodule has fetched remote refs and switch to branch if available
  if [ -d "$path/.git" ]; then
    git -C "$path" fetch --all --tags || true
    if git -C "$path" ls-remote --exit-code --heads origin "$branch" >/dev/null 2>&1; then
      git -C "$path" checkout -B "$branch" "origin/$branch" || true
      git -C "$path" branch --set-upstream-to="origin/$branch" "$branch" >/dev/null 2>&1 || true
      echo "   - checked out $branch in $path"
    else
      echo "   - origin/$branch not found for $path; kept default branch"
    fi
  fi
done

# finalize
git add .gitmodules 2>/dev/null || true
if ! git diff --cached --quiet -- .gitmodules 2>/dev/null; then
  git commit -m "Add SDL3 submodules" || true
fi

git submodule sync --recursive
git submodule update --init --recursive

echo "Done. Backups: .gitmodules.bak.${TS}, .git/config.bak.${TS}"