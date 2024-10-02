#!/bin/bash

rm -rf ./scratch/part_1_wireless_high_static
mkdir -p ./scratch/part_1_wireless_high_static
gnu_file=./scratch/graph.gnuplot


plot_graph()
{
   
    # 1 -> file-name, 2 -> plot-title, 3 -> xlabel, 4 -> ylabel, 5 -> path-to-the-data-file, 6 -> x-data-col, 7 -> y-data-col
     echo "Plotting data from file: $5"
    echo "x-data-col: $6, y-data-col: $7"
    gnuplot -c $gnu_file "$1/static_throughput_$2.png" "throughput vs $2" "$2" "throughput (Kbps)" "$3" "$4" 5
    gnuplot -c $gnu_file "$1/static_delivery_ratio_$2.png" "delivery ratio vs $2" "$2" "delivery ratio" "$3" "$4" 6
    #gnuplot -c $gnu_file "$1/throughput_$2.png" "throughput vs $2" "Number of Flows" "throughput (Kbps)" $3 $4
    #gnuplot -c $gnu_file "$1/delivery_ratio_$2.png" "delivery ratio vs $2" "Number of Flows" "delivery ratio" $3 $4


}


# changing nodes
file=./scratch/part_1_wireless_high_static/nodes.txt
rm -f $file
touch $file

for i in $(seq 20 20 100)  # 20,40,..,100
do
    ./ns3 run "scratch/1905034_1.cc --nNodes=${i} --nFlow=$((i/2)) --filename=${file}" 
    echo "Node : $i done"
done

plot_graph "./scratch" "nodes" $file 1


# changing flows
file=./scratch/part_1_wireless_high_static/flows.txt
rm -f $file
touch $file

for i in $(seq 10 10 50) # 10,20,..50
do
    ./ns3 run "scratch/1905034_1.cc --nFlow=${i} --filename=${file}"
    echo "Flow : $i done"
done

plot_graph "./scratch" "flow" $file 2


# changing packets per second
file=./scratch/part_1_wireless_high_static/packets.txt
rm -f $file
touch $file

for i in $(seq 100 100 500) # 100,200,...,500
do
    ./ns3 run "scratch/1905034_1.cc --nPacketsPerSecond=${i} --filename=${file}"
    echo "Packets per s : $i done"
done

plot_graph "./scratch" "packets per second" $file 3


#  changing coverage area
file=./scratch/part_1_wireless_high_static/coverage.txt
rm -f $file
touch $file

for i in $(seq 1 1 5) #1,2,3,4,5
do
    ./ns3 run "scratch/1905034_1.cc --range=${i} --filename=${file}"
    echo "Coverage Area : $i done"
done

plot_graph "./scratch" "coverage area" $file 4

