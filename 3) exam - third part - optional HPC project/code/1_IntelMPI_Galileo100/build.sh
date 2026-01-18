#!/bin/bash

module load autoload intelmpi;
mpiicc -std=c99 matrix_transformation.c -o matrix_transformation;