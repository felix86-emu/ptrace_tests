#!/bin/bash

BUILD_DIR="./build"

if [[ ! -d "$BUILD_DIR" ]]; then
    echo "Error: build directory not found"
    exit 1
fi

# Disable core dumps as some processes may purposefully crash
ulimit -c 0

for test in "$BUILD_DIR"/*; do
    if [[ -x "$test" && -f "$test" ]]; then
        name=$(basename "$test")
        echo -n "Running $name... "

        output=$("$test" 2>&1)
        status=$?

        if [[ $status -ne 0 ]]; then
            echo "failed"
            echo "Test output: $output"
            exit 1
        fi

        echo "passed"
    fi
done

echo "All tests passed"
exit 0