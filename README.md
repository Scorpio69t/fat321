# Feng OS

Feng OS is a simple operating system based on x86-64 cpu.


## How to run

Feng OS is developed using Linux, you can compile and run it on any Linux distribution. The following example will using Debian.

First, clone the project to you computer:

```bash
git clone git@github.com:lml256/fengos.git
cd fengos
```

Before compiling, you need to install some necessary packages:

```bash
sudo apt-get update
sudo apt-get install build-essential qemu-system-x86 gdisk parted
```

Then run `make` command to compile the project.

There are some useful scripts in the `scripts` directory, and you can use `mkimg.sh` to make a virtual disk image for use by qemu and package the compile output to it.

```bash
$ ./scripts/mkimg.sh
Copying the necessary files...
Install grub for device...
Installing for i386-pc platform.
Installation finished. No error reported.
 done
```

Finally, run it with qemu:

```bash
qemu-system-x86_64 -drive file=feng.img,format=raw,index=0,media=disk -m 1G -smp 1
```
