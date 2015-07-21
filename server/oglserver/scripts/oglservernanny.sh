#!/bin/sh
case "$1" in
  start)	
	PYTHONPATH=/usr/share/oglserver/pythonlib python /usr/share/oglserver/OglServerNanny.py $1
    	;;
  stop)
	PYTHONPATH=/usr/share/oglserver/pythonlib python /usr/share/oglserver/OglServerNanny.py $1
  	;;
  restart)
	PYTHONPATH=/usr/share/oglserver/pythonlib python /usr/share/oglserver/OglServerNanny.py $1
  	;;
  *)
	echo $"Usage: $0 {start|stop|restart}"
  esac
