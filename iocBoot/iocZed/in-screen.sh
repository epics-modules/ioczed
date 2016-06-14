#!/bin/sh

# $Id: in-screen.sh 17504 2014-06-02 22:43:07Z kpetersn $

/usr/bin/screen -dm -S zzz -h 5000 ./run

# start the IOC in a screen session
#  type:
#   screen -r   to start interacting with the IOC command line
#   ^a-d        to stop interacting with the IOC command line
#   ^c          to stop the IOC
