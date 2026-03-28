#!/bin/bash

##########
# example:
# ./fF_mergefiles.sh 38 /eos/experiment/newtile/beamtests/26_05_t10/fers_root/splitted 0 /eos/experiment/newtile/beamtests/26_05_t10/fers_root/merged
##########

# argument 1: run name (mandatory)
RUNNR=$1

# argument 2: path to input data (mandatory)
INPATH=$2

# argument 3: input data format - binary (0) or CSV (1)
idinfmtdefault=0
IDINFMT=${3:-$idinfmtdefault}

# argument 4: path to output data
outpathdefault=.
OUTPATH=${4:-$outpathdefault}

if [ $IDINFMT -eq 0 ] ; then
    INFMT=.dat
else
    INFMT=.csv
fi

#echo $(find $INPATH -name "Run${RUNNR}_*$INFMT.root")
hadd -f $OUTPATH/Run$RUNNR$INFMT.root $(find $INPATH -name "Run${RUNNR}_*$INFMT.root")
