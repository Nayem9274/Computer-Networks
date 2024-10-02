#!/bin/bash

rm -rf ./scratch/part_1_wireless_high_mobile
mkdir -p ./scratch/part_1_wireless_high_mobile
gnu_file=./scratch/graph.gnuplot


plot_all()
{

     echo "Plotting data from file: $5"
    echo "x-data-col: $6, y-data-col: $7"
    gnuplot -c $gnu_file "$1/mobile_throughput_$2.png" "throughput vs $2" "$2" "throughput (Kbps)" "$3" "$4" 5
    gnuplot -c $gnu_file "$1/mobile_delivery_ratio_$2.png" "delivery ratio vs $2" "$2" "delivery ratio" "$3" "$4" 6
    #gnuplot -c $gnu_file "$1/throughput_$2.png" "throughput vs $2" "Number of Flows" "throughput (Kbps)" $3 $4
    #gnuplot -c $gnu_file "$1/delivery_ratio_$2.png" "delivery ratio vs $2" "Number of Flows" "delivery ratio" $3 $4


}



file=./scratch/part_1_wireless_high_mobile/nodes.txt
rm -f $file
touch $file

for i in $(seq 20 20 100) 
do
    ./ns3 run "scratch/1905034_2.cc --nNodes=${i} --nFlow=$((i/2)) --filename=${file}" 
    echo "Node : $i done"
done

plot_all "./scratch" "nodes" $file 1



file=./scratch/part_1_wireless_high_mobile/flows.txt
rm -f $file
touch $file

for i in $(seq 10 10 50) 
do
    ./ns3 run "scratch/1905034_2.cc --nFlow=${i} --filename=${file}"
    echo "Flow : $i done"
done

plot_all "./scratch" "flow" $file 2



file=./scratch/part_1_wireless_high_mobile/packets.txt
rm -f $file
touch $file

for i in $(seq 100 100 500) 
do
    ./ns3 run "scratch/1905034_2.cc --nPacketsPerSecond=${i} --filename=${file}"
    echo "Packets per s : $i done"
done

plot_all "./scratch" "packets per second" $file 3


# changing speed
file=./scratch/part_1_wireless_high_mobile/speed.txt
rm -f $file
touch $file

for i in $(seq 5 5 25) #5,10,...,25
do
    ./ns3 run "scratch/1905034_2.cc --speed=${i} --filename=${file}"
    echo "speed : $i done"
done

plot_all "./scratch" "speed" $file 4

