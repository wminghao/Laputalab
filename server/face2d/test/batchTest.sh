#!/bin/sh

a=0
while [ $a -lt 5 ]
do
   echo "launching instances"+$a
   ./launchSingleClient.sh &
   a=`expr $a + 1`
done

sleep 1

b=0
while [ $b -lt 5 ]
do
   echo "launching instances"+$b
   ./launchSingleClient.sh &
   b=`expr $b + 1`
done
