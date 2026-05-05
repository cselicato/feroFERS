#!/bin/bash

##########
# example:
# ./fF_manualconvert.sh /eos/experiment/newtile/beamtests/26_05_t10/fers_daq/bin/DataFiles 1 /eos/experiment/newtile/beamtests/26_05_t10/fers_root/splitted/ 0 0
##########

# argument 1: path to input data (mandatory)
INPATH=$1

# argument 2: input data format - binary (0) or CSV (1)
idinfmtdefault=0
IDINFMT=${2:-$idinfmtdefault}

# argument 3: path to output data
outpathdefault=.
OUTPATH=${3:-$outpathdefault}

#argument 4: if it is not 0, override runarray and run on the latest file (unless argument 7 is not RunX, see below)
latestdefault=0
LATEST=${4:-$latestdefault} 

# argument 5: erase and redo whole run (0) or keep existing files (1)?
resetdefault=0
RESET=${5:-$resetdefault}

# argument 6: multi-file (1) or single-file (0) format?
multifiledefault=1
MULTIFILE=${6:-$multifiledefault}

# argument 7: if LATEST (argument 4) is not 0 and this is not RunX, this will be the run nr that will be processed (e.g Run123)
overriderundefault=RunX
OVERRIDERUN=${7:-$overriderundefault}

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
    Run334
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
	if [ "$OVERRIDERUN" = "RunX" ] ; then
            echo "Iteration on latest run"
	else
            echo "Iteration on selected run $OVERRIDERUN"
	fi
    fi

    echo "---"
    
    # work on the run - core operations are performed in here
    if [ $MULTIFILE -eq 0 ] ; then
        endfilenrs=0
    else
	if [ $LATEST -eq 0 ] ; then
            endfilenrs=$(ls -1 $INPATH | grep $run | grep $INFMT | wc -l)
	else
	    if [ "$OVERRIDERUN" = "RunX" ] ; then
	        latestrun=$(ls -1rt $INPATH | grep .0_list$INFMT | tail -n 1 | sed -e "s/\(.*\).0_list$INFMT/\1/")
	        endfilenrs=$(ls -1 $INPATH | grep $latestrun | grep $INFMT | wc -l)
		(( endfilenrs = $endfilenrs - 2 ))
            else
                endfilenrs=$(ls -1 $INPATH | grep $OVERRIDERUN | grep $INFMT | wc -l)
	    fi
	fi
    fi
    for filenr in $(seq 0 $endfilenrs) ; do  # loop is needed for multi-file runs, in case of single-file same action is repeated for endfilenrs times (set to 0 above)
        if [ $LATEST -eq 0 ] ; then
            finalname=$run
	else
	    if [ "$OVERRIDERUN" = "RunX" ] ; then
                latestname0=$(ls -rt $INPATH | grep $INFMT | tail -n 1)
		if [ $MULTIFILE -ne 0 ] ; then
                    #if [ $filenr -lt 10 ] ; then
                    #    limnamestr=12
                    #elif [ $filenr -lt 100 ] ; then
                    #    limnamestr=12
                    #elif [ $filenr -lt 1000 ] ; then
                    #    limnamestr=14
                    #elif [ $filenr -lt 10000 ] ; then
	            #    limnamestr=15
                    #elif [ $filenr -lt 100000 ] ; then
                    #    limnamestr=16
                    #fi
		    limnamestr=12
		else
                    limnamestr=9
		fi
	        finalname=${latestname0::-$limnamestr}
	    else
                finalname=$OVERRIDERUN
	    fi
        fi

	# if requested, erase already created file (same run, same format) and redo it
	if [ $RESET -eq 1 ] ; then
	    if [ $MULTIFILE -eq 0 ] ; then
	        if [ -f $OUTPATH/$finalname$INFMT.root ] ; then
	            echo "File $finalname$INFMT.root already esists, moving on..."
	            continue
	        fi
	    else
	        if [ -f $OUTPATH/${finalname}_$filenr$INFMT.root ] ; then
                    echo "File ${finalname}_$filenr$INFMT.root already esists, moving on..."
                    continue
                fi
	    fi
	fi

	if [ $filenr -eq 0 ] ; then
	    if [ $MULTIFILE -eq 0 ] ; then
                ./main $INPATH/${finalname}_list$INFMT --output $OUTPATH/$finalname$INFMT.root
	    else
		./main $INPATH/$finalname.${filenr}_list$INFMT --output $OUTPATH/${finalname}_$filenr$INFMT.root
	    fi
	else
	    if [ $MULTIFILE -eq 0 ] ; then
	        ./main $INPATH/${finalname}_list$INFMT --output $OUTPATH/$finalname$INFMT.root
	    else
	        ./main $INPATH/$finalname.${filenr}_list$INFMT --output $OUTPATH/${finalname}_$filenr$INFMT.root --is-not-file-header 1 --input-ref-info $OUTPATH/${finalname}_0$INFMT.root
	    fi
	fi
    done

    echo "---"
    
done

echo "Done!"
