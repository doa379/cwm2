#! /bin/sh

MAKE=bmake

if [ "$1" = "dbg" ]; then
  $MAKE DEBUG=1
else
  $MAKE $@
fi
