#!/usr/bin/env python

import os
import signal
import getopt
import sys
import StringIO
import traceback
import string
import re
import time
import grp
import subprocess
import syslog
import httplib

from interface import Interface
from daemon import Daemon
from mail import Mail

#setting
pidFileName = '/tmp/oglserver-nanny.pid'

#server script
OglServerInitdScript = "/etc/init.d/oglserver.sh"
OglStatusListeningPort = 1235
queryInterval = 1
maxNonresponsiveCount = 5

#import daemon.pidlockfile 
class OglServerNanny(Daemon):

    def __init__( self, pidfile):
        Daemon.__init__(self, pidfile)
        self.nonresponsiveCount_ = 0
        
    def run( self ):
        syslog.syslog('OglServerNanny running!')
        self._restartOglServerProcess()

        start = time.time()
        while True:
            exceptionTraceout = StringIO.StringIO()
            try:
                now = time.time()
                if now - start > queryInterval:
                    self._queryOglServerStat()
                    start = now
            except:
                traceback.print_exc( file = exceptionTraceout )
            time.sleep( 0.1 )

    def start( self ):
        #daemon start will automatically start the Oglserver
        Daemon.start(self)

    def stop( self ):
        #daemon stop first need to stop the Ogl server
        try:
            subprocess.Popen( [OglServerInitdScript, "stop"],
                              stdout=subprocess.PIPE ).communicate()[0]
        except:
            pass
        Daemon.stop(self)

    def _restartOglServerProcess( self ):
        syslog.syslog('OglServerNanny (re)start Ogl!')
        output = ''

        try:
            output += subprocess.Popen( [OglServerInitdScript, "stop"],
                                        stdout=subprocess.PIPE ).communicate()[0]
        except:
            pass

        publicIp = Interface.get_ip_address('eth0')
        try:
            output += subprocess.Popen( [OglServerInitdScript, "start"],
                                        stdout=subprocess.PIPE).communicate()[0]
            Mail.mail( "Oglserver from host %s restarted" % publicIp, output )
        except:
            Mail.mail( "Oglserver from host %s failed to restart" % publicIp, output )
        
    def _queryOglServerStat( self ):
        success = False
        exceptionTraceout = StringIO.StringIO()
        try:
            url = "localhost:%d" % OglStatusListeningPort
            conn = httplib.HTTPConnection(url)
            conn.request("GET", "/")
            r1 = conn.getresponse()
            if r1.status == 200:
                success = True
        except:

            traceback.print_exc( file = exceptionTraceout )

        if success:
            self.nonresponsiveCount_ = 0
        else:
            self.nonresponsiveCount_ += 1

        if self.nonresponsiveCount_ > maxNonresponsiveCount:
            self._restartOglServerProcess()
            self.nonresponsiveCount_ = 0

if __name__ == "__main__":
    syslog.openlog(logoption=syslog.LOG_PID, facility=syslog.LOG_MAIL)
    daemon = OglServerNanny( pidFileName )
    if len(sys.argv) == 2:
        if 'start' == sys.argv[1]:
            daemon.start()
        elif 'stop' == sys.argv[1]:
            daemon.stop()
        elif 'restart' == sys.argv[1]:
            daemon.restart()
        else:
            print "Unknown command"
            sys.exit(2)
        sys.exit(0)
    else:
        print "usage: %s start|stop|restart" % sys.argv[0]
        sys.exit(2)
