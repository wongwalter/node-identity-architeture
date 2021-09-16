#!/bin/sh

# 1. Select host
# 2. Test conectivity
# 3. 
#

# Redirect stderr to /dev/null
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
	  default=2000:1:8::5
	  
	  status_eth0=$(ip -6 address show dev eth0 | grep eth0)
	  for i in $status_eth0 ; do
	      
	      if [ $i=$eth0 ] ; then
		  echo "eth0 ok!"
	      else
		  wrong_addr=$(ip -6 address show dev eth0 | grep 2000)
		  for j in $wrong_addr ; do
		      tmp=$(echo $j | grep 2000)
		      if [ $tmp ] ; then
			  echo $tmp
		      fi
		  done
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