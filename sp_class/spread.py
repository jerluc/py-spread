#!/usr/bin/env python
# encoding: utf-8
"""
spread.py

Created by  on 2008-04-14.
Copyright (c) 2008 __MyCompanyName__. All rights reserved.
"""
import socket,struct
from untils import protocol_Connect,protocol_Create

class Spread:
    def __init__(self, sp_name, sp_host):
        self.sp_name = sp_name
        self.sp_host = sp_host
        host_list = sp_host.split('@')
        self.sp_port = int(host_list[0])
        self.sp_ip = host_list[1]
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.sp_ip,self.sp_port))
        self.private_name = ''
    
    def socket_send(self, head):
        self.sock.send(head)
    
    def socket_rec(self, len=1):
        return self.sock.recv(len)
    
    def connect(self):
        msg_connect = protocol_Connect(self.sp_name)
        self.socket_send(msg_connect)
    	msg = self.socket_rec()
    	buffer = [ord(m) for m in self.socket_rec(ord(msg))]
    	sendAuthMethod = [0,]*90
    	for i in xrange(4):
    		sendAuthMethod[i] = buffer[i]
    	msg_auth = struct.pack('!90B',*sendAuthMethod)
    	self.socket_send(msg_auth)
    	msg = self.socket_rec()
    	#read Version
    	majorVersion = self.socket_rec()
    	majorVersion = ord(majorVersion)
    	minorVersion = self.socket_rec()
    	minorVersion = ord(minorVersion)
    	patchVersion = self.socket_rec()
    	#read group
    	private_name = self.sock.recv(ord(self.socket_rec()))
    	self.private_name = private_name
    
    def multicast(self, groups, message):
        data_len = len(message)
        send_head = protocol_Create('SEND_MESS',self.private_name,groups,data_len)
        self.socket_send(send_head)
    	msg = struct.pack('!%ss'%data_len,message)
    	self.socket_send(msg)
    
    def join(groups):
        """docstring for join"""
        pass
    
    def leave(self):
        """docstring for leave"""
        pass
    
    def disconnect(self):
        """docstring for disconnect"""
        pass

if __name__ == '__main__':
    sp_host = '3333@10.55.37.105'
    sp_name = 'junyw'
    sp = Spread(sp_name, sp_host)
    sp.connect()
    for i in xrange(0,100):
        group = ['spreadtest']
        data = '2008-04-16 00:00:00|1206979200|editarticle|172.16.175.200|/rss/zcwxs.xml%s\n'%i
        sp.multicast(group, data)

