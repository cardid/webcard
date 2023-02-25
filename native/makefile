################################################################
# "Webcard" cross-platform project.
# List of required source files.

WEBCARD_HEADERS = \
  src/json/json.h \
  src/os_specific/os_specific.h \
  src/misc/misc.h \
  src/os_specific/wtypes_for_unix.h \
  src/smart_cards/smart_cards.h \
  src/utf/utf.h

WEBCARD_SOURCES = \
  src/webcard_main.c \
  src/json/json_array.c \
  src/json/json_bytestream.c \
  src/json/json_object.c \
  src/json/json_pair.c \
  src/json/json_string.c \
  src/json/json_value.c \
  src/misc/misc.c \
  src/os_specific/os_specific.c \
  src/smart_cards/sc_conn.c \
  src/smart_cards/sc_db.c \
  src/smart_cards/sc_webcard.c \
  src/utf/utf.c

################################################################
# Detecting Target Operating System and Processor Architecture.
# Selecting: "C Compiler", "Output directory",
#  "Output executable name", "C Compiler libraries".

MSG_OS_OK        = * Target OS detected as
MSG_OS_MISMATCH  = * Unsupported Operating System
MSG_CPU_OK       = * Processor Architecture detected as
MSG_CPU_MISMATCH = * Unsupported Processor Architecture

ifeq ($(OS),Windows_NT)
  $(info $(MSG_OS_OK) "Windows")
  CPU = $(PROCESSOR_ARCHITECTURE)

  ifeq ($(CPU),x86)
    $(info $(MSG_CPU_OK) 32-bit ($(CPU)))
    CC = i686-w64-mingw32-gcc.exe
    BINDIR = out/win32

  else ifeq ($(CPU),AMD64)
    $(info $(MSG_CPU_OK) 64-bit ($(CPU)))
    CC = x86_64-w64-mingw32-gcc.exe
    BINDIR = out/win64

  else
    $(error $(MSG_CPU_MISMATCH)! ($(CPU)))
  endif

  EXEC_WEBCARD = $(BINDIR)/webcard.exe
  LDFLAGS = -lwinscard

else
  OS = $(shell uname -s)

  ifeq ($(OS),Linux)
    $(info $(MSG_OS_OK) "Linux")
    CPU = $(shell uname -m)

    ifeq ($(CPU),i686)
      $(info $(MSG_CPU_OK) 32-bit ($(CPU)))
      CC = i686-linux-gnu-gcc
      BINDIR = out/linux32

    else ifeq ($(CPU),x86_64)
      $(info $(MSG_CPU_OK) 64-bit ($(CPU)))
      CC = x86_64-linux-gnu-gcc
      BINDIR = out/linux64

    else
      $(error $(MSG_CPU_MISMATCH)! ($(CPU)))
    endif

    EXEC_WEBCARD = $(BINDIR)/webcard
    LDFLAGS = -lpcsclite

  else ifeq ($(OS),Darwin)
    $(info $(MSG_OS_OK) "macOS")
    CPU = $(shell uname -m)

    ifeq ($(CPU),x86_64)
      $(info $(MSG_CPU_OK) 64-bit ($(CPU)))
      BINDIR = out/macos

    else ifeq ($(CPU),arm64)
      $(info $(MSG_CPU_OK) arm64 ($(CPU)))
      BINDIR = out/macos

    else
      $(error $(MSG_CPU_MISMATCH)! ($(CPU)))
    endif

    CC = clang
    EXEC_WEBCARD = $(BINDIR)/webcard_arm

    LDFLAGS = -framework PCSC -target arm64-apple-macos11

  else
    $(error $(MSG_OS_MISMATCH)! ($(OS)))
  endif
endif

################################################################
# Detecting current Shell ("CMD" or "SH").
# Selecting shell-specific commands.

ifeq ($(COMSPEC),)
  COMSPEC = $(ComSpec)
endif
ifneq ($(COMSPEC),)
  SHELL = $(COMSPEC)
  .SHELLFLAGS = /C
  DOS_BINDIR = $(subst /,\,$(BINDIR))
  SHELL_BINDIR_CHECK = IF NOT EXIST $(DOS_BINDIR)\ MD $(DOS_BINDIR)
  SHELL_RESDIR_CHECK = IF NOT EXIST res\out\ MD res\out
  SHELL_WHERE = where
  SHELL_DEVNUL = NUL
else
  SHELL_BINDIR_CHECK = mkdir -p $(BINDIR)
  SHELL_RESDIR_CHECK = mkdir -p res/out
  SHELL_WHERE = which
  SHELL_DEVNUL = /dev/null
endif

$(info * SHELL is `$(SHELL)`)

################################################################
# Check if all the required build tools are installed.

ifdef RES_WEBCARD
  WINDRES = windres
endif

BUILD_TOOLS = $(WINDRES) "$(CC)"

$(foreach tool,$(BUILD_TOOLS),\
  $(if $(shell $(SHELL_WHERE) $(tool) 2>$(SHELL_DEVNUL)),\
    ,\
    $(error * $(tool) not in PATH! Please refer to "README.MD")))

################################################################
# Header files are scanned starting from the "./src" directory

CPPFLAGS = -I./src

CFLAGS = -Wall -pedantic-errors

################################################################
# Recipes for specific targets.
# Selecting "Compiler flags" and "Linker flags"
#  depending on the target: "release" (default) or "debug".

.PHONY: release debug

release: CFLAGS += -O3
release: LDFLAGS += -s
release: $(EXEC_WEBCARD)

debug: CPPFLAGS += -D_DEBUG
debug: CFLAGS += -g
debug: LDFLAGS += -g
debug: $(EXEC_WEBCARD)

$(EXEC_WEBCARD): $(WEBCARD_HEADERS) $(WEBCARD_SOURCES) $(RES_WEBCARD)
	$(info )
	@$(SHELL_BINDIR_CHECK)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(WEBCARD_SOURCES) $(RES_WEBCARD) $(LDFLAGS)

$(RES_WEBCARD): res/webcard.rc res/webcard.ico
	$(info )
	@$(SHELL_RESDIR_CHECK)
	$(WINDRES) res/webcard.rc -O coff $(RES_WEBCARD)

################################################################
