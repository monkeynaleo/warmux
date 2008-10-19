#!/bin/sh

path=`dirname $0`
cd $path
path=`pwd`

r=0

# removing old stats files
rm -rf daily.csv hourly.csv

# get last index server logs
lftp -f get_stats_wormux.lftp
r=$?

if [ $r -ne 0 ]; then
    echo "ERROR $r: Fail to get last index server logs"
    exit 1;
fi

# generate new stats files
cat wormux_log/*hourly > hourly.csv
cat wormux_log/*daily > daily.csv

# send the new stats files
lftp -f send_stats_wormux.lftp
r=$?

if [ $r -ne 0 ]; then
    echo "ERROR $r: Fail to send network stats"
    exit 1;
fi