#!/bin/bash

##########
# example:
# ./fF_liveconvert.sh /eos/experiment/newtile/beamtests/26_05_t10/fers_daq/bin/DataFiles 0 /eos/experiment/newtile/beamtests/26_05_t10/fers_root/splitted 1
##########

# argument 1: path to input data (mandatory)
INPATH=$1

# argument 2: input data format - binary (0) or CSV (1)
idinfmtdefault=0
IDINFMT=${2:-$idinfmtdefault}

# argument 3: path to output data
outpathdefault=.
OUTPATH=${3:-$outpathdefault}

# argument 4: multi-file (1) or single-file (0) format?
multifiledefault=1
MULTIFILE=${4:-$multifiledefault}

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
echo "==="

i=0
while true
do

trap "exit" SIGINT

echo "Iteration number $i"
i=$(($i + 1))

if [ $MULTIFILE -eq 0 ] ; then
    # TBC, currently this doesn't separate correctly single-file from multi-file if single-file mode is wanted
    runlastlast=$(ls -1rt $INPATH | grep "_list$INFMT" | tail -n 1 | sed -e "s/\(.*\)._list$INFMT/\1/")
    runlastprev=$(ls -1rt $INPATH | grep "_list$INFMT" | tail -n 2 | head -n 1 | sed -e "s/\(.*\)_list$INFMT/\1/")
else
    runlastlast=$(ls -1rt $INPATH | grep "\.0_list$INFMT" | tail -n 1 | sed -e "s/\(.*\).0_list$INFMT/\1/")
    runlastprev=$(ls -1rt $INPATH | grep "\.0_list$INFMT" | tail -n 2 | head -n 1 | sed -e "s/\(.*\).0_list$INFMT/\1/")
fi
echo "Found last 2 runs: $runlastprev , $runlastlast"

echo "==="

OUTPATHMERGED=$OUTPATH/../merged

# run before the latest (only once, the first time a new run is processed)
# --> first execution rewrites all previously created ROOT files
# --> second execution to make sure as many raw files as possible are converted (TBC because of memory leak)
# --> then create the corresponding merged file
if [ -f "$OUTPATHMERGED/$runlastprev$INFMT.root" ] ; then
    echo "$runlastprev (the one before the latest) already fully converted and merged --> skipping"
else
    echo "Converting files of $runlastprev (the one before the latest)..."
    echo "==="
    ./fF_manualconvert.sh $INPATH $IDINFMT $OUTPATH 1 0 $MULTIFILE $runlastprev
    ./fF_manualconvert.sh $INPATH $IDINFMT $OUTPATH 1 1 $MULTIFILE $runlastprev

    echo "Now merging run $runlastprev (the one before the latest)..."
    echo "==="
    ./fF_mergefiles.sh ${runlastprev:3} $OUTPATH $IDINFMT $OUTPATHMERGED
fi

echo "==="

# very latest run --> simply convert existing raw files
echo "Now converting files of $runlastlast (very latest)..."
echo "==="
./fF_manualconvert.sh $INPATH $IDINFMT $OUTPATH 1 1 $MULTIFILE $runlastlast

echo "==="

sleep 1

done
