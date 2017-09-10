#!/usr/bin/python2

import os
import select
import socket

rPath = "/home/pupillo/p1"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    os.mkfifo(rPath)
except OSError:
    pass

s.connect(("192.168.1.70", 9100))

rp = os.open(rPath, os.O_RDWR)
while(True):
	print("wait")
	select.select([rp],[],[], 3)
	response = os.read(rp, 400)
	if response != "":
	    print("received from pipe: {}".format(response))
            s.send(response)

s.close()
rp.close()

