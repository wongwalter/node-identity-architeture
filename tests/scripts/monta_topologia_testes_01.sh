#!/bin/sh

# 1. Select host
# 2. Test conectivity
# 3. 
#

# Redirect stderr to /dev/null
exec 2>/dev/null

while :
  do
  clear
  echo "-------------------------------------------------"
  echo "-------------Setup test topology 01 -------------"
  echo "This utility must be run on the host to be set up"
  echo "or to be reachable to the host."
  echo "-------------------------------------------------"
  echo "[1] ALFAROMEO"
  echo "[2] MASERATI"
  echo "[3] FERRARI"
  echo "[4] DUCATI"
  echo "[5] LAMBORGHINI"
  echo "[6] GILERA"
  echo "[7] EXIT"
  echo "-------------------------------------------------"
  echo "Select menu [1-7:]"
  read choice
  case $choice in
      1) echo "Configuring ALFAROMEO"
	  eth0=2000:1:1::2
	  eth1=2000:1:2::2
	  eth2=2000:1:7::2
	  eth3=2000:1:8::2
	  routes=2000:1:8::5
	  
	  list="$eth0 $eth1 $eth2 $eth3"
	  status=$(ifconfig | grep inet6)
	  for i in $status ; do
	      tmp=$(echo $i | grep 2000)
	      if [ $tmp ] ; then
		  addresses="$addresses+$tmp"
	      fi
	  done
	  
	  echo "Checking IPv6 addresses"
	  for j in $list ; do
	      tmp=$(echo $addresses | grep $j)
	      if [ $tmp ] ; then
		  echo "$j present!"
	      else
		  echo "missing $j"
	      fi
	  done
	  echo "Ok! Checking routes"
	  get_routes=$(ip -6 route | grep default)
	  for i in $get_routes ; do
	      tmp=$(echo $i | grep 2000)
	      if [ $tmp ] ; then
		  if [ $tmp=$routes ] ; then
		      echo "Default route set to $tmp"
		  fi
	      fi
	  done
	  echo "Done"
	  exit 0 ;;
      2)
	  exit 0 ;;	  	 
      3)
	  exit 0 ;;
      4)
	  exit 0 ;;
      5)
	  exit 0 ;;
      6)
	  exit 0 ;;
      7)
	  echo "Exit!"
	  exit 0 ;;
      *) echo "Select a valid option: "; read ;;
  esac
done