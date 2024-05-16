#!/bin/bash
function stop_server {
    echo "Stopping server..."
    kill $(pgrep test_server)
}
function change_params(){
    while read line
    do

        my_arr[0]=$(echo $line | cut -d " " -f 1)
        my_arr[1]=$(echo $line | cut -d " " -f 2)
        my_arr[2]=$(echo $line | cut -d " " -f 3)
        my_arr[3]=$(echo $line | cut -d " " -f 4)

        q1="int L = "
        fenhao=";"
        q2="int N_pairs = pow(2,";
        rep1="$q1${my_arr[1]}$fenhao"
        rep2="$q2${my_arr[0]});"
        echo "$rep1"
        echo "$rep2"
        # 修改参数
        # replace=$a${my_arr[2]}$b${my_arr[3]}$c${my_arr[3]}$d
        a="const int C = ${my_arr[2]}, P1 = ${my_arr[3]}"
        b=", P2 = ${my_arr[3]}"
        
        replace="$a$b$fenhao"
        echo "$replace"
        sed -i "16c $replace" src/node.hpp
        sed -i "2c $rep1" src/client.cpp
        sed -i "3c $rep2" src/client.cpp
        cd build
        make
        sleep 1
        ./test_server &
        sleep 1
        ./test_omap_1
        stop_server
        cd ..
    done < params1.txt
}



change_params
