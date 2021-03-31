#!/usr/bin/env bash

mdir=/mnt
cmds=(xorriso grub-mkrescue)

function check_cmd() {
    if [[ ! -x "$(command -v $1)" ]]; then
        echo "error: $1 command is not exist" >&2
        exit 1
    fi
}

if [[ ! "$(pwd)" =~ /fengos ]]; then
    echo "please run this script in project root dir"
    exit 1
fi

if [[ ! -e feng.img ]]; then
    echo "please make feng.img file"
    exit 1
fi

echo -n "checking required command..."
for cmd in "${cmds[@]}"; do
    check_cmd $cmd
done
echo "done"

if [[ ! -e src/kernel.bin ]]; then
    echo "kernel.bin is not exits, please run `make` first"
    exit 1
fi

map_dev=$(losetup -f)

set -x
sudo losetup -P "$map_dev" feng.img
sudo mount "$map_dev"p1 $mdir

sudo cp -r config/grub $mdir/boot
sudo cp src/kernel.bin $mdir/boot
sudo cp src/init/init.out $mdir/boot

if [[ -d img.tmp ]]; then
        sudo cp -r img.tmp/* $mdir
fi

sudo grub-install --boot-directory=/mnt/boot "$map_dev"

sudo umount "$map_dev"p1
sudo losetup -d "$map_dev"
