
CC = avr-gcc

CC_FLAGS_DEBUG = -g
CC_FLAGS_RELEASE = -Os

OUTPUT_DIR_DEBUG = bin/Debug
OUTPUT_DIR_RELEASE = bin/Release

CC_FLAGS = -Wall -std=c11 -mmcu=atmega328p
LINK_FLAGS = -mmcu=atmega328p -Wl,-Map=$(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).map,--cref

MAIN_SRCS = $(wildcard src/*.c)
NET_SRCS += $(wildcard src/net*.c)
BASE_SRCS += $(wildcard src/base*.c)
SPI_SRCS += $(wildcard src/spi*.c)

MAIN_OBJS = $(addprefix obj$(SL), $(notdir $(MAIN_SRCS:.c=.o)))
NET_OBJS = $(addprefix obj$(SL)net_, $(notdir $(NET_SRCS:.c=.o)))
BASE_OBJS = $(addprefix obj$(SL)base_, $(notdir $(BASE_SRCS:.c=.o)))
SPI_OBJS = $(addprefix obj$(SL)spi_, $(notdir $(SPI_SRCS:.c=.o)))

OBJS = MAIN_OBJS NET_OBJS BASE_OBJS SPI_OBS

after:
	avr-size --mcu=atmega328p --format=avr $(TARGET_OUTPUT_FILE)
	cmd /c \"avr-objdump -h -S $(TARGET_OUTPUT_FILE) > $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).lss\"
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(TARGET_OUTPUT_FILE) $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).hex
	avr-objcopy --no-change-warnings -j .eeprom --change-section-lma .eeprom=0 -O ihex $(TARGET_OUTPUT_FILE) $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).eep
	avr-objcopy --no-change-warnings -j .lock --change-section-lma .lock=0 -O ihex $(TARGET_OUTPUT_FILE) $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).lock
	avr-objcopy --no-change-warnings -j .signature --change-section-lma .signature=0 -O ihex $(TARGET_OUTPUT_FILE) $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).sig
	avr-objcopy --no-change-warnings -j .fuse --change-section-lma .fuse=0 -O ihex $(TARGET_OUTPUT_FILE) $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).fuse
	srec_cat $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).fuse -Intel -crop 0x00 0x01 -offset  0x00 -O $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).lfs -Intel
	srec_cat $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).fuse -Intel -crop 0x01 0x02 -offset -0x01 -O $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).hfs -Intel
	srec_cat $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).fuse -Intel -crop 0x02 0x03 -offset -0x02 -O $(TARGET_OUTPUT_DIR)$(TARGET_OUTPUT_BASENAME).efs -Intel

