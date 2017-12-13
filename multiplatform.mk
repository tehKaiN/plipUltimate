# Multi-platform
ifdef ComSpec
	# Windows
	ROOT = $(shell chdir)
	NEWLINE = @echo.
	RM = del
	SL = /
else
	# Linux
	ROOT = $(shell pwd)
	NEWLINE = @echo ""
	RM = rm
	SL = /
endif
