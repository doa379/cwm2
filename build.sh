#! /bin/sh

SYSNAME=$(uname -a)
mkdir -p obj

if [ "$(echo $SYSNAME | grep -c BSD)" = "1" ]
then
  echo "Build for BSD system"
  if [ "$1" = "debug" ]
  then
    make -f make_bsd DEBUG=1
    exit 0
  else
    make -f make_bsd $@
  fi

elif [ "$(echo $SYSNAME | grep -c GNU)" = "1" ]
then
  echo "Build for GNU system"
  if [ "$1" = "debug" ]
  then
    make -f make_gnu debug
    exit 0
  else
    make -f make_gnu $@
  fi
fi

