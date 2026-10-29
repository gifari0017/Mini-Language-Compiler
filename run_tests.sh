#!/bin/bash
# Runs the compiler on every test program and shows the output.
# Usage:  ./run_tests.sh        (after `make`)
make -s minilang || exit 1
for f in tests/*.ml; do
    echo ""
    echo "############################################################"
    echo "## $f"
    echo "############################################################"
    ./minilang "$f" 2>&1
done
