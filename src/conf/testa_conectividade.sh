#!/bin/sh

echo "Testing ALFAROMEO"
eth0=10.1.1.2
eth1=10.1.2.2
eth2=10.1.7.2
eth3=10.1.8.2
for i in $eth0 $eth1 $eth2 $eth3 ; do
    ping -c 1 $i > /dev/null
    if [ $? -eq 0 ] ; then
	echo "Ping to $i ok!"
    else
	echo "Ping to $i failed!"
    fi
done

echo "Testing MASERATI"
eth0=10.1.4.5
eth1=10.1.5.5
eth2=10.1.8.5
eth3=10.1.9.5
for i in $eth0 $eth1 $eth2 $eth3 ; do
    ping -c 1 $i > /dev/null
    if [ $? -eq 0 ] ; then
	echo "Ping to $i ok!"
    else
	echo "Ping to $i failed!"
    fi
done

echo "Testing LAMBORGHINI"
eth0=10.1.5.6
eth1=10.1.6.6
eth2=10.1.11.19
for i in $eth0 $eth1 $eth2 ; do
    ping -c 1 $i > /dev/null
    if [ $? -eq 0 ] ; then
	echo "Ping to $i ok!"
    else
	echo "Ping to $i failed!"
    fi
done

echo "Testing FERRARI"
eth0=10.1.2.3
eth1=10.1.6.3
eth2=10.1.9.3
for i in $eth0 $eth1 $eth2 ; do
    ping -c 1 $i > /dev/null
    if [ $? -eq 0 ] ; then
	echo "Ping to $i ok!"
    else
	echo "Ping to $i failed!"
    fi
done

echo "Testing GILERA"
eth0=10.1.11.17
eth1=10.1.12.17
for i in $eth0 $eth1 ; do
    ping -c 1 $i > /dev/null
    if [ $? -eq 0 ] ; then
	echo "Ping to $i ok!"
    else
	echo "Ping to $i failed!"
    fi
done

