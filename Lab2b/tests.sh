#!/bin/bash
#tests.sh
#NAME: Brendon Ng
#EMAIL: brendonn8@gmail.com
# ID: 304925492
rm -f lab2b_list.csv
touch lab2b_list.csv

for t in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --iterations=1000 --sync=m --threads=$t >> lab2b_list.csv
    ./lab2_list --iterations=1000 --sync=s --threads=$t >> lab2b_list.csv
done


for t in 1, 4, 8, 12, 16
do
    for i in 1, 2, 4, 8, 16
    do
	./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 >>lab2b_list.csv
    done

    for i in 10, 20, 40, 80
    do
	./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 --sync=s >>lab2b_list.csv
	./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 --sync=m >>lab2b_list.csv
    done
done

for t in 1, 2, 4, 8, 12
do
    for l in 4, 8, 16
    do
	./lab2_list --threads=$t --lists=$l --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=$t --lists=$l --iterations=1000 --sync=m >> lab2b_list.csv
    done
done

echo done
