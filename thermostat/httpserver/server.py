#!/usr/bin/python2
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from os import curdir, sep
import md5
import os


PORT_NUMBER = 8000

#This class will handles any incoming request from
#the browser 
class myHandler(BaseHTTPRequestHandler):
    
    #Handler for the GET requests
    def do_GET(self):

        print(self.headers)
        useragent = self.headers.get("User-Agent")
        if useragent != "ESP8266-http-Update":
            print("FORBIDDEN")
            self.send_response(403)
            self.end_headers()
            return

        if self.path[0] == "/":
            self.path = self.path[1:]

        filepath = os.path.join(os.getcwd(), self.path)
        versionpath = filepath + ".version"
        
        if not os.path.exists(filepath) or not os.path.exists(versionpath):
            print("NOT EXISTENT")
            self.send_response(404)
            self.end_headers()
            return
        
        f = open(versionpath)
        version = f.read()
        f.close()
        fileVersion = version.strip()
        reqVersion = self.headers.get("x-ESP8266-version")

        if fileVersion == reqVersion:
            print("ALREADY UPDATED")
            self.send_response(304)
            self.end_headers()
            return
        

        f = open(filepath)
        buff = f.read()
        f.close()

        m = md5.new()
        m.update(buff)
        print(str(m.hexdigest()))

        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header(
                "Content-Disposition", 
                "attachment; filename={}".format(os.path.basename(filepath)))
        self.send_header("Content-Length", str(len(buff)))
        self.send_header("x-MD5", str(m.hexdigest())) 
        self.end_headers()
        self.wfile.write(buff)
        return


#MAIN

if __name__ == "__main__":
    
    try:
    	#Create a web server and define the handler to manage the
        #incoming request
        server = HTTPServer(('', PORT_NUMBER), myHandler)
        print 'Started httpserver on port ' , PORT_NUMBER
    
        #Wait forever for incoming htto requests
        server.serve_forever()

    except KeyboardInterrupt:
	print '^C received, shutting down the web server'
