#!/usr/bin/env python
# encoding: utf-8
"""
untils.py

Created by  on 2008-04-14.
Copyright (c) 2008 __MyCompanyName__. All rights reserved.
"""
import struct

from __init__ import serviceType_dic,TAG


def protocol_Create(serTpye,pname,gname):
    
    msg_header = serviceType_dic.get(serTpye)
    msg_header.append(pname)
    
    toB(msg_header, len(gname));
    print msg_header

    toB(msg_header, (0 << 8) & 0x00FFFF00);
    
    #buffer = [0,0,0,2,pname, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, len(pname), gname]
    #p = struct.pack(TAG,*buffer)
    #print p

def toB(buffer,i):
	buffer.append('1')

def toBName(buffer,bufferIndex,name):
	for i in name:
 		buffer[bufferIndex] = i
 		bufferIndex = bufferIndex + 1

if __name__ == '__main__':
	protocol_Create('JOIN_MESS','#test#machine1',['spreadtest',])

