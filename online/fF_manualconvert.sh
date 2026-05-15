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
Run1025
Run1026
Run1027
Run1028
Run1029
Run1030
Run1031
Run1035
Run1036
Run1040
Run1042
Run1043
Run1045
Run1048
Run1049
Run1050
Run1054
Run1055
Run1056
Run1057
Run1058
Run1059
Run1060
Run1061
Run1066
Run1067
Run1068
Run1069
Run1070
Run1071
Run1072
Run1073
Run1074
Run1075
Run1076
Run1077
Run1078
Run1079
Run1080
Run1081
Run1082
Run1083
Run1085
Run1086
Run1087
Run1089
Run1090
Run1092
Run1093
Run1094
Run1095
Run1096
Run1098
Run1099
Run1100
Run1101
Run1102
Run1103
Run1104
Run1105
Run1106
Run1107
Run1108
Run1110
Run1112
Run1113
Run1116
Run1117
Run1119
Run1120
Run1121
Run1122
Run1123
Run1124
Run1125
Run1126
Run1127
Run1129
Run1130
Run1131
Run1132
Run1133
Run1134
Run1135
Run1136
Run1137
Run1138
Run1139
Run1141
Run1146
Run1147
Run1151
Run1152
Run1155
Run1156
Run1158
Run1159
Run1160
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
		    # TBC, right now I had to set it manually!
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
		    limnamestr=11
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
