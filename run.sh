#!/bin/sh

set -e
set -x

make
sudo sh -c 'echo stop > /sys/class/remoteproc/remoteproc1/state' || echo 'not able to stop'
sudo cp -v pru/gen/pru.out /lib/firmware/am335x-pru0-fw
sudo config-pin P9_31 pruout
sudo config-pin P9_29 pruin
sudo sh -c 'echo start > /sys/class/remoteproc/remoteproc1/state'
sudo sh -c 'echo a > /dev/rpmsg_pru30'
sudo host/read

