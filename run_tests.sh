#!/usr/bin/env bash
clang++ --std=c++11 test_mbusparser.cpp libraries/mbusparser/mbusparser.cpp -o test_mbusparser && ./test_mbusparser
