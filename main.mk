# Makefile's name
HERE := $(lastword $(MAKEFILE_LIST))

# Multi-platform
-include multiplatform.mk

OUTPUT_DIR = bin$(SL)main$(SL)
OUTPUT_NAME = plipUltimate
OUT = $(OUTPUT_DIR)$(OUTPUT_NAME)
ELF = $(OUT).elf

CC_FLAGS = -Wall -std=c11 -mmcu=atmega328p
LINK_FLAGS = -mmcu=atmega328p -Wl,-Map=$(OUT).map,--cref

SRC_DIR = src$(SL)main$(SL)
OBJ_DIR = obj$(SL)main$(SL)
INC_DIR = inc

MAIN_SRCS = $(wildcard $(SRC_DIR)*.c)
NET_SRCS = $(wildcard $(SRC_DIR)net$(SL)*.c)
BASE_SRCS = $(wildcard $(SRC_DIR)base$(SL)*.c)
SPI_SRCS = $(wildcard $(SRC_DIR)spi$(SL)*.c)

MAIN_OBJS = $(addprefix $(OBJ_DIR), $(notdir $(MAIN_SRCS:.c=.o)))
NET_OBJS = $(addprefix $(OBJ_DIR)net_, $(notdir $(NET_SRCS:.c=.o)))
BASE_OBJS = $(addprefix $(OBJ_DIR)base_, $(notdir $(BASE_SRCS:.c=.o)))
SPI_OBJS = $(addprefix $(OBJ_DIR)spi_, $(notdir $(SPI_SRCS:.c=.o)))

OBJS = $(MAIN_OBJS) $(NET_OBJS) $(BASE_OBJS) $(SPI_OBJS)

CC = avr-gcc
CC_FLAGS_DEBUG = -g
CC_FLAGS_RELEASE = -Os -fno-common
CC_FLAGS_COMMON = -std=c11 -mmcu=atmega328p -Wall -Werror -Wstrict-prototypes

CC_FLAGS = $(CC_FLAGS_RELEASE) -I$(INC_DIR) $(CC_FLAGS_COMMON)

plipUltimate.elf: $(OBJS)
	@echo Linking: $@
	@$(CC) $(LINK_FLAGS) -o "$(OUTPUT_DIR)$@" $(OBJS)
	@$(MAKE) --no-print-directory post-build -f $(HERE)

all: clean plipUltimate.elf

clean:
	$(RM) obj\main\*.o

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

$(OBJ_DIR)net_%.o: $(SRC_DIR)net$(SL)%.c
	@echo Building file: $<
	@$(CC) $(CC_FLAGS) -c -o "$@" "$<"

$(OBJ_DIR)base_%.o: $(SRC_DIR)base$(SL)%.c
	@echo Building file: $<
	@$(CC) $(CC_FLAGS) -c -o "$@" "$<"

$(OBJ_DIR)spi_%.o: $(SRC_DIR)spi$(SL)%.c
	@echo Building file: $<
	@$(CC) $(CC_FLAGS) -c -o "$@" "$<"
