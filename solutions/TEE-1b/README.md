# OP-TEE singer application

## Create UUID
```
python -c "import uuid; u=uuid.uuid4(); print(u); \
   n = [', 0x'] * 11; \
   n[::2] = ['{:12x}'.format(u.node)[i:i + 2] for i in range(0, 12, 2)]; \
   print('\n' + '#define TA_UUID\n\t{ ' + \
         '0x{:08x}'.format(u.time_low) + ', ' + \
         '0x{:04x}'.format(u.time_mid) + ', ' + \
         '0x{:04x}'.format(u.time_hi_version) + ', \\ \n\n\t\t{ ' + \
         '0x{:02x}'.format(u.clock_seq_hi_variant) + ', ' + \
         '0x{:02x}'.format(u.clock_seq_low) + ', ' + \
         '0x' + ''.join(n) + '} }')"
```

## Build

### Build TA
```
cdta && cd signer-tee/ta
make clean && make
```

### Build host
```
cdta && cd signer-tee/host
make clean && make
```

## Copy to target
```
cdta && cd signer-tee
cp ta/34b955bf-3459-40f4-ab23-3b2deed0ec14.ta /export/nfs/rpi/lib/optee_armtz
cp host/signer-tee /export/nfs/rpi/usr/bin
```

## Execute on target
```
signer-tee
```

