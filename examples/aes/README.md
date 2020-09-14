# OP-TEE AES application

## Build

### Build TA
```
cdex && cd aes/ta
make clean && make
```

### Build host
```
cdex && cd aes/host
make clean && make
```

## Copy to target
```
cdex && cd aes
cp ta/5dbac793-f574-4871-8ad3-04331ec17f24.ta /export/nfs/rpi/lib/optee_armtz
cp host/optee_example_aes /export/nfs/rpi/usr/bin
```

## Execute on target
```
optee_example_aes
```

