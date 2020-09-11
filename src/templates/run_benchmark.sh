#!/bin/sh

#---------------------------------------------------------------------
#
# Copyright IRISA, 2003-2017
#
# This file is part of Heptane, a tool for Worst-Case Execution Time (WCET)
# estimation.
# APP deposit IDDN.FR.001.510039.000.S.P.2003.000.10600
#
# Heptane is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Heptane is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details (COPYING.txt).
#
# See CREDITS.txt for credits of authorship
#
#---------------------------------------------------------------------

if [ $# -lt 1 ] || [ $# -gt 4 ]
then
    echo "usage: $0 benchmark_name arch solver [-q]"
    echo "   -q for quiet "
    exit 1
fi


if [ $# == 4 ]
    then
    if [ $4 == "-q" ]
	then
	./extract.sh $1 $2 
	./analysis.sh $1 $2 $3 -t
    else
	echo "usage: $0 benchmark_name arch solver [-q]"
	exit 1
    fi
else
    ./extract.sh $1 $2 "-v"
    ./analysis.sh $1 $2 $3
fi
