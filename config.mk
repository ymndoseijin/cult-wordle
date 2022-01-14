# Customize below to fit your system

# paths
PREFIX = /usr/local

PKG_CONFIG = pkg-config

# includes and libs
INCS = 

LIBS = 

# flags
STCPPFLAGS =
STCFLAGS = $(INCS) $(STCPPFLAGS) $(CPPFLAGS) $(CFLAGS)
STLDFLAGS = $(LIBS) $(LDFLAGS)

# compiler and linker
# CC = c99
