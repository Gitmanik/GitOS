#!/bin/sh

DISK_BIN="$1"
MOUNT_DIR="$2"
FS_DIR="$3"
USERLAND_DIR="$4"
KERNEL_BIN="$5"
GITBOOT_BIN="$6"
BOOT_DIR="$2/boot"

echo "Using disk.bin: $DISK_BIN"
echo "Using mount dir: $MOUNT_DIR"
echo "Using boot dir: $BOOT_DIR"
echo "Using userland dir: $USERLAND_DIR"
echo "Using kernel.bin: $KERNEL_BIN"
echo "Using gitboot.bin: $GITBOOT_BIN"

umount "$MOUNT_DIR" || (exit 0)
rm -rf "$MOUNT_DIR"
mkdir "$MOUNT_DIR"

rm -f "$DISK_BIN"
dd if=/dev/zero of="$DISK_BIN" bs=1M count=32

parted "$DISK_BIN" --script mklabel msdos
parted "$DISK_BIN" --script mkpart primary fat16 1MiB 100%

# Get a loop device for the disk image
LOOP_DEV=$(losetup -f --show -P "$DISK_BIN")
if [ -z "$LOOP_DEV" ]; then
    echo "Error: Could not assign a loop device."
    exit 1
fi

echo "Using loop device: $LOOP_DEV"

kpartx -a "$LOOP_DEV"

LOOP_BASE=$(basename "$LOOP_DEV")
PARTITION="/dev/mapper/${LOOP_BASE}p1"
if [ ! -e "$PARTITION" ]; then
    echo "Error: Partition mapping $PARTITION does not exist."
    sudo losetup -d "$LOOP_DEV"
    exit 1
fi
echo "Using partition mapping: $PARTITION"

# Format the partition as FAT16
mkfs.vfat -F 16 "$PARTITION"

# Mount the partition
mount "$PARTITION" "$MOUNT_DIR"

cp -r "$FS_DIR"/. "$MOUNT_DIR"/.
cp -r "$USERLAND_DIR"/. "$MOUNT_DIR"/.

dd if=$GITBOOT_BIN of=$LOOP_DEV bs=446 count=1 seek=0
dd if=$KERNEL_BIN of=$LOOP_DEV bs=512 seek=1

umount "$MOUNT_DIR"
kpartx -d "$LOOP_DEV"
losetup -d "$LOOP_DEV"

