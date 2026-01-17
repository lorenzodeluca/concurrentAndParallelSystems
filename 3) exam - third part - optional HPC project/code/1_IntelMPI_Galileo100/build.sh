#!/bin/bash

module load autoload intelmpi;
mpiicc -std=c99 matrix_trasformation.c -o matrix_trasformation;