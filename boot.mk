# Makefile's name
HERE := $(lastword $(MAKEFILE_LIST))

# Multi-platform
-include multiplatform.mk

OUTPUT_DIR = bin$(SL)bootloader$(SL)
OUTPUT_NAME = bootloader
OUT = $(OUTPUT_DIR)$(OUTPUT_NAME)
ELF = $(OUT).elf

SRC_DIR = src$(SL)bootloader$(SL)
OBJ_DIR = obj$(SL)bootloader$(SL)
INC_DIR = inc

MAIN_SRCS = $(wildcard $(SRC_DIR)*.c)
MAIN_OBJS = $(addprefix $(OBJ_DIR), $(notdir $(MAIN_SRCS:.c=.o)))

OBJS = $(MAIN_OBJS)

CC = avr-gcc
CC_FLAGS_DEBUG = -g
CC_FLAGS_RELEASE = -Os -fno-common
CC_FLAGS_COMMON = -std=gnu11 -mmcu=atmega328p -Wall -Werror -Wstrict-prototypes

CC_FLAGS = $(CC_FLAGS_RELEASE) -I$(INC_DIR) $(CC_FLAGS_COMMON)

LINK_FLAGS = -mmcu=atmega328p \
	-Wl,-Map=$(OUT).map,--cref \
	-Wl,--section-start=.text=0x7000

bootloader.elf: $(OBJS)
	@echo Linking: $@
	@$(CC) $(LINK_FLAGS) -o "$(OUTPUT_DIR)$@" $(OBJS)
	@$(MAKE) --no-print-directory post-build -f $(HERE)

all: clean bootloader.elf

clean:
	@$(RM) obj\bootloader\*.o

post-build:
	@avr-size $(ELF)
	@cmd /c "avr-objdump -h -S $(ELF) > $(OUT).lss"
	@avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(ELF) $(OUT).hex
	@avr-objcopy --no-change-warnings -j .eeprom --change-section-lma .eeprom=0 -O ihex $(ELF) $(OUT).eep
	@avr-objcopy --no-change-warnings -j .lock --change-section-lma .lock=0 -O ihex $(ELF) $(OUT).lock
	@avr-objcopy --no-change-warnings -j .signature --change-section-lma .signature=0 -O ihex $(ELF) $(OUT).sig
	@avr-objcopy --no-change-warnings -j .fuse --change-section-lma .fuse=0 -O ihex $(ELF) $(OUT).fuse
	@srec_cat $(OUT).fuse -Intel -crop 0x00 0x01 -offset  0x00 -O $(OUT).lfs -Intel
	@srec_cat $(OUT).fuse -Intel -crop 0x01 0x02 -offset -0x01 -O $(OUT).hfs -Intel
	@srec_cat $(OUT).fuse -Intel -crop 0x02 0x03 -offset -0x02 -O $(OUT).efs -Intel

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@echo Building file: $<
	@$(CC) $(CC_FLAGS) -c -o "$@" "$<"
