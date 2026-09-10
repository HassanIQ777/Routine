CXX := g++
TARGET := routine
BINDIR := .

# Where 'make install' drops the binary
PREFIX  ?= $(HOME)/.local
DESTDIR ?=
INSTALLDIR := $(DESTDIR)$(PREFIX)/bin

# Library directory
LIB_UTILS_DIR := libutils
LIB_UTILS_LIB := $(LIB_UTILS_DIR)/libutils.a

# Root program sources
ROOT_SRCS := $(wildcard *.cpp)
ROOT_OBJS := $(patsubst %.cpp,%.o,$(ROOT_SRCS))

# If libutils exists, add includes
ifeq ($(wildcard $(LIB_UTILS_DIR)),)
  INCLUDES :=
  LIB_OBJS :=
  LIB_TARGET :=
else
  INCLUDES := -I$(LIB_UTILS_DIR)

  # Case 1: If libutils.a already exists, just use it
  ifneq ($(wildcard $(LIB_UTILS_LIB)),)
    LIB_TARGET := $(LIB_UTILS_LIB)
    LIB_OBJS :=
  else
    # Case 2: Build from sources if they exist
    LIB_SRCS := $(wildcard $(LIB_UTILS_DIR)/*.cpp)
    ifneq ($(LIB_SRCS),)
      LIB_OBJS := $(patsubst %.cpp,%.o,$(LIB_SRCS))
    else
      LIB_OBJS :=
    endif
  endif
endif

OBJS := $(ROOT_OBJS) $(LIB_OBJS)

# ---- Compiler flags ----
WARNINGS := -Wall -Wextra -Wpedantic -Wconversion

RELEASE_FLAGS := -std=c++20 -O2 -march=native $(WARNINGS)
DEBUG_FLAGS   := -std=c++20 -g -Og -DDEBUG $(WARNINGS) \
                 -fsanitize=address,undefined -fno-omit-frame-pointer

# Default is release
CXXFLAGS := $(RELEASE_FLAGS)

.PHONY: all debug release clean run install uninstall

all: release

release: CXXFLAGS := $(RELEASE_FLAGS)
release: $(BINDIR)/$(TARGET)

debug: CXXFLAGS := $(DEBUG_FLAGS)
debug: $(BINDIR)/$(TARGET)

# Link step
$(BINDIR)/$(TARGET): $(OBJS) $(LIB_TARGET)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LIB_TARGET)
	@echo "Built -> $@"

# Compile rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# If libutils.a exists and has its own Makefile, defer build to it
$(LIB_UTILS_LIB):
	@if [ -f $(LIB_UTILS_DIR)/Makefile ]; then \
	  echo "Building libutils..."; \
	  $(MAKE) -C $(LIB_UTILS_DIR); \
	fi

run: all
	@./$(BINDIR)/$(TARGET)

install: release
	@mkdir -p $(INSTALLDIR)
	@install -m 755 $(BINDIR)/$(TARGET) $(INSTALLDIR)/$(TARGET)
	@echo "Installed -> $(INSTALLDIR)/$(TARGET)"
	@echo "Make sure $(PREFIX)/bin is in your PATH."

uninstall:
	@rm -f $(INSTALLDIR)/$(TARGET)
	@echo "Uninstalled $(TARGET). It was fun while it lasted."

clean:
	-@rm -f $(OBJS)
	-@rm -f $(BINDIR)/$(TARGET)
	@echo "Cleaned up the ashes. Nothing but echoes remain..."

.SUFFIXES: