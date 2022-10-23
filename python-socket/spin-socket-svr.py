#!/usr/bin/python3

import sys, getopt, time, socket


def printUsage():
    print('usage: spin-socket-svr.py -h <BIND-IP> -p <PORT> -i <inputfile> -d <packet-delay msec>')
    print('example: spin-socket-svr.py -h 127.0.0.1 -p 50000 -i test.data -d 300')
    print('-v print version# and usage')
    print('-L add packet# in stream for text terminal clients')

inputfile = ''
bind_ip = ''
port = 0
packet_delay = 0.1
data = 0
add_packet_label = False

def readInputFile():
    global data
    print('reading [{}]'.format(inputfile))
    f = open(inputfile, 'rb')
    data = f.read()

def printInputFile():
    print("data=[{} Bytes]".format(len(data)))

def printConfig():
    print('Input file [{}]'.format(inputfile))
    print('Bind IP [{}]'.format(bind_ip))
    print('Port [{}]'.format(port))
    print('Delay [{}]'.format(packet_delay))    

def sizeof_fmt(num, suffix="B"):
    for unit in ["", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi"]:
        if abs(num) < 1024.0:
            return f"{num:3.1f}{unit}{suffix}"
        num /= 1024.0
    
def serveClient(c):
    pnum = 0
    total_sent = 0
    session_start = time.time()
    print_time = 0
    tag = 10
    try:
        with c:
            while True:
                pkt_label = ""
                pkt_label_enc = 0 
                packet_size = len(data)
                if add_packet_label:
                    pkt_label = "\n\033[2;31;43m[" + str(pnum) + ']\033[0;0m\n'
                    pkt_label_enc = pkt_label.encode()
                    packet_size = packet_size + len(pkt_label_enc)
                c.sendall(pnum.to_bytes(8, 'big'))
                c.sendall(packet_size.to_bytes(8, 'big'))
                if add_packet_label:
                    c.sendall(pkt_label_enc)
                c.sendall(data)
                c.sendall(tag.to_bytes(1, 'big'))
                pnum = pnum + 1
                total_sent = total_sent + packet_size
                time_delta = time.time() - session_start
                if time_delta > 1 and int(time_delta)%2==0 and print_time != int(time_delta):
                    print_time = int(time_delta)
                    bpsec = int(total_sent / time_delta)
                    print('#{} {} Bytes [{}/sec]'.format(pnum, total_sent, sizeof_fmt(bpsec)))
                if packet_delay > 0:
                    time.sleep(packet_delay)
    except IOError as e:
        print("client disconnected")
        pass
    
    
def runServer():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((bind_ip, port))
    while True:
        s.listen()
        c, addr = s.accept()
        print(f"Connected by {addr}")
        serveClient(c)
    
def main(argv):
    global inputfile
    global bind_ip
    global port
    global packet_delay
    global add_packet_label
    if len(sys.argv) < 7:
        printUsage()
        sys.exit()
    try:
        opts, args = getopt.getopt(argv,"vLh:i:p:d:")
    except getopt.GetoptError:
        printUsage()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-v':
            printUsage()
            sys.exit()
        elif opt in ("-L"):
            add_packet_label = True
        elif opt in ("-h"):
            bind_ip = arg
        elif opt in ("-i"):
            inputfile = arg
        elif opt in ("-p"):
            port = int(arg)
        elif opt in ("-d"):
            packet_delay = float(arg) / 1000
    printConfig()
    readInputFile()
    printInputFile()
    runServer()
    
if __name__ == "__main__":
    main(sys.argv[1:])
