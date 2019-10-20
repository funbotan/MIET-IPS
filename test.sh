#!/bin/bash
source ../intel/compilers_and_libraries/linux/bin/compilervars.sh intel64
../intel/bin/icpc lab1/lab1.cpp
./a.out
