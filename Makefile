.PHONY: all clean release update_build_number run

SRC_DIR            := src
OBJ_DIR            := build/obj
BIN_DIR            := build
TARGET_EXE         := $(BIN_DIR)/supademo.exe
TARGET_ISO         := $(BIN_DIR)/supademo.iso
BUILD_NUMBER_FILE  := build_number.txt
BUILD_INFO_HEADER  := include/build_info.h

DJGPP_PREFIX       ?= $(HOME)/.local/djgpp
DJGPP_TOOLS_PREFIX ?= $(DJGPP_PREFIX)/bin/i586-pc-msdosdjgpp-

CXX     := $(DJGPP_TOOLS_PREFIX)g++
STRIP   := $(DJGPP_TOOLS_PREFIX)strip

CXXFLAGS += -Os
CXXFLAGS += -march=i386
CXXFLAGS += -std=gnu++23
CXXFLAGS += -Wall -Wextra -Werror
CXXFLAGS += -MMD -MP
CXXFLAGS += -ffunction-sections -fdata-sections
CXXFLAGS += -fno-ident
CXXFLAGS += -fno-stack-protector -fno-exceptions
CXXFLAGS += -fno-asynchronous-unwind-tables
CXXFLAGS += -Iinclude -Ietl/include -I.

LDFLAGS  := -L$(DJGPP_PREFIX)/i586-pc-msdosdjgpp/lib
LDFLAGS  += -Wl,--gc-sections -s

SRCS_CXX := $(shell find $(SRC_DIR) -name "*.cc")
OBJS := $(SRCS_CXX:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

all: update_build_number $(TARGET_EXE) $(TARGET_ISO)
	@echo $(BUILD_MSG)
	@$(MAKE) $(TARGET_EXE) --no-print-directory

# Read build number from file and increment it. Then write the build number to a
# header file.
update_build_number:
	@BUILD_NUMBER=$$(cat $(BUILD_NUMBER_FILE)); \
	echo $$((BUILD_NUMBER + 1)) > $(BUILD_NUMBER_FILE); \
	echo "#pragma once" > $(BUILD_INFO_HEADER); \
	echo "#define BUILD_NUMBER $$((BUILD_NUMBER + 1))" >> $(BUILD_INFO_HEADER)

$(TARGET_EXE): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@$(STRIP) --strip-all $@
	@chmod -x $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_ISO): $(TARGET_EXE)
	@mkdir -p $(BIN_DIR)/release
	@cp -f $(TARGET_EXE) $(BIN_DIR)/release
	@xorriso -report_about sorry -outdev $(TARGET_ISO) -blank as_needed -map $(BIN_DIR)/release /
	@du -sh $(TARGET_EXE)
	@echo "Build number $(BUILD_NUMBER_FILE)"

run:
	@dosemu -dumb $(TARGET_EXE)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

-include $(DEPS)
