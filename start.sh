#!/bin/sh
make
./odv &
sleep 2
ifconfig tap0 up
brctl addif br0 tap0
