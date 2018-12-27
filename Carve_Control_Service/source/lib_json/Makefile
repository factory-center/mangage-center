#common makefile header
SLNDIR := $(CURDIR)/../../
include $(SLNDIR)/prj/linux/configure.mk

#this module is $(LIBOUTPUT) liblib_json.a
MODULE_TARGET := $(MODULE_LIB)
LOCAL_INCLUDE  :=  -I$(CURDIR)/
LOCAL_FLAG := $(PIC_FLAG) 


# additional include dir

MODULE_OBJ = $(MODULE_OUTPUT)/json_reader.o \
	     $(MODULE_OUTPUT)/json_value.o \
	     $(MODULE_OUTPUT)/json_writer.o

#common makefile foot
include $(SLNDIR)/prj/linux/rules.mk

#automatically trace files
.KEEP_STATE_FILE: $(MODULE_OUTPUT)
