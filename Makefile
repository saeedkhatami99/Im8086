HOST_UNAME ?= $(shell uname -s)
OS ?= $(HOST_UNAME)
CXX ?= g++
AR ?= ar
RM ?= rm -f
MKDIR_P ?= mkdir -p
BUILD ?= release
TARGET ?= 8086emu
STD ?= c++17
WARN ?= -Wall -Wextra -Wpedantic
DEFS ?=
INCLUDES ?= -Iinclude
CPPFLAGS ?= $(DEFS) $(INCLUDES)
TARGET_OS ?=
TARGET_ARCH ?=
CROSS_PREFIX ?=

ifeq ($(TARGET_OS),)
	ifeq ($(OS),Windows_NT)
		TARGET_OS := windows
	else ifeq ($(OS),Darwin)
		TARGET_OS := macos
	else ifeq ($(OS),Linux)
		TARGET_OS := linux
	else
		TARGET_OS := other
	endif
endif

ifeq ($(TARGET_ARCH),)
	UNAME_M := $(shell uname -m)
	ifeq ($(UNAME_M),x86_64)
		TARGET_ARCH := x86_64
	else ifeq ($(UNAME_M),amd64)
		TARGET_ARCH := x86_64
	else
		TARGET_ARCH := unknown
	endif
endif

ifeq ($(TARGET_OS),windows)
	EXECUTABLE_EXT := .exe
else
	EXECUTABLE_EXT :=
endif

ifneq ($(CROSS_PREFIX),)
	CXX := $(CROSS_PREFIX)g++
	AR := $(CROSS_PREFIX)ar
endif

ifeq ($(BUILD),debug)
  OPT ?= -O0 -g
  DEFS += -DDEBUG
else
  OPT ?= -O2
endif

CXXFLAGS += $(WARN) -std=$(STD) $(OPT)
LDFLAGS ?=

LDLIBS ?=
ifeq ($(TARGET_OS),windows)
	LDLIBS += -lpdcurses
else ifeq ($(TARGET_OS),macos)
	LDLIBS += -lncurses
else ifeq ($(TARGET_OS),linux)
	LDLIBS += -lncurses
endif

SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/dep

SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
			$(wildcard $(SRC_DIR)/instructions/*.cpp)

OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

EXECUTABLE := $(BUILD_DIR)/$(TARGET)$(EXECUTABLE_EXT)

OS_SUFFIX := _$(TARGET_OS)
ARCH_SUFFIX := _$(TARGET_ARCH)

ARTIFACT_BASENAME := $(TARGET)$(ARCH_SUFFIX)$(OS_SUFFIX)
DIST_ZIP := $(BUILD_DIR)/$(ARTIFACT_BASENAME).zip

.PHONY: all
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) $(DEP_DIR)
	@$(MKDIR_P) $(dir $(DEP_DIR)/$*.d)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $(DEP_DIR)/$*.d -c $< -o $@

-include $(DEPS)

$(BUILD_DIR):
	$(MKDIR_P) $(BUILD_DIR)
$(OBJ_DIR):
	$(MKDIR_P) $(OBJ_DIR) $(OBJ_DIR)/instructions
$(DEP_DIR):
	$(MKDIR_P) $(DEP_DIR) $(DEP_DIR)/instructions

.PHONY: run run-tui rebuild clean dist distcheck check help format

run: $(EXECUTABLE)
	$(EXECUTABLE)

run-tui: $(EXECUTABLE)
ifeq ($(FILE),)
	@echo "Specify FILE=<program file> e.g. make run-tui FILE=samples/sample_01.txt" && exit 1
else
	$(EXECUTABLE) --tui $(FILE)
endif

rebuild: clean all

clean:
	$(RM) -r $(BUILD_DIR)

dist: $(EXECUTABLE)
	@echo "Creating distribution: $(DIST_ZIP)";
	cd $(BUILD_DIR) && zip -q $(notdir $(DIST_ZIP)) $(notdir $(EXECUTABLE))
	@echo "Done."

.PHONY: package
package: dist

distcheck: dist
	@echo "Distribution check completed for $(DIST_ZIP)"
	@if [ -f "$(EXECUTABLE)" ]; then \
		echo "✓ Executable exists: $(EXECUTABLE)"; \
		echo "✓ Distribution package: $(DIST_ZIP)"; \
	else \
		echo "✗ Executable not found: $(EXECUTABLE)"; \
		exit 1; \
	fi

check: $(EXECUTABLE)
	@echo "Executable size: $$(stat -c%s $(EXECUTABLE) 2>/dev/null || stat -f%z $(EXECUTABLE)) bytes"
	@echo "✓ Build OK"

help:
	@echo "Targets:";
	@echo "  all (default)   Build executable";
	@echo "  run              Run REPL";
	@echo "  run-tui FILE=..  Run TUI with program file";
	@echo "  rebuild          Clean then build";
	@echo "  clean            Remove build artifacts";
	@echo "  dist             Create zip distribution";
	@echo "  distcheck        Create and verify distribution";
	@echo "  check            Basic build verification";
	@echo "Variables:";
	@echo "  BUILD=debug|release (current: $(BUILD))";
	@echo "  TARGET (current: $(TARGET))";
	@echo "  TARGET_OS=linux|macos|windows (current: $(TARGET_OS))";
	@echo "  TARGET_ARCH=x86_64 (current: $(TARGET_ARCH))";
	@echo "  CROSS_PREFIX (current: $(CROSS_PREFIX))";
	@echo "  CXX (current: $(CXX))";
	@echo "Examples:";
	@echo "  make BUILD=debug";
	@echo "  make TARGET_OS=linux";
	@echo "  make run-tui FILE=samples/sample_01.txt";
	@echo "  make dist";
	@echo "  make distcheck";

format:
	@echo "(Placeholder) Add formatting step here if desired."

