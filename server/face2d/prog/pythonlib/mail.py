#!/usr/bin/python

import smtplib
import syslog

class Mail():
    @staticmethod
    def mail(title, output):
        sender = 'daemon@laputalab.com'
        receivers = ['wminghao@gmail.com']
        try:
            smtpObj = smtplib.SMTP('localhost')
            message = 'Subject: %s\n\n%s' % (title, output)
            smtpObj.sendmail(sender, receivers, message)         
            syslog.syslog("Successfully sent email")
        except SMTPException:
            syslog.syslog("Failed to send email")
