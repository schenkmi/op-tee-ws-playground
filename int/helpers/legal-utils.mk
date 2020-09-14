####################################################################
# Helper functions for creating OSS stuff
# M.Schenk
# 2020.03.16
#

# $(call legal-create,$(EXE),$(VERSION),$(LICENSE),$(LICENSE_FILES),$(PACKAGE),$(PATCH_LIST))
define legal-create
	echo 'legal-create with "$(1)","$(2)","$(3)","$(4)","$(5)","$(6)"'
	
	$(TEST) ! -d $(OSS_SOURCES_DIR)/$(1)-$(2) || rm -rf $(OSS_SOURCES_DIR)/$(1)-$(2)
	$(TEST) ! -d $(OSS_LICENSES_DIR)/$(1)-$(2) || rm -rf $(OSS_LICENSES_DIR)/$(1)-$(2)
	mkdir -p $(OSS_SOURCES_DIR)/$(1)-$(2) $(OSS_LICENSES_DIR)/$(1)-$(2)
	
	# copy package, patches and license files
	if [ -n "${5}" ]; then  										\
	  cp ${5} ${OSS_SOURCES_DIR}/${1}-${2};							\
	fi;
	if [ -n "${6}" ]; then  										\
	  cp ${6} ${OSS_SOURCES_DIR}/${1}-${2};							\
	fi;
	if [ -n "${4}" ]; then  										\
	  cd ${SOURCE_DIR} && cp ${4} ${OSS_LICENSES_DIR}/${1}-${2};	\
	fi;
	
	# remove old entry from manifest.csv
	@$(TEST) ! -f $(OSS_DIR)/manifest.csv || sed -i -e "/^\"$(1)*/d" $(OSS_DIR)/manifest.csv
	echo '"$(1)","$(2)","$(3)","$(4)","$(5)"' >>$(OSS_DIR)/manifest.csv
endef
