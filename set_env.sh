#!/bin/sh
brctl addbr br0
brctl addif br0 eth0
ip addr flush eth0
ifconfig br0 28.1.150.105
route add -net default gw 28.1.150.1
