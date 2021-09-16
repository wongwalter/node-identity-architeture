#!/bin/sh
var1=$(ifconfig dummy0 | grep 'inet6')
count=0
for i in $var1
do
  var2=$(echo $i | grep '2001')
  if [ $var2 ]
      then
      if [ $count -lt 3 ]
	  then
	  echo "Removing $var2 ..."
	  ip -6 a d $var2 dev dummy0
	  count=`expr $count + 1`
      else
	  echo "Leaving HIT=$var2"
	  exit 1
      fi
  fi
done
