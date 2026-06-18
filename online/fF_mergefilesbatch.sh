#!/bin/bash

##########
# example:
# 
##########

# argument 1: path to input data (mandatory)
INPATH=$1

# argument 2: input data format - binary (0) or CSV (1)
idinfmtdefault=0
IDINFMT=${2:-$idinfmtdefault}

# argument 3: path to output data
outpathdefault=.
OUTPATH=${3:-$outpathdefault}

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

# loop on all the requested runs
for run in "${runarray[@]}" ; do
  echo "Merging Run${run}..."
  ./fF_mergefilessingle.sh ${run:3} $INPATH $IDINFMT $OUTPATH
  echo "Done!"
  echo ""
done
