# OP-TEE random application

## Build

### Build TA
```
cdex && cd random/ta
make clean && make
```

### Build host
```
cdex && cd random/host
make clean && make
```

## Copy to target
```
cdex && cd random
cp ta/b6c53aba-9669-4668-a7f2-205629d00f86.ta /export/nfs/rpi/lib/optee_armtz
cp host/optee_example_random /export/nfs/rpi/usr/bin
```

## Execute on target
```
optee_example_random
```

