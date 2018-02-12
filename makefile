
SUBDIRS=mpx-lib sftp sftp-test

.PHONY: subdirs $(SUBDIRS)

all clean: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS):
	cd $@/Debug; $(MAKE) $(MAKECMDGOALS)

