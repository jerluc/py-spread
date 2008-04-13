MAX_GROUP_NAME = 32
def sp():
	import socket,struct,sys
	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	except:
		print 'err'
		sys.exit(-1)
	sock.connect(('10.55.37.105',3333))
	p = struct.pack('!5B4s',4,0,0,0,len('test'),'test')
	sock.send(p)
	msg = sock.recv(1)
	l = ord(msg)
	msg = sock.recv(l)
	buffer = [ord(m) for m in msg]
	sendAuthMethod = [0,]*90
	for i in xrange(4):
		sendAuthMethod[i] = buffer[i]
	p = struct.pack('!90B',*sendAuthMethod)
	sock.send(p)
	msg = sock.recv(1)
	l = ord(msg)
	#read Version
	majorVersion = sock.recv(1)
	majorVersion = ord(majorVersion)
	minorVersion = sock.recv(1)
	minorVersion = ord(minorVersion)
	patchVersion = sock.recv(1)
	#read group
	msg = sock.recv(1)
	grouplen = ord(msg)
	msg = sock.recv(grouplen)
	print msg
	data_len = '2008-04-12 00:00:00|1206979200|editarticle|172.16.175.200|/rss/zcwxs.xml\n'
	#send message 
	tag = '!4B32s12B32s'
	buffer = [0,0,0,2, '#test#machine1', 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, len(data_len), 'spreadtest' ]
	p = struct.pack(tag,*buffer)
	print p
	sock.send(p)
	p = struct.pack('!%ss'%len(data_len),data_len)
	print p
	sock.send(p)
	#join group
	tag = '!4B32s12B32s'
	buffer = [0,1,0,0, '#test#machine1', 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'spreadtest' ]
	p = struct.pack(tag,*buffer)
	print p
	sock.send(p)
	p = struct.pack('!0s','')
	sock.send(p)
	#rec message
	buffer = sock.recv(48)
	print buffer
	buffer = [ord(b) for b in buffer]
	print sock.recv(32)#private_name
	print sock.recv(buffer[-4])#message
	#leave group
	buffer = [0,2,0,0, '#test#machine1', 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'spreadtest' ]
	p = struct.pack(tag,*buffer)
	print p
	sock.send(p)
	p = struct.pack('!0s','')
	sock.send(p)
	#disconnect
	buffer = [0,4,0,0, '#test#machine1', 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '#test#machine1']
	p = struct.pack(tag,*buffer)
	print p
	sock.send(p)
	p = struct.pack('!0s','')
	#print Rec.spread_Rec(sock)
	sock.close()

if __name__ == '__main__':
	 import struct
	 sp()
