OS ?= $(shell uname)
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

ifeq ($(OS),Windows_NT)
  EXECUTABLE_EXT := .exe
  ifeq ($(SHELL),cmd)
    RM := del /Q
    MKDIR_P := mkdir
  endif
else
  EXECUTABLE_EXT :=
endif

ifeq ($(BUILD),debug)
  OPT ?= -O0 -g
  DEFS += -DDEBUG
else
  OPT ?= -O2
endif

CXXFLAGS += $(WARN) -std=$(STD) $(OPT)
LDFLAGS ?=

ifeq ($(OS),Windows_NT)
  LDLIBS ?= -lpdcurses
else ifeq ($(OS),Darwin)
  LDLIBS ?= -lncurses
else
  LDLIBS ?= -lncurses
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

ARCH_SUFFIX :=
ifeq ($(ARCH),64)
  CXXFLAGS += -m64
  ARCH_SUFFIX := _64
endif
ifeq ($(ARCH),arm)
  ARCH_SUFFIX := _arm
  ifneq ($(findstring aarch64,$(CXX)),)
  else ifneq ($(findstring arm,$(CXX)),)
  else ifneq ($(findstring clang,$(CXX)),)
    ifdef CXXFLAGS_ARM
      CXXFLAGS += $(CXXFLAGS_ARM)
    endif
  else
    ifeq ($(shell $(CXX) -dumpmachine 2>/dev/null | grep -E '(aarch64|arm)'),)
      $(warning Warning: ARCH=arm specified but $(CXX) is not an ARM cross-compiler)
    else
      CXXFLAGS += -march=armv8-a
    endif
  endif
endif

ifeq ($(OS),Darwin)
  OS_SUFFIX := _macos
else ifeq ($(OS),Linux)
  OS_SUFFIX := _linux
else
  OS_SUFFIX := _other
endif

DIST_ZIP := $(BUILD_DIR)/$(TARGET)$(ARCH_SUFFIX)$(OS_SUFFIX).zip

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
	@echo "  ARCH=64|arm (optional, current: $(ARCH))";
	@echo "  CXX (current: $(CXX))";
	@echo "Examples:";
	@echo "  make BUILD=debug";
	@echo "  make ARCH=arm";
	@echo "  make run-tui FILE=samples/sample_01.txt";
	@echo "  make dist";
	@echo "  make distcheck";

format:
	@echo "(Placeholder) Add formatting step here if desired."

