#!/usr/bin/env bash

mount_dir=/mnt
tmp_dir=img.tmp
modules=$(find src -name "*.mod")
loop_device=$(losetup -f)
image_file=feng.img
root_partition_type_guid=B02F4C03-F4A9-4297-9F98-D6E20949450C

function check_cmd() {
    if [[ ! -x "$(command -v "$1")" ]]; then
        echo "error: $1 command is not exist"
        exit 1
    fi
}

if [[ ! "$(pwd)" =~ /fengos ]]; then
    echo "please run this script in project root directory"
    exit 1
fi

if [[ ! -e "${image_file}" ]]; then
    echo "Creating img file, please wait a little time..."
    check_cmd parted
    check_cmd sgdisk
    dd if=/dev/zero of="${image_file}" bs=512 count=$((2048*1024)) # 1GB
    sudo losetup -P "${loop_device}" "${image_file}"
    sudo parted "${loop_device}" mklabel gpt
    sudo parted "${loop_device}" mkpart primary 2048s 4095s
    sudo parted "${loop_device}" name 1 BIOS_boot_partition
    sudo parted "${loop_device}" set 1 bios_grub on
    sudo parted "${loop_device}" mkpart primary 4096s 1024M
    sudo parted "${loop_device}" name 2 fengos
    sudo mkfs.ext2 "${loop_device}p2"
    sudo sgdisk -t 2:"${root_partition_type_guid}" "${loop_device}"
    sudo losetup -d "${loop_device}" 
fi

if [[ ! -e src/kernel.bin ]]; then
    echo "kernel.bin is not exits, please run 'make' first"
    exit 1
fi

echo "Copying the necessary files..."
sudo losetup -P "${loop_device}" "${image_file}"
sudo mount "${loop_device}p2" "${mount_dir}"

sudo mkdir -p "${mount_dir}/boot"
sudo cp -r config/grub "${mount_dir}/boot"
sudo cp src/kernel.bin "${mount_dir}/boot"

for module in ${modules}; do
    sudo cp "${module}" "${mount_dir}/boot"
done

mkdir -p "${tmp_dir}/bin"
mkdir -p "${tmp_dir}/usr/bin"
cp -r shell/sh "${tmp_dir}/bin"
cp -r app/hello/hello "${tmp_dir}/usr/bin"
sudo cp -r "${tmp_dir}"/* "${mount_dir}" 

echo "Install grub for device..."
sudo grub-install --target=i386-pc --boot-directory=/mnt/boot "${loop_device}"

sudo umount "${loop_device}p2" 
sudo losetup -d "${loop_device}"
echo -e "\033[32m done \033[0m"
