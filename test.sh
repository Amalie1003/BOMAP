#!/bin/bash
set -e # 如果运行过程中出现报错，脚本停止

# 停止test_server进程
function stop_server {
    echo "Stopping server..."
    kill $(pgrep test_server)
}
function change_params(){
    while read line  # 按行读param1.txt
    do

        my_arr[0]=$(echo $line | cut -d " " -f 1)
        my_arr[1]=$(echo $line | cut -d " " -f 2)
        my_arr[2]=$(echo $line | cut -d " " -f 3)
        my_arr[3]=$(echo $line | cut -d " " -f 4) #这四个操作是在切分读的param1.txt每行，每行的格式是：1 2 3 4

        q1="int L = "  #注意赋值符号前后都不能有空格
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
        echo "$replace"   # Line18-28都是在构造要修改的语句
        sed -i "16c $replace" src/node.hpp  # （sed文件编辑操作，c表示change）将src/node.hpp的第16行修改成replace对应的字符串
        sed -i "2c $rep1" src/client.cpp
        sed -i "3c $rep2" src/client.cpp
        cd build # 从主文件夹目录 cd到build
        make  
        sleep 1
        ./test_server &
        sleep 1
        ./test_bomap  # 我要实现的是先test_server 然后启动test_bomap建立通信，&表示同时运行吧
        stop_server # 上面的运行完之后 kill掉进程（test_bomap自动结束，test_server需要手动kill）
        cd ..  # 回退继续循环
    done < params1.txt
}



change_params #主函数
