-include $(PROJECT_ROOT)/int/project.include

CFLAGS=-g -Wall -Wextra $(shell pkg-config --cflags openssl)
LDFLAGS=$(shell pkg-config --libs openssl)

OBJS = ecverify.o
BINARY = ecverify

####################################################################################
# Dependencies generation defs
####################################################################################
DEPDIR=./.deps
DEPFLAGS=-MD -MF $(DEPDIR)/$(patsubst %.o,%.d,$@)

####################################################################################
# Build rules
####################################################################################
%.o : %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<
	@cd $(DEPDIR); cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

.PHONY: all
all: depdir $(BINARY)

nfs: all
	cp $(BINARY) $(NFS_ROOT)/usr/bin

depdir:
	@$(TEST) -d $(DEPDIR) || $(INSTALL) -d -m 775 $(DEPDIR)

$(BINARY): $(OBJS)
	$(CC) -o $@ $< $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)
	rm -rf .deps

-include $(patsubst %.o,$(DEPDIR)/%.P,$(depobj))