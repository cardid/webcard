ifeq ($(OS),Windows_NT)
	CCFLAGS += -D WIN32
	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		CCFLAGS += -D AMD64
	else
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			CCFLAGS += -D AMD64
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			CCFLAGS += -D IA32
		endif
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CCFLAGS += -D LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		CCFLAGS += -D OSX
	endif
endif

CC := clang++

PCSC_CFLAGS := -framework PCSC

CXXFLAGS += $(PCSC_CFLAGS)

FLAGS := $(CXXFLAGS) -std=c++11

all: clean build webcard

webcard: webcard.cpp
	$(CC) $(FLAGS) -o bin/webcard webcard.cpp

build:
	mkdir bin

clean:
	rm -rf bin