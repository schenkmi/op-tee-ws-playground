# OP-TEE AES application

## Build

### Build TA
```
cdex && cd secure_storage/ta
make clean && make
```

### Build host
```
cdex && cd secure_storage/host
make clean && make
```

## Copy to target
```
cdex && cd secure_storage
cp ta/f4e750bb-1437-4fbf-8785-8d3580c34994.ta /export/nfs/rpi/lib/optee_armtz
cp host/optee_example_secure_storage /export/nfs/rpi/usr/bin
```

## Execute on target
```
optee_example_secure_storage
```

