#!/usr/bin/python3

import sys, getopt, time, socket

def printUsage():
    print('usage: spin-socket-client.py -h <HOST> -p <PORT>')
    print('example: spin-socket-client.py -h 127.0.0.1 -p 50000')
    print('-v print version# and usage')
    print('-D print data on terminal')

host = ''
port = 0
print_data = False

def printConfig():
    print('HOST [{}]'.format(host))
    print('Port [{}]'.format(port))
    print('Print [{}]'.format(print_data))

def sizeof_fmt(num, suffix="B"):
    for unit in ["", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi"]:
        if abs(num) < 1024.0:
            return f"{num:3.1f}{unit}{suffix}"
        num /= 1024.0
    
def receiveBlock(s, num):
    chunks = []
    bytes_recd = 0
    while bytes_recd < num:
        chunk = s.recv(num - bytes_recd)
        chunks.append(chunk)
        bytes_recd = bytes_recd + len(chunk)
        if chunk == b'':
            raise RuntimeError("socket connection broken")
    rv = b''.join(chunks)
#    print('received {} bytes'.format(num))
#    print(''.join(format(x, '02x') for x in rv))
    return rv
    
def runClient():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    total_received = 0
    session_start = time.time()
    print_time = 0
    while True:
        frame_start = int.from_bytes(receiveBlock(s, 1), 'big')
        if frame_start != 4:
            raise RuntimeError("invalid 'frame_start' received [{}] expected [4]".format(frame_start))        
        protocol = int.from_bytes(receiveBlock(s, 2), 'big')
        if protocol != 32:
            raise RuntimeError("invalid 'protocol' received [{}] expected [32]".format(protocol))
        packet_size = int.from_bytes(receiveBlock(s, 4), 'big')
        data = receiveBlock(s, packet_size)
        frame_end = int.from_bytes(receiveBlock(s, 1), 'big')
        if frame_end != 3:
            raise RuntimeError("invalid 'frame_end' received [{}] expected [3]".format(frame_end))
        total_received = total_received + packet_size
        time_delta = time.time() - session_start
        if time_delta > 1 and int(time_delta)%2==0 and print_time != int(time_delta):
            print_time = int(time_delta)
            bpsec = int(total_received / time_delta)
            print('{} [{}/sec]'.format(sizeof_fmt(total_received), sizeof_fmt(bpsec)))
        if print_data:
            print("[{}]".format(data))

def main(argv):
    global host
    global port
    global print_data
    if len(sys.argv) < 5:
        printUsage()
        sys.exit()
    try:
        opts, args = getopt.getopt(argv,"vDh:p:")
    except getopt.GetoptError:
        printUsage()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-v':
            printUsage()
            sys.exit()
        elif opt in ("-D"):
            print_data = True            
        elif opt in ("-h"):
            host = arg
        elif opt in ("-p"):
            port = int(arg)
    printConfig()
    runClient()
    
if __name__ == "__main__":
    main(sys.argv[1:])

