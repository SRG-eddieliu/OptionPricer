#!/usr/bin/env bash
set -euo pipefail

mkdir -p output

if command -v brew >/dev/null 2>&1; then
  inc_dir="$(brew --prefix boost)"
else
  inc_dir="/opt/homebrew"
fi

src_files=()
while IFS= read -r file; do
  src_files+=("$file")
done < <(find ./src -name '*.cpp' -print)

c++ -std=c++20 -O2 -I./src -I"${inc_dir}/include" "${src_files[@]}" -o output/main
