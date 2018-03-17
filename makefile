
SUBDIRS=mpx-lib sftp sftp-test mpx-test-1 mpx-test-02 mpx-test-03 mpx-edlib mpx-task-provider mpx-task-consumer

.PHONY: subdirs $(SUBDIRS)

all clean: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS):
	cd $@/$(CONFIGURATION); $(MAKE) $(MAKECMDGOALS)

