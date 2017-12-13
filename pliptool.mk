# Makefile's name
HERE := $(lastword $(MAKEFILE_LIST))

# Multi-platform
-include multiplatform.mk

OUTPUT_DIR = bin$(SL)pliptool$(SL)
OUTPUT_NAME = pliptool
OUT = $(OUTPUT_DIR)$(OUTPUT_NAME)

SRC_DIR = src$(SL)pliptool$(SL)
OBJ_DIR = obj$(SL)pliptool$(SL)
INC_DIR = inc

MAIN_SRCS = $(wildcard $(SRC_DIR)*.c)
MAIN_OBJS = $(addprefix $(OBJ_DIR), $(notdir $(MAIN_SRCS:.c=.o)))

OBJS = $(MAIN_OBJS)

# -sd doesn't work because of ciaa and ciab
CC = vc
CC_FLAGS_DEBUG = -g
CC_FLAGS_RELEASE = -speed -sc
CC_FLAGS_COMMON = +kick13 -c99

CC_FLAGS = $(CC_FLAGS_RELEASE) -I$(INC_DIR) $(CC_FLAGS_COMMON)

LINK_FLAGS = +kick13 -lamiga


pliptool: $(OBJS)
	@echo Linking: $@
	@$(CC) $(LINK_FLAGS) -o "$(OUTPUT_DIR)$@" $(OBJS)

all: clean pliptool

clean:
	@$(RM) obj\pliptool\*.o

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@echo Building file: $<
	@$(CC) $(CC_FLAGS) -c -o "$@" "$<"
