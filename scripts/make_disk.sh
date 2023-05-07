STAGE1_BIN=$3
KERNEL_BIN=$4
DISK_BIN=$2
FS_DIR=$1

dd if=${STAGE1_BIN} >> ${DISK_BIN}
dd if=${KERNEL_BIN} >> ${DISK_BIN}

# 							   16MB=16777216 - 1
dd if=/dev/zero of=${DISK_BIN} seek=16777215 bs=1 count=1
mkdir mnt
mount -t vfat -o fat=16 ${DISK_BIN} ./mnt
cp -r ${FS_DIR}/. ./mnt/.
umount ./mnt
rm -rf ./mnt