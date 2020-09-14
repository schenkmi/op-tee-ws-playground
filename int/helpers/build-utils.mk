####################################################################
# Helper functions for building
# M.Schenk
# 2020.03.16
#

# $(call patch-source,$(SOURCE_DIR),$(PATCH_LIST))
define patch-source
	echo 'patch-source with "$(1)","$(2)"'
	
	@CURDIR=`pwd`;  						\
	for p in  $(2); do						\
	  echo "--> apply patch $$p in $(1)/"; 	\
	  cd $(1) && patch -p1 < ../$$p; 		\
	  cd $$CURDIR;							\
	done
endef
