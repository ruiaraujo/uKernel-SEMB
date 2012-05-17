###############################################################################
# Makefile for project 
###############################################################################

## General options
PROJECT = 
MCU = ATmega8
COMPILER = "WinAVR"
TARGET = Debug
CLOCK = 14745600
SHELL = C:\WINDOWS\system32\cmd.exe

## Tools general options
CCFLAGS=-Wall -gdwarf-2 -fsigned-char -MD -MP -DF_CPU=14745600 -O1 -mmcu=atmega8
LDFLAGS=-mmcu=atmega8

## Processing Tools
CC=avr-gcc.exe
ASM=avr-gcc.exe
LD=avr-gcc.exe
#  C:/Documents and Settings/Administrator/My Documents/semb/uKernel-SEMB/Debug/Debug.elf: TOOL=LD
#  Post: $(OBJ) avr-objcopy -O ihex -R .eeprom "Debug/Debug.elf" "Debug/Debug.hex"
#  Post: $(OBJ) avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex "Debug/Debug.elf" "Debug/Debug.eep" || exit 0 
#     Debug/main.o: TOOL=CC
#        C:/Documents and Settings/Administrator/My Documents/semb/uKernel-SEMB/src/main.c
#     Debug/scheduler.o: TOOL=CC
#        C:/Documents and Settings/Administrator/My Documents/semb/uKernel-SEMB/src/scheduler.c

# Build tree:
all:	Debug

Debug:	Debug/Debug.elf

Debug/Debug.elf:	Debug/main.o Debug/scheduler.o 
	$(LD) $(LDFLAGS)  -o "Debug/Debug.elf" "Debug/main.o" "Debug/scheduler.o"
	$(OBJ) avr-objcopy -O ihex -R .eeprom "Debug/Debug.elf" "Debug/Debug.hex"
	$(OBJ) avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex "Debug/Debug.elf" "Debug/Debug.eep" || exit 0 

Debug/main.o:	src\main.c 
	$(CC) $(CCFLAGS)  -o "Debug/main.o" -c "src/main.c"

Debug/scheduler.o:	src\SCHEDU~1.C 
	$(CC) $(CCFLAGS)  -o "Debug/scheduler.o" -c "src/scheduler.c"

# tidy - delete all temporary files which are not involved in the target generation
tidy:
	rm -rf Debug/main.d
	rm -rf Debug/scheduler.d

# cleanup - delete all generated files
clean:	tidy
	rm -rf Debug/Debug.elf
	rm -rf Debug/main.o
	rm -rf Debug/scheduler.o
