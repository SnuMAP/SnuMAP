#!/usr/bin/env python

import commands, subprocess, re, time, sys, csv
import datetime
from os.path import exists
import os
import itertools
import sys

number_re = "+([0-9]+\.[01-9]+|[0-9]+)"

commands.getoutput("cp $SNUMAP_ROOT/bin/gantt.py .")

input_file = "merge-log.csv"
input_stream = open(input_file, "w+")

for i in range(1, len(sys.argv), 1):
    input_ = open(sys.argv[i], "r")
    input_stream.write(input_.read())
    input_.close()

input_stream.close()

# generate gpl file
gplfile = "merge-log.gpl"
gpl_gen = "python gantt.py -i -o " + gplfile + " " + input_file
commands.getoutput(gpl_gen)
#input_delete =  "rm " + input_file
#commands.getoutput(input_delete)

# generate gantt chart by gnuplot with command line argument
epsfile = "output.eps"
gantt_gnuplot = "gnuplot -e " + "\"set terminal postscript eps color solid\"" + \
                " -e " + "\"set output '" + epsfile + "'\"" + \
                " -e " + "\"load '" + gplfile + "'\"" + \
                " -e " + "\"unset output\""

#print gantt_gnuplot
commands.getoutput(gantt_gnuplot)

gpl_delete = "rm " + gplfile
commands.getoutput(gpl_delete)

commands.getoutput("rm gantt.py")

commands.getoutput("epstopdf output.eps")
