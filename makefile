
SUBDIRS=mpx-lib examples/sftp examples/sftp-test examples/mpx-test-1 examples/mpx-test-02 examples/mpx-test-03 examples/mpx-edlib examples/mpx-task-provider examples/mpx-task-consumer examples/mpx-edtest examples/mpx-edlib-simple examples/mpx-task-provider-simple examples/mpx-task-consumer-simple examples/mpx-env

.PHONY: subdirs $(SUBDIRS)

all clean: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS):
	cd $@/$(CONFIGURATION); $(MAKE) $(MAKECMDGOALS)

