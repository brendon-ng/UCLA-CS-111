#!/bin/bash

rm -f lab2_list.csv
touch lab2_list.csv

for t in 10, 100, 1000, 10000, 20000
do
    ./lab2_list --iterations=$t >> lab2_list.csv
done 

for t in 2, 4, 8, 12
do
    for i in 1, 10, 100, 1000
    do
	./lab2_list --iterations=$i --threads=$t >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --yield=i >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --yield=d >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --yield=l >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --yield=id >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --yield=il >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --yield=dl >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --yield=idl >> lab2_list.csv
    done
done

for t in 2, 4, 8, 12
do
    for i in 1, 2, 4, 8, 16, 32
    do
	./lab2_list --iterations=$i --threads=$t >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=i >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=d >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=l >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=id >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=il >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=dl >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=idl >> lab2_list.csv
    done
done

for t in 2, 4, 8, 12
do
    for i in 1, 2, 4, 8, 16, 32
    do
	./lab2_list --iterations=$i --threads=$t --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=i --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=d --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=l --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=id --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=il --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=dl --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=idl --sync=m >> lab2_list.csv
	./lab2_list --iterations=$i --threads=$t --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=i --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=d --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=l --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=id --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=il --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=dl --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=idl --sync=s >> lab2_list.csv
    done
done

for t in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --iterations=1000 --threads=$t --sync=m >> lab2_list.csv
    ./lab2_list	--iterations=1000 --threads=$t --sync=s	>> lab2_list.csv
done

echo "done"
