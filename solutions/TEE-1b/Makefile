-include $(PROJECT_ROOT)/int/project.include

all: clean build nfs

.PHONY: build
build:
	make -C host
	make -C ta

.PHONY: clean
clean:
	make -C host $@
	make -C ta $@

.PHONY: nfs
nfs:
	cp ta/*.ta $(NFS_ROOT)/lib/optee_armtz
	cp host/signer-tee $(NFS_ROOT)/usr/bin
