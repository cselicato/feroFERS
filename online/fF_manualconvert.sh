#!/bin/bash

##########
# example:
# ./fF_manualconvert.sh /eos/experiment/newtile/beamtests/26_05_t10/fers_daq/bin/DataFiles 1 /eos/experiment/newtile/beamtests/26_05_t10/fers_root/splitted/ 0 0
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

#argument 4: if it is not 0, override runarray and run on the latest file
latestdefault=0
LATEST=${4:-$latestdefault} 

# argument 5: erase and redo whole run (0) or keep existing files (1)?
resetdefault=0
RESET=${5:-$resetdefault}

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

RUNSTRL=1
RUNSTRR=5  # should be 4 for Run1-9, 5 for Run 10-99, 6 for Run100-999, etcetera
runarray=(  # set run numbers here
$(ls -1 $INPATH/. | cut -c$RUNSTRL-$RUNSTRR | sort -r | uniq)
)

runarray=(
    Run279
    Run280
    Run290
    Run291
    Run292
    Run283
    Run286
    Run288
    Run289
    Run293
    Run305
    Run306
    Run307
    Run309
    Run310
    Run308
    Run311
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
    if [ $multifile -eq 0 ] ; then
        endfilenrs=0
    else
        endfilenrs=$(ls -1 $INPATH | grep $run | grep $INFMT | wc -l)
    fi
    for filenr in $(seq 0 $endfilenrs) ; do  # loop is needed for multi-file runs, in case of single-file (uncomment the proper lines below) same action is repeated for endfilenrs times (can be set to 1 in that case)
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

	# if requested, erase already created file (same run, same format) and redo it
	if [ $RESET -eq 1 ] ; then
	    if [ -f $OUTPATH/$finalname$INFMT.root ] ; then
	        echo "File already esists, moving on..."
	        continue
	    fi
	fi

	if [ $filenr -eq 0 ] ; then
	    if [ $multifile -eq 0 ] ; then
                ./main $INPATH/${finalname}_list$INFMT --output $OUTPATH/$finalname$INFMT.root
	    else
		./main $INPATH/$finalname.${filenr}_list$INFMT --output $OUTPATH/${finalname}_$filenr$INFMT.root
	    fi
	else
	    if [ $multifile -eq 0 ] ; then
	        ./main $INPATH/${finalname}_list$INFMT --output $OUTPATH/$finalname$INFMT.root
	    else
	        ./main $INPATH/$finalname.${filenr}_list$INFMT --output $OUTPATH/${finalname}_$filenr$INFMT.root --is-not-file-header 1 --input-ref-info $OUTPATH/${finalname}_0$INFMT.root
	    fi
	fi
    done

    echo "---"
    
done

echo "Done!"
