PROJECT=np34
OBJECTS=$(PROJECT).o

GCC_DIR=/usr/local/ti/msp430-gcc/bin
SUPPORT_FILE_DIRECTORY = /usr/local/ti/msp430-gcc/include

# Please set your device here
DEVICE = msp430g2744
CC = $(GCC_DIR)/msp430-elf-gcc
GDB = $(GCC_DIR)/msp430-elf-gdb

CFLAGS = -D EMBEDDED -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -Os -g \
-ffunction-sections -fdata-sections -fno-inline-small-functions \
-Wl,--relax -Wl,--gc-sections,--section-start=.rodata_prg38c=0x0fb00,--section-start=.rodata_prg34c=0x0fc00

LFLAGS = -L $(SUPPORT_FILE_DIRECTORY) -T $(DEVICE).ld 

all: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(PROJECT).out
	$(GCC_DIR)/msp430-elf-objdump -DS $(PROJECT).out > $(PROJECT).lst
	$(GCC_DIR)/msp430-elf-strip $(PROJECT).out
	$(GCC_DIR)/msp430-elf-size --totals $(PROJECT).out
	$(GCC_DIR)/msp430-elf-objcopy -O ihex $(PROJECT).out $(PROJECT).hex

debug: all
	$(GDB) $(PROJECT).out

clean:
	rm *.o *.out *.elf *.map *.hex *.lst 2> /dev/null

