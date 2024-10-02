#!/bin/bash
gnu_file1="./scratch/cwnd.plt"
gnu_file2="./scratch/throughput.plt"
gnu_file3="./scratch/fairness.plt"

mkdir -p 1905034

prep_files()
{
    mkdir -p ./scratch/1905034/"$1"
    rm -f ./scratch/1905034/"$1"/data_1.txt
    touch ./scratch/1905034/"$1"/data_1.txt
    rm -f ./scratch/1905034/"$1"/data_2.txt
    touch ./scratch/1905034/"$1"/data_2.txt
    rm -f ./scratch/1905034/"$1"/"$1".txt
}

# first arg selects method, second argument selects no of nodes
if [ "$1" == 1 ]; then
    tcp="ns3::TcpHighSpeed"
    method="tcphs"
elif [ "$1" == 2 ]; then
    tcp="ns3::TcpAdaptiveReno"
    method="areno"
else [ "$1" == 3 ]
    tcp="ns3::TcpWestwoodPlus"
    method="westwood"
fi
prep_files $method
path="./scratch/1905034"/"$method"


if [ $# == 1 ]; then
    set "$2" 2
fi

# task 
# for i in $(seq 50 10 100) #inclusive
arr=("1" "5" )
for i in "${arr[@]}";
do
    ./ns3 run "scratch/1905034_A.cc --tcp2=${tcp} --nLeaf=$2 --BottleNeckDataRate=${i} --output_folder=${path} --exp=1" >> "${path}/${method}.txt" 2>&1
    # output file name | file 1 | file1_1stcol | file1_2ndcol | title 1 | file2 | file2_2ndcol | title 2
    gnuplot -c $gnu_file1 "$path/cwnd_bttlnkRate_${i}.png" "${path}/flow1.cwnd" 1 2 "newreno" "${path}/flow2.cwnd" 2 ${method}
    echo "BottleNeckkRate $i done"
done

# output file name | title | x label | file 1 | file1_1stcol | file1_2ndcol | file1_3rdcol | title 2
gnuplot -c $gnu_file2 "$path/throughput vs bttlnkRate.png" "Throughput VS Bottleneck Link Capacity" "Bottleneck Link Capacity (Mbps)" "$path/data_1.txt"  1 3 4 "$method"
gnuplot -c $gnu_file3 "$path/fairness index vs bttlnkRate.png" "Fairness Index VS Bottleneck Link Capacity" "Bottleneck Link Capacity (Mbps)" "Fairness Index" "$path/data_1.txt" 1 5 "newreno + ${method}"

# # # task 
# doesnt work for i=1 or loss ratio 0.1
for i in {2..6}
do
    ./ns3 run "scratch/1905034_A.cc --tcp2=${tcp} --nLeaf=$2 --packetLossRate=${i} --output_folder=${path} --exp=2">> "${path}/${method}.txt" 2>&1
    # output file name | file 1 | file1_1stcol | file1_2ndcol | title 1 | file2 | file2_2ndcol | title 2
    gnuplot -c $gnu_file1 "$path/cwnd_loss_rate_${i}.png" "${path}/flow1.cwnd" 1 2 "newreno" "${path}/flow2.cwnd" 2 ${method}
    echo "loss rate exp -$i done"
done


# output file name | title | x label | file 1 | file1_1stcol | file1_2ndcol | file1_3rdcol | title 2
gnuplot -c $gnu_file2 "$path/throughput vs packet loss rate.png" "Throughput VS Packet Loss Rate" "Packet Loss Rate (%) Exponent" "$path/data_2.txt" 2 3 4 ${method}
gnuplot -c $gnu_file3 "$path/fairness index vs packet loss rate.png" "Fairness Index VS Packet Loss Rate" "Packet Loss Rate (%) Exponent" "Fairness Index" "$path/data_2.txt" 2 5 "newreno + ${method}"