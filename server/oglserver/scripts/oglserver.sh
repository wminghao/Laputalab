#!/bin/sh
	
# description: ogl SDK server
# processname: ogl_server
PROG=ogl_server
PROG_HOME=/usr/share/oglserver
DAEMON=$PROG_HOME/$PROG
PIDFILE=$PROG_HOME/$PROG.pid

# Source function library
RETVAL=0

case "$1" in
  start)	
  	echo -n "Starting $PROG: "
	cd $PROG_HOME
    	$DAEMON >/dev/null 2>/dev/null &
    	RETVAL=$?
    	if [ $RETVAL -eq 0 ]; then
           echo $! > $PIDFILE
    	   touch $PROG_HOME/$PROG."lock"
    	fi
    	if [ $RETVAL -eq 0 ]; then 
	   echo "$PROG startup successful"
	else 
	   echo "$PROG startup failed"
        fi
    	echo
    	;;
  stop)
	echo -n "Shutting down $PROG: "
	pid=`cat $PIDFILE`
  	kill -9 $pid
  	RETVAL=$?
  	echo
        [ $RETVAL -eq 0 ] && rm -f $PROG_HOME/$PROG."lock" && rm -f $PIDFILE
  	;;
  restart)
	$0 stop
  	$0 start
  	;;
  status)
	status $PROG -p $PIDFILE
  	RETVAL=$?
  	;;
  *)
	echo $"Usage: $0 {start|stop|restart|status}"
  	RETVAL=1
  esac
exit $RETVAL
