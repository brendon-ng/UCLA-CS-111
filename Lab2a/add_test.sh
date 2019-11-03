#!/bin/bash

rm -f lab2_add.csv
touch lab2_add.csv

for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
do
    for t in 1, 2, 4, 8, 12
    do
        ./lab2_add --iterations=$i --threads=$t >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield >> lab2_add.csv
    done
done

for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
do
    for t in 1, 2, 4, 8, 12
    do
        ./lab2_add --iterations=$i --threads=$t >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --sync=m >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield --sync=m >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --sync=s >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield --sync=s >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --sync=c >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield --sync=c >> lab2_add.csv
    done
done
