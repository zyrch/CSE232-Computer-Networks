#! /bin/bash   
set -m

rm ./logs/*
make clean_resp > ./logs/make_clean_resp.log
make > ./logs/make.log 

./server_ > ./logs/server.log 2>&1 &
serverPID=$!

for i in {1..1000}; 
do 
  echo $i
  start="$(date +'%s.%N')"
  ./client_ ${i} > ./logs/client_${i}.log
  echo "$(date +"%s.%N - ${start}" | bc)" > ./response_times/times_${i}.txt
done

kill -9 ${serverPID}

