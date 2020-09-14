# OP-TEE hotp application

## Build

### Build TA
```
cdex && cd hotp/ta
make clean && make
```

### Build host
```
cdex && cd hotp/host
make clean && make
```

## Copy to target
```
cdex && cd hotp
cp ta/484d4143-2d53-4841-3120-4a6f636b6542.ta /export/nfs/rpi/lib/optee_armtz
cp host/optee_example_hotp /export/nfs/rpi/usr/bin
```

## Execute on target
```
optee_example_hotp
```

