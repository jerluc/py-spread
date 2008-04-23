#!/usr/bin/env python
# encoding: utf-8
"""
spread.py

Created by  on 2008-04-14.
Copyright (c) 2008 __MyCompanyName__. All rights reserved.
"""
import socket,struct
from untils import protocol_Connect,protocol_Create,val_g
from sp_error import SpreadException

class Spread:
    def __init__(self, sp_name='test', sp_host='4803@localhost'):
        """
        Create a new Spread object.
        
        @param sp_name: a string specifying the name under which this client is to be
                        known; the default is a private name made up by Spread.
                        The default is test.
                        
        @param sp_host: a string specifying the port and host of the spread daemon to use,
                        of the form "<port>@<hostname>" or "<port>", where <port> is an
                        integer port number (typically the constant DEFAULT_SPREAD_PORT)
                        represented as a string, and <hostname> specifies the host where
                        the daemon runs, typically "localhost".  The default is the
                        default Spread port and host.
        
        >>> sp = Spread()
        >>> sp = Spread('test','4803@localhost')
        """
        self.sp_name = sp_name
        self.sp_host = sp_host
        host_list = sp_host.split('@')
        self.sp_port = int(host_list[0])
        self.sp_ip = host_list[1]
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.sp_ip,self.sp_port))
        self.private_name = None
        self.group = None
        
    def socket_send(self, head):
        return self.sock.send(head)
    
    def socket_rec(self, len=1):
        return self.sock.recv(len)
    
    def connect(self):
        """
        Connect to the Spread daemon.
        Upon failure, SpreadException is raised.
        
        >>> sp.connect()
        #test#machine1
        >>> try:
        >>>     sp.connect()
        >>> except SpreadException spErr:
        >>>     print spErr.err_msg
        """
        msg_connect = protocol_Connect(self.sp_name)
        self.socket_send(msg_connect)
    	msg = self.socket_rec()
    	authlen = ord(msg)
    	if authlen == -1 or authlen >= 128:
    	    raise SpreadException(authlen)
    	buffer = [ord(m) for m in self.socket_rec(authlen)]
    	sendAuthMethod = [0,]*90
    	for i in xrange(4):
    		sendAuthMethod[i] = buffer[i]
    	msg_auth = struct.pack('!90B',*sendAuthMethod)
    	self.socket_send(msg_auth)
    	#checkAccept
    	accept = ord(self.socket_rec())
    	if accept == -1 or accept != 1:
    	    raise SpreadException(accept)
    	#read Version
    	majorVersion = self.socket_rec()
    	majorVersion = ord(majorVersion)
    	minorVersion = self.socket_rec()
    	minorVersion = ord(minorVersion)
    	patchVersion = self.socket_rec()
    	patchVersion = ord(patchVersion)
    	version = (majorVersion | minorVersion | patchVersion)
    	if version == -1:
    	    raise SpreadException(version)
    	#read group
    	group_len = ord(self.socket_rec())
    	if group_len == -1:
    	    raise SpreadException(group_len)
    	private_name = self.sock.recv(group_len)
    	self.private_name = private_name
        return private_name
    
    def multicast(self, groups, message):
        """
        Send a message to all members of a group.  
        Return the number of bytes sent
        
        @param groups: group list
        @param message: data

        >>> data = 'test spread multicast'
        >>> groups = ['group1']
        >>> sp.multicast(groups, message)
        21
        """
        data_len = len(message)
        send_head = protocol_Create('SEND_MESS',self.private_name,groups,data_len)
        self.socket_send(send_head)
    	msg = struct.pack('!%ss'%data_len,message)
        return self.socket_send(msg)
    
    def receive(self):
        """
        Block (if necessary) until a message is received, and
        return an object representing the received message.
        The return value is of type RegularMsgType
    
        >>> sp.receive()
        """
        while True:
            recv_head = self.socket_rec(48)
            if len(recv_head) == 48:
                break
        self.socket_rec(32)
    	return self.socket_rec(ord(recv_head[-4]))#message
    
    def join(self, groups=[]):
        """
        Join the group with the given name.  Return None.
        Upon failure, SpreadException is raised.
        
        @param groups: group list
        
        >>> sp.join(['group1','group2'])
        
        >>> try:
        >>>     sp.join(['group1','group2'])
        >>> except SpreadException spErr:
        >>>     print spErr.err_msg
        """
        val_g(groups)
        send_head = protocol_Create('JOIN_MESS',self.private_name,groups)
        self.socket_send(send_head)
        self.socket_send(struct.pack('!0s',''))
        self.group = groups
    
    def leave(self):
        """
        leave the group with the given name.  Return None.
        Upon failure, SpreadException is raised.
        
        >>> sp.leave()
        
        >>> try:
        >>>     sp.leave()
        >>> except SpreadException spErr:
        >>>     print spErr.err_msg
        """        
        send_head = protocol_Create('LEAVE_MESS',self.private_name,self.group)
        self.socket_send(send_head)
        self.socket_send(struct.pack('!0s',''))

    def disconnect(self):
        """
        Disconnect from the spread daemon.  
        The spread object should not be used after this call.
        
        >>> sp.disconnect()      
        """
        send_head = protocol_Create('KILL_MESS',self.private_name,[self.private_name])
        self.socket_send(send_head)
        self.socket_send(struct.pack('!0s',''))
        self.sock = None
        self.private_name = None
        self.group = None

if __name__ == '__main__':
    sp_host = '3333@10.55.37.127'
    sp_name = 'junyw123'
    sp = Spread(sp_name, sp_host)
    sp.connect()
    sp.private_name
    group = ['purge_gz']
    sp.join(group)
    for i in xrange(0,10):
        print i,':',sp.receive()
    sp.leave()
    sp.disconnect()
    
    #data = '2008-04-16 00:00:00|1206979200|editarticle|172.16.175.200|/rss/zcwxs.xml%s\n'
    #for i in xrange(0,100):
        #sp.multicast(group, data)

