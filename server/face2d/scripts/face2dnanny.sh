#!/bin/sh
case "$1" in
  start)	
	PYTHONPATH=/usr/share/face2d/pythonlib python /usr/share/face2d/Face2dServerNanny.py $1
    	;;
  stop)
	PYTHONPATH=/usr/share/face2d/pythonlib python /usr/share/face2d/Face2dServerNanny.py $1
  	;;
  restart)
	PYTHONPATH=/usr/share/face2d/pythonlib python /usr/share/face2d/Face2dServerNanny.py $1
  	;;
  *)
	echo $"Usage: $0 {start|stop|restart}"
  esac
