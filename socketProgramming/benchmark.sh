#! /bin/bash   
set -m

rm ./logs/*
make clean_resp > ./logs/make_clean_resp.log
make > ./logs/make.log 


for i in {1..200}; 
do 

  ./server_ > ./logs/server.log 2>&1 &
  serverPID=$!
  
  echo $i
  start="$(date +'%s.%N')"
  ./client_ ${i} > ./logs/client_${i}.log
  echo "$(date +"%s.%N - ${start}" | bc)" > ./response_times/times_${i}.txt

  kill  ${serverPID}

done


