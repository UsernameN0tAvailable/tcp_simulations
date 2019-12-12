#!/bin/bash

cpt=1

while [ $cpt -lt 17 ]
do
	declare -a results
	results=()

	duration=30

	while [ $duration -lt 481 ]
	do

	    rm log*

        if [ $2 -ne 2 ]
        then

            cmd='(NS_GLOBAL_VALUE="RngRun=0" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --ber='$1' --mode='$2'")2>log0ber'$1'.txt & '
		    cmd=$cmd'(NS_GLOBAL_VALUE="RngRun=1" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --ber='$1' --mode='$2'")2>log1ber'$1'.txt & '
            cmd=$cmd'(NS_GLOBAL_VALUE="RngRun=2" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --ber='$1' --mode='$2'")2>log2ber'$1'.txt & '
		    cmd=$cmd'(NS_GLOBAL_VALUE="RngRun=3" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --ber='$1' --mode='$2'")2>log3ber'$1'.txt &
		    wait'
		else
		    cmd='(NS_GLOBAL_VALUE="RngRun=0" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --per='$1' --mode='$2'")2>log0ber'$1'.txt & '
		    cmd=$cmd'(NS_GLOBAL_VALUE="RngRun=1" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --per='$1' --mode='$2'")2>log1ber'$1'.txt & '
            cmd=$cmd'(NS_GLOBAL_VALUE="RngRun=2" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --per='$1' --mode='$2'")2>log2ber'$1'.txt & '
		    cmd=$cmd'(NS_GLOBAL_VALUE="RngRun=3" ./waf --run "SatSim --cpt='$cpt' --duration='$duration' --per='$1' --mode='$2'")2>log3ber'$1'.txt &
		    wait'
        fi


		eval $cmd


		re='^[0-9]+([.][0-9]+)?$'
		i=0


		while [ $i -lt 4 ]
		do

		    val=$(head -n 1 "log"$i"ber"$1.txt)

            if  [[ $val =~ $re ]]
            then
                results+=($val)
            fi
            i=$[$i+1]
		done


	    duration=$[$duration+$duration]
	done


	#average the data and put it into a log file
	counter=0
	tot=0.0
	for o in "${results[@]}"
	do
		counter=$[$counter+1]
		tot=$(awk "BEGIN {print $tot+$o; exit}")
	done

	avg=$(awk "BEGIN {print $tot/$counter; exit}")

	yo="mode="$2"_ber="$1"_cpt="$cpt".txt"

	echo $avg > "$yo"

	cpt=$[$cpt+3]
done
