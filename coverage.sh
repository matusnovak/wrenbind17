#!/bin/bash

set -e

lcov --directory ./build --capture --output-file ./build/coverage.info
lcov --remove ./build/coverage.info '/usr/*' "${HOME}"'/.cache/*' '*/catch2/*' '*/tests/*' --output-file ./build/coverage.info
lcov --list ./build/coverage.info
