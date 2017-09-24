#!/usr/bin/python2

import os
import select
import socket
import argparse
import signal
import sys


EXIT = False


def vprint(line):
    
    if VERBOSE:
        print(line)


def exit_gracefully(signum, frame):

    global EXIT

    print("exit required")
    EXIT = True



#####################################
#MAIN
#####################################

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser()
    parser.add_argument('address', help='Printer Server IP address')
    parser.add_argument('-p', '--port', help='Printer Server Port', default=9100)
    parser.add_argument('-n', '--pipe', help='Pipe Path', default="/tmp/printer_pipe")
    parser.add_argument('-v', '--verbose', help='Verbose mode', default=False, action="store_true")
    args = parser.parse_args()
    
    SRV_ADDRESS = args.address
    SRV_PORT = args.port
    PIPE_PATH = args.pipe
    VERBOSE = args.verbose

    if (os.path.exists(PIPE_PATH)):
        os.remove(PIPE_PATH)

    try:
        os.mkfifo(PIPE_PATH)
    except OSError as e:
        print("Error creating pipe {}".format(PIPE_PATH))
        print(e)
        sys.exit(1)
   
    signal.signal(signal.SIGINT, exit_gracefully)
    signal.signal(signal.SIGTERM, exit_gracefully)
    
    PIPE = os.open(PIPE_PATH, os.O_RDWR)
    EXIT = False
    response = None
    print_wait = True

    while(EXIT == False):
    
        if print_wait:
            vprint("wait")
            print_wait = False
        
        #readable, writable, exceptional = select.select([PIPE],[],[], 1)
        try:
            readable, writable, exceptional = select.select([PIPE],[],[], 1)
        except Exception as e:
            print("select exception, continue")
            print(e)
            pass

        response = None
        if PIPE in readable:
            response = os.read(PIPE, 400)
        
        if response is not  None:
            vprint("received from pipe: {}".format(response))
        
            try:
                SCK = socket.create_connection((SRV_ADDRESS, SRV_PORT), 5)
                SCK.send(response)
                SCK.close()
            except Exception as e:
                print("cannot setup socket, skipping")

            print_wait = True

    os.close(PIPE)


    if (os.path.exists(PIPE_PATH)):
        os.remove(PIPE_PATH)
   
    vprint("EXIT")
    sys.exit(0)

