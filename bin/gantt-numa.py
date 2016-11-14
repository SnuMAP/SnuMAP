#!/usr/bin/env python

import commands, subprocess, re, time, sys, csv
import datetime
from os.path import exists
import os
import itertools
import sys

number_re = "+([0-9]+\.[01-9]+|[0-9]+)"

linegraph_gnuplot = "gnuplot -e "+"\"fontsize = 12\""+\
                    " -e "+"\"set terminal postscript enhanced eps font \'Times-New-Roman\' fontsize\""+\
                    " -e "+"\"set output \'output-numa.eps\'\""+\
                    " -e "+"\"set lmargin 5\""+\
                    " -e "+"\"set rmargin 5\""+\
                    " -e "+"\"set tmargin 0.5\""+\
                    " -e "+"\"set bmargin 2\""+\
                    " -e "+"\"set style data linespoints\""+\
                    " -e "+"\"set title \'Local DRAM Accesses Ratio\' font \'Times-New-Roman:Bold, 32\' offset 0,-3.8\""+\
                    " -e "+"\"set yrange [0:1]\""+\
                    " -e "+"\"set key font \'Times-New-Roman, 16\' \""+\
                    " -e "+"\"set xtics font \', 12\'\""+\
                    " -e "+"\"set ytics font \', 12\'\""+\
                    " -e "+"\"set xlabel \'Time (increase with jiffies)\' font \', 16\' offset 1.5,3.5\""+\
                    " -e "+"\"set datafile separator \',\'\""+\
                    " -e "+"\"plot \'"+sys.argv[1]+"\'"

# node local / remote accesses
gnuplot_command = linegraph_gnuplot
line_option_0 = "with lp lt 1 lw 0.5 ps 0.5 lc rgb \'#00008B\' pt 5"
line_option_1 = "with lp lt 1 lw 0.5 ps 0.5 lc 1 pt 7"
gnuplot_command += "using 2:xtic(1) " + line_option_0 + " ti \'Node 0\',\'\'"
gnuplot_command += "using 3:xtic(1) " + line_option_1 + " ti \'Node 1\'\""
gnuplot_command += " -e "+"\"unset output\""
print gnuplot_command
os.system(gnuplot_command)

commands.getoutput("epstopdf output-numa.eps")
