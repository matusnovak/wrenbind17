#!/bin/bash

set -e

# Step 1: Capture coverage data
lcov --directory ./build --capture --output-file ./build/coverage.info
# Step 2: Remove unwanted files from coverage data
lcov --remove ./build/coverage.info '/usr/*' "${HOME}/.cache/*" '*/catch2/*' '*/tests/*' --output-file ./build/coverage.info
# Step 3: Suppress warnings about unused exclude patterns
lcov --ignore-errors unused --list ./build/coverage.info
# Step 4: Generate coverage report
geninfo ./build --output-filename ./build/coverage.info --memory 0 --rc geninfo_unexecuted_blocks=1
# Step 5: List coverage results
lcov --list ./build/coverage.info
