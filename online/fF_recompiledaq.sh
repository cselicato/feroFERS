#!/bin/bash

##########
# no arguments, simply run ./fF_recompiledaq.sh
##########

echo "Recompiling program in"
echo "$(pwd)"

g++ -o main ../main.cpp ../modes_helpers.cpp ../csv_parser.cpp ../bin_parser.cpp `root-config --cflags --glibs`

echo "Done!"
echo "Have you set up things properly in"
echo "$(pwd)/../modes_helpers.hpp"
echo "before compiling? Check it out:"
head -n 3 $(pwd)/../modes_helpers.hpp
