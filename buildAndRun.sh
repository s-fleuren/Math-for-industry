#!/bin/sh

# Shell script to compile and run an OMP parallel demonstration program.
# build.sh (C) 2018 by:
#   Scientific Volume Imaging Holding B.V.
#   Laapersveld 63,
#   1213 VB Hilversum,
#   The Netherlands,
#   email: info@svi.nl

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Define the compiler, do medium optimization, enable many warnings, and
# enable OpenMP. 

CC="gcc"
CFLAGS="-O2 -Wall -Wextra -fopenmp"
SRCNAME="example_basic.c"
EXENAME="./example_basic"

# Remove the executable if an old version is still present.
if [ -f $EXENAME ]; then
    rm $EXENAME
fi

# Invoke the compiler. 
$CC -o $EXENAME $CFLAGS $SRCNAME

# If a new executable was made, run it.
if [ -f $EXENAME ]; then
    $EXENAME
fi

