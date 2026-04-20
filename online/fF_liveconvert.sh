#!/bin/bash

##########
# example:

##########

multifile=1 # set here whether the acquisition mode is single-file (0) or multi-file (1)

# argument 1: path to input data (mandatory)
INPATH=$1

# argument 2: input data format - binary (0) or CSV (1)
idinfmtdefault=0
IDINFMT=${2:-$idinfmtdefault}

# argument 3: path to output data
outpathdefault=.
OUTPATH=${3:-$outpathdefault}

# argument 4: erase and redo whole run (0) or keep existing files (1)?
resetdefault=0
RESET=${4:-$resetdefault}

if [ $IDINFMT -eq 0 ] ; then
    INFMT=.dat
else
    INFMT=.csv
fi

echo "Starting live ROOT file creation"
echo "The input data path folder is:"
echo $INPATH "(format $INFMT)"
echo "The output data path folder is:"
echo $OUTPATH
echo "Kill the process to interrupt"
echo "---"

i=0
while true
do

trap "exit" SIGINT

echo "Iteration number $i"
i=$(($i + 1))

echo "---"

runlastlast=$(ls -1rt $INPATH | grep .0_list$INFMT | tail -n 1 | sed -e "s/\(.*\).0_list$INFMT/\1/")
runlastprev=$(ls -1rt $INPATH | grep .0_list$INFMT | tail -n 2 | head -n 1 | sed -e "s/\(.*\).0_list$INFMT/\1/")





echo "Found last 2 runs: "

echo "---"

sleep 1

done
