# op-tee-ws-playground
Playground for OP-TEE Workshop 2020


## Artifacts for OP-TEE Workshop 2020

### aarch64-buildroot-linux-gnu_sdk-buildroot.tar.gz
This is the toolchain created by buildroot. Install it with
```
tar -xzvf aarch64-buildroot-linux-gnu_sdk-buildroot.tar.gz -C /opt
cd /opt/aarch64-buildroot-linux-gnu_sdk-buildroot && ./relocate-sdk.sh
```

### TA make helper
As with OP-TEE OS 3.9.0 it looks like a bug. To drive the TA Loglevel from make please
```
gedit $BR_SDK_SYSROOT/lib/optee/export-ta_arm64/mk/conf.mk &
```
and change the line
```
CFG_TEE_TA_LOG_LEVEL := 1
```
into
```
CFG_TEE_TA_LOG_LEVEL ?= 1
```

### sdcard.img
This is the image used to flash the SD card. Please change /dev/sdX with the correct mounting point of your sd card.
```
sudo dd if=images/sdcard.img of=/dev/sdX bs=1M conv=fdatasync status=progress
```

### rootfs.tar.gz
This is the RPI3 root file system which can be used for NFS operation. Can be extracted to /export/nfs/rpi with (sudo is needed)
```
sudo rm -rf /export/nfs/rpi/* && sudo tar -xzvf rootfs.tar.gz -C /export/nfs/rpi
sudo chown $USER /export/nfs/rpi/lib/optee_armtz
sudo chown $USER /export/nfs/rpi/usr/bin
```

### Configure NFS Boot in U-BOOT environment
Inside U-BOOT console copy past the lines below. Keep in mind that you probaly need to adapt the ip addresses.
```
setenv set_bootargs_nfs2 "setenv bootargs '${bootargs}' root=/dev/nfs rw rootfstype=nfs nfsroot=10.0.0.10:/export/nfs/rpi,vers=4.1,proto=tcp rw ip=10.0.0.20:10.0.0.10:10.0.0.1:255.255.255.0:rpi3::off"
setenv nfsboot2 "run load_fit; run set_bootargs_tty set_bootargs_nfs2 set_common_args; run boot_fit"
setenv bootcmd "run nfsboot2"
saveenv
```

Reset to default boot
```
setenv bootcmd "run mmcboot"
```
or reflash the SD card.

### Building the artifacts
```
git clone --recursive https://github.com/schenkmi/buildroot-rpi-optee.git
cd buildroot-rpi-optee
git checkout tags/V1.2.0
```
```
mkdir -p out && cd out
make -C ../buildroot O="$(pwd)" BR2_EXTERNAL=".." rpi3_defconfig
make
make sdk
```

### Console
```
sudo picocom -b 115200 /dev/ttyUSB0
```

### Login
The login is root / root
