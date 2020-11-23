#!/bin/bash

echo "Generate Makefile..."
cmake -DAUTOTEST=ON ..

echo "Run Makefile..."
make

echo "update out.test..."
cp out.test ../autotest/
if (($? != 0)); then echo "update out.test failed."; fi
