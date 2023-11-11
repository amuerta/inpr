#!/bin/bash
NAME="inpr"
TARGET="main"

clang -o $NAME $TARGET.c -Wall -g

if [[ $(ls | grep "$NAME") ]]; then
  echo "[COMPILED SUCCESFULLY]"
fi

if [[ $1 == "run" ]]; then
  ./$NAME ${@:2}
fi
