#!/usr/bin/env python
# encoding: utf-8
from sp_class.spread import *
sp_host = '3333@10.55.37.127'
sp_name = 'testsend'
sp = Spread(sp_name, sp_host)
sp.connect()
send_host = '3333@10.55.37.105'
send_name = 'send123'
sp_send = Spread(send_name, send_host)
sp_send.connect()
group = ['purge_gz']
sp.join(group)
for i in xrange(0,10):
        msg = sp.receive()
        print msg
        sp_send.multicast(group, msg)
sp.leave()
sp.disconnect()
sp_send.disconnect()


