#!/usr/bin/env python
# encoding: utf-8
"""
sp_error.py

Created by  on 2008-04-17.
Copyright (c) 2008 __MyCompanyName__. All rights reserved.
"""
from ctypes import c_long

error_dic = {
    '-1':'ILLEGAL_SPREAD',
    '-2':'COULD_NOT_CONNECT',
    '-3':'REJECT_QUOTA',
    '-4':'REJECT_NO_NAME',
    '-5':'REJECT_ILLEGAL_NAME',
    '-6':'REJECT_NOT_UNIQUE',
    '-7':'REJECT_VERSION',
    '-8':'CONNECTION_CLOSED',
    '-9':'REJECT_AUTH',
    '-11':'ILLEGAL_SESSION',
    '-12':'ILLEGAL_SERVICE',
    '-13':'ILLEGAL_MESSAGE',
    '-14':'ILLEGAL_GROUP',
    '-15':'BUFFER_TOO_SHORT',
    '-16':'GROUPS_TOO_SHORT',
    '-17':'MESSAGE_TOO_LONG',
    '-18':'NET_ERROR_ON_SESSION',
    '0':'unrecognized error',
}

class SpreadException(Exception):
    def __init__(self, err_no):
        Exception.__init__(self)
        err_no = str(c_long(0xffffff00 | err_no).value)
        print err_no
        if error_dic.has_key(err_no):
            self.err_msg = error_dic.get(err_no)
        else:
            self.err_msg = error_dic.get('0')
        print self.err_msg

if __name__ == '__main__':
    s = raw_input('Enter something --> ')
    authlen = int(s)
    if authlen == -1 or authlen >= 128:
        raise SpreadException(authlen)
