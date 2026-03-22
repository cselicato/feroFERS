#!/bin/bash

# argument 1: path to input data (mandatory)
INPATH=$1

# argument 2: input data format - binary (0) or CSV (1)
idinfmtdefault=0
IDINFMT=${2:-$idinfmtdefault}

# argument 3: path to output data
outpathdefault=.
OUTPATH=${3:-$outpathdefault}

#### argument 4: erase and redo whole run (0) or keep existing files (1)?
###resetdefault=0
###RESET=${4:-$resetdefault}

#argument 4: if it is not 0, override runarray and run on the latest file
latestdefault=0
LATEST=${4:-$latestdefault}

if [ $IDINFMT -eq 0 ] ; then
    INFMT=.dat
else
    INFMT=.csv
fi

echo "Starting ROOT file creation on manually selected runs"
echo "The input data path folder is:"
echo $INPATH "(format $INFMT)"
echo "The output data path folder is:"
echo $OUTPATH
echo "Kill the process to interrupt"
echo "---"

# run list (set)
# ---------------
# to manually select runs:
# runarray=(  # set run numbers here
# 100000 100001 100002
# )
# alternatively, to process all runs, you can try: 
# RUNSTRL=1
# RUNSTRR=10
# runarray=(
# $(ls -1 $actualpath/data_ascii/. | cut -c$RUNSTRL-$RUNSTRR | sort -r | uniq)
# )
# ---------------
runarray=(  # set run numbers here
Run17
)

# if requested, overwrite runarray to run on the latest file
if [ $LATEST -ne 0 ] ; then
    runarray=(-1)
fi

# loop on all the requested runs
for run in "${runarray[@]}" ; do

    if [ $LATEST -eq 0 ] ; then
        echo "Iteration on run $run"
    else
        echo "Iteration on latest run"
    fi

    echo "---"

    # work on the run - core operations are performed in here
    #endfilenrs=0
    endfilenrs=20
    for filenr in $(seq 0 $endfilenrs) ; do
        if [ $LATEST -eq 0 ] ; then
            finalname=$run
	else
            latestname0=$(ls -rt $INPATH | grep $INFMT | tail -n 1)
   
            #finalname=${latestname0::-9}
            if [ $filenr -lt 10 ] ; then
                limnamestr=11
	    else
	        limnamestr=12
	    fi
	    finalname=${latestname0::-$limnamestr}
        fi

	if [ $filenr -eq 0 ] ; then
            #./main $INPATH/${finalname}_list$INFMT --output $OUTPATH/$finalname.root
            ./main $INPATH/$finalname.${filenr}_list$INFMT --output $OUTPATH/${finalname}_$filenr.root ###$RESET
	else
	    #./main $INPATH/${finalname}_list$INFMT --output $OUTPATH/$finalname.root
	    ./main $INPATH/$finalname.${filenr}_list$INFMT --output $OUTPATH/${finalname}_$filenr.root --is-not-file-header 1 --input-ref-info $OUTPATH/${finalname}_0.root ###$RESET
	fi
    done

    echo "---"
    
done

echo "Done!"
