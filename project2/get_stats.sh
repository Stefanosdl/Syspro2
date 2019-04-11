#!/bin/bash

raise_error() {
  local error_message="$@"
  echo "${error_message}" 1>&2;
}

numOfClients=0;
max=1;
min=1;
numOfWriteBytes=0;
numOfReadBytes=0;
numOfLeft=0;
numOfWrite=0;
numOfRead=0;
declare -a array1
while read -a array; do
  if [[ "${array[0]}" == "My" ]]; then
    numOfClients=$(($numOfClients + 1));
    array1+=(${array[1]})
    if [[ ${array[1]} > $max ]]; then
      max=${array[1]}
    elif [[ ${array[1]} < $min ]]; then
      min=${array[1]}
    fi
  elif [[ "${array[0]}" == "Write" ]]; then
    numOfWriteBytes=$(($numOfWriteBytes + ${array[1]}));
    numOfWrite=$(($numOfWrite + 1));
  elif [[ "${array[0]}" == "Read" ]]; then
    numOfReadBytes=$(($numOfReadBytes + ${array[1]}));
    numOfRead=$(($numOfRead + 1));
  elif [[ "${array[0]}" == "Left" ]]; then
    numOfLeft=$(($numOfLeft + 1));
  fi
done

echo "Number of clients connected: $numOfClients"
for i in "${array1[@]}"; do
  echo "Client id: $i"
done
echo "Max client id: $max"
echo "Min client id: $min"
echo "Number of bytes Written: $numOfWriteBytes"
echo "Number of bytes Read: $numOfReadBytes"
echo "Number of files Written: $numOfWrite"
echo "Number of files Read: $numOfRead"
echo "Number of clients Left: $numOfLeft"
