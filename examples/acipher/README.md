# OP-TEE acipher application

## Build

### Build TA
```
cdex && cd acipher/ta
make clean && make
```

### Build host
```
cdex && cd acipher/host
make clean && make
```

## Copy to target
```
cdex && cd acipher
cp ta/a734eed9-d6a1-4244-aa50-7c99719e7b7b.ta /export/nfs/rpi/lib/optee_armtz
cp host/optee_example_acipher /export/nfs/rpi/usr/bin
```

## Execute on target
```
optee_example_acipher 1024 Michael
optee_example_acipher 4096 Michael
```

