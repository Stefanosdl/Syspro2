#!/bin/bash

raise_error() {
  local error_message="$@"
  echo "${error_message}" 1>&2;
}

declare -a array1
argument="$4"
if [[ -z $argument ]] ; then
  raise_error "Missing arguments"
else
  dirs="$3"
  #create the directories
  array1+=($1)
  while ((dirs>0)); do
    rDir=$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w $((1+($RANDOM%10)))| head -n 1)
    #if we can create directories for all the levels given
    if ((dirs >= $4)); then
      i=1
      newDir="$rDir"
      array1+=($1/$newDir)
      while ((i<$4)); do
        #get a new directory
        rDir=$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w $((1+($RANDOM%10)))| head -n 1)
        newDir="$newDir/$rDir"
        array1+=($1/$newDir)
        i=$((i+1))
      done
      mkdir -p "$1/$newDir"
    #now create the remaining directories
    else
      rDir=$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w $((1+($RANDOM%10)))| head -n 1)
      if ((dirs > 1)); then
        newDir="$rDir"
        array1+=($1/$newDir)
        #create the remaininh directories
        for ((j=1;j<$dirs;j++)); do
          rDir=$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w $((1+($RANDOM%10)))| head -n 1)
          newDir="$newDir/$rDir"
          array1+=($1/$newDir)
        done
        mkdir -p "$1/$newDir"
      #if there is only 1 directory left to create
      else
        array1+=($1/$rDir)
        mkdir -p "$1/$rDir"
      fi
    fi
    dirs=$((dirs-$4))
  done
  j=0
  for ((i=0; i<$2; i++)); do
    if ((j == ${#array1[@]})); then
      j=0
    fi
    if ((j != ${#array1[@]})); then
      txtName=$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w $((1+($RANDOM%10)))| head -n 1)
      txtSize=$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w $((1+($RANDOM%128000)))| head -n 1)
      echo $txtSize>${array1[$j]}/$txtName
      j=$((j+1))
    fi
  done
fi
