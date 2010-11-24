#!/bin/sh
if [ "$1" = "" ]; then
    echo "Usage: $0 /path/to/warmux <warmux options>"
    exit
fi

GLIBCXX_FORCE_NEW=1 valgrind \
  --tool=memcheck --show-reachable=yes --log-file=valgrind.log \
  --leak-check=full --leak-resolution=high --num-callers=12 \
  --suppressions=$(dirname $0)/warmux.supp $* -f
