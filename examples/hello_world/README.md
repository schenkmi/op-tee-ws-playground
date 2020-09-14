# OP-TEE hello world application

## Build

### Build TA
```
cdex && cd hello_world/ta
make clean && make
```

### Build host
```
cdex && cd hello_world/host
make clean && make
```

## Copy to target
```
cdex && cd hello_world
cp ta/8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta /export/nfs/rpi/lib/optee_armtz
cp host/optee_example_hello_world /export/nfs/rpi/usr/bin
```

## Execute on target
```
optee_example_hello_world
```

