PRU_CGT=/usr/share/ti/cgt-pru
PROJ_NAME=pru
LINKER_COMMAND_FILE=./AM335x_PRU.cmd
LIBS=--library=lib/rpmsg_lib.lib
INCLUDE=--include_path=include --include_path=include/am335x
STACK_SIZE=0x100
HEAP_SIZE=0x100
GEN_DIR=gen

CFLAGS=-v3 -O2 --display_error_number --endian=little --hardware_mac=on --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) -ppd -ppa
LFLAGS=--reread_libs --warn_sections --stack_size=$(STACK_SIZE) --heap_size=$(HEAP_SIZE)

TARGET=$(GEN_DIR)/$(PROJ_NAME).out
MAP=$(GEN_DIR)/$(PROJ_NAME).map
SOURCES=$(wildcard *.c)

#Using .object instead of .obj in order to not conflict with the CCS build process
OBJECTS=$(patsubst %,$(GEN_DIR)/%,$(SOURCES:.c=.object))

all: $(TARGET)

# Invokes the linker (-z flag) to make the .out file
$(TARGET): $(OBJECTS) $(LINKER_COMMAND_FILE)
	$(PRU_CGT)/bin/clpru $(CFLAGS) -z -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $(TARGET) $(OBJECTS) -m$(MAP) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)

# Invokes the compiler on all c files in the directory to create the object files
$(GEN_DIR)/%.object: %.c
	mkdir -p $(GEN_DIR)
	$(PRU_CGT)/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

.PHONY: all clean

# Includes the dependencies that the compiler creates (-ppd and -ppa flags)
-include $(OBJECTS:%.object=%.pp)

