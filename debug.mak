#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug

#Toolchain
CC := gcc
CXX := g++
LD := $(CXX)
AR := ar
OBJCOPY := objcopy

#Additional flags
PREPROCESSOR_MACROS := DEBUG=1
INCLUDE_DIRS := ./iutils/include ./json
LIBRARY_DIRS := ./lib
LIBRARY_NAMES := readline ncurses pthread dl termcap
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 



CFLAGS :=  -ggdb
CXXFLAGS :=  -ggdb
ASFLAGS := 
LDFLAGS := -static
COMMONFLAGS := 
LINKER_SCRIPT := 

START_GROUP := 
END_GROUP := 

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1




CFLAGS += -Dlinux -DLINUX -DMDC_MDIO_OPERATION -DRTK_X86_CLE
INCLUDE_DIRS +=  ./lib/rtl8367c/includes

