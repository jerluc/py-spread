from py_spread.spread import Spread
def testSpread():
    sp_host = '4803@localhosts'
    sp_name = 'testSpread'
    sp = Spread(sp_name, sp_host)
    #test connect
    sp.connect()
    groups = ['test']
    data = 'test spread multicast'
    #test multicast
    sp.multicast(groups, message)
    #test join
    sp.join(group)
    #test receive
    sp.receive()
    #test leave
    sp.leave()
    #test leave
    sp.disconnect()
    
if __name__ == '__main__':
    testSpread()
