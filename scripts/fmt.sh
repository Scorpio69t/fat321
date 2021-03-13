#!/usr/bin/env bash

if [[ ! -x "$(command -v clang-format)" ]]; then
    echo "Error: clang-format is not installed" >&2
    exit 1
fi

echo "formating..."
clang-format -style=file -i $(find . -name "*.c" -or -name "*.h")
echo "done"