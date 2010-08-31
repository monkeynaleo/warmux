#!/bin/sh
if [ "$1" = "" ]; then
    echo "Usage: $0 /path/to/wormux <wormux options>"
    exit
fi

LOG=valgrind.log
valgrind --tool=memcheck --show-reachable=yes --log-file=$LOG \
    --leak-check=yes --leak-resolution=high --num-callers=12 \
    --suppressions=$(dirname $0)/wormux.supp $*
