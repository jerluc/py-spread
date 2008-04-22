#!/usr/bin/env python
# encoding: utf-8
"""
untils.py

Created by  on 2008-04-14.
Copyright (c) 2008 __MyCompanyName__. All rights reserved.
"""
import struct,copy

serviceType_dic = {
    'JOIN_MESS'  : [0,1,0,0],
    'LEAVE_MESS' : [0,2,0,0],
    'KILL_MESS'  : [0,4,0,0],
    'SEND_MESS'  : [0,0,0,2],
}
TAG = '!4B32s12B%s'

def protocol_Create(serTpye,pname,gname,data_len=0): 
    msg_header = copy.copy(serviceType_dic.get(serTpye))
    msg_header.append(pname)
    
    toB(msg_header, len(gname));

    toB(msg_header, 0);
    
    toB(msg_header, data_len);
    
    for g in gname:
        msg_header.append(g)
    tag = TAG%('32s'*len(gname))
    return struct.pack(tag,*msg_header)

def toB(buffer,i):
    for x in [24,16,8,0]:
		  buffer.append((i >> x) & 0xFF)

def protocol_Connect(connect_name):
    name_len = len(connect_name)
    return struct.pack('!5B%ss'%name_len, 4, 0, 0, 0, name_len, connect_name)
    def join_err(self,x):
        if x<36 or x>126:
            raise SpreadException(-14)
    
def group_len(g):
    if g >= 32:
        raise SpreadException(-14)

def val_g(groups):
    try:
        val_groups = lambda g:[[lambda b:self.join_err(ord(c)) for c in b] or self.group_len(b) for b in g]
        val_groups(groups)
    except:
        raise SpreadException(-14)

if __name__ == '__main__':
    p = protocol_Create('JOIN_MESS','#test#machine1',['spreadtest',])
    print p
    p = protocol_Connect('junyw')
    print p

