#!/usr/bin/env bash
# this script is deprecated

tmpdir=mkiso.tmp
cmds=(xorriso grub-mkrescue)

function check_cmd() {
    if [[ ! -x "$(command -v "$1")" ]]; then
        echo "$1 not exist, please install it"
        exit 1
    fi
}

if [[ ! "$(pwd)" =~ /fengos ]]; then
    echo "please run this script in project root dir"
    exit 1
fi

echo "checking required command..."
for cmd in "${cmds[@]}"; do
    check_cmd "$cmd"
done
echo "done"

if [[ ! -e src/kernel.bin ]]; then
    echo "kernel.bin is not exits, please run 'make' first"
    exit 1
fi

if [[ ! -d "$tmpdir" ]]; then
    mkdir -p $tmpdir/boot
fi

cp -r config/grub $tmpdir/boot
cp src/kernel.bin $tmpdir/boot
grub-mkrescue -o feng.iso $tmpdir
