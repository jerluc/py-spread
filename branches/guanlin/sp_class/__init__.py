MAX_GROUP_NAME = 32

serviceType_dic = {
    'JOIN_MESS'  : [0,1,0,0],
    'LEAVE_MESS' : [0,2,0,0],
    'KILL_MESS'  : [0,4,0,0],
    'SEND_MESS'  : [0,0,0,2],
}

TAG = '!4B32s12B32s'

if __name__ == '__main__':
    print serviceType_dic