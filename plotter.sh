# This script is a wrapper around plotter.c that changes the function for y',
# recompiles plotter.c, makes a dir for the data if necessary, deletes any old data, and
# cats the seperate forward and backward files together. The reason it doesn't
# also run gnuplot is that loading scripts takes gnuplot out of interactive mode,
# which means I can't set the scale of the plot for nice viewing.

#! /bin/bash

if [[ $# -ne 8 ]]; then
    echo "usage: ./plotter \"<function>\" <xmin> <xmax> <ymin> <ymax> <rows> <cols> <stepsize>"
    echo "<function> must be a valid C expression in x and y using math.h like cos(M_PI*x) + sin(M_PI*y)"
    exit 1
fi

functiondef=$(echo $1 | sed 's/\//\\\//g' | sed 's/,/\\,/g') #this changes / from division to \/ for the next line
sed -i 's,\(#define FUNCTION\)\s.*,\1 '"$functiondef"',g' plotter.c
gcc plotter.c -o plotter -lm -pthread

if [ -d "curves" ]; then
    rm curves/*
else
    mkdir curves
fi

./plotter $2 $3 $4 $5 $6 $7 $8

for row in `seq 0 $(($6 + 1))`;
do
    mv "curves/curve_${row}_0f.dat" "curves/curve_${row}_0.dat"
done

for row in `seq 0 $(($6 + 1))`;
do
    for col in `seq 1 $7`;
    do curve="curves/curve_${row}_${col}.dat"

	mv "curves/curve_${row}_${col}f.dat" $curve
	echo "" >> $curve
	cat "curves/curve_${row}_${col}b.dat" >> $curve
	rm "curves/curve_${row}_${col}b.dat"
    done
done

for row in `seq 0 $(($6 + 1))`;
do
    mv "curves/curve_${row}_$(($7 + 1))b.dat" "curves/curve_${row}_$(($7 + 1)).dat"
done
