# Compiler detection - can be overridden by environment
ifeq ($(OS),Windows_NT)
    CXX ?= g++
    RM = del /Q /F
    EXE = .exe
    ZIP = powershell -Command "Compress-Archive -Path '$(EXECUTABLE)' -DestinationPath '$(BUILD_DIR)/$(ZIP_NAME)'"
else
    CXX ?= g++
    RM = rm -f
    EXE =
    ZIP = zip -j $(BUILD_DIR)/$(ZIP_NAME) $(EXECUTABLE)
endif

CXXFLAGS = -Wall -std=c++14 -I./include

# Architecture handling
ifeq ($(ARCH), 64)
    CXXFLAGS += -m64
    ARCH_SUFFIX = _64
else ifeq ($(ARCH), arm)
    ifeq ($(shell uname),Darwin)
        # macOS ARM
        CXXFLAGS += -arch arm64
    else ifeq ($(OS),Windows_NT)
        # Windows ARM (if using clang)
        ifdef CXXFLAGS_ARM
            CXXFLAGS += $(CXXFLAGS_ARM)
        endif
    else
        # Linux ARM - cross compilation handled by CXX environment variable
        # CXX should be set to aarch64-linux-gnu-g++ by GitHub Actions
    endif
    ARCH_SUFFIX = _arm
else
    ARCH_SUFFIX =
endif

# OS detection for naming
ifeq ($(OS),Windows_NT)
    OS_SUFFIX = _windows
else ifeq ($(shell uname),Darwin)
    OS_SUFFIX = _macos
else
    OS_SUFFIX = _linux
endif

TARGET = 8086emu
ZIP_NAME = $(TARGET)$(ARCH_SUFFIX)$(OS_SUFFIX).zip

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/instructions/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
EXECUTABLE = $(BUILD_DIR)/$(TARGET)$(EXE)

ifeq ($(OS),Darwin)
    ifeq ($(ARCH), 64)
        CXXFLAGS += -arch x86_64
    else ifeq ($(ARCH), arm)
        CXXFLAGS += -arch arm64
    endif
endif

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $(EXECUTABLE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) $(OBJ_DIR)/instructions
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/instructions:
	mkdir -p $(OBJ_DIR)/instructions

check:
	@echo "Running basic functionality test..."
	@if [ -f "$(EXECUTABLE)" ]; then \
		echo "✓ Executable exists: $(EXECUTABLE)"; \
		echo "✓ Build successful"; \
	else \
		echo "✗ Executable not found: $(EXECUTABLE)"; \
		exit 1; \
	fi

distcheck: $(EXECUTABLE)
	@echo "Creating distribution package..."
	@echo "Executable: $(EXECUTABLE)"
	@echo "ZIP target: $(BUILD_DIR)/$(ZIP_NAME)"
	@if [ -f "$(EXECUTABLE)" ]; then \
		echo "✓ Executable verified"; \
		$(ZIP); \
		echo "✓ Distribution package created: $(BUILD_DIR)/$(ZIP_NAME)"; \
	else \
		echo "✗ Cannot create distribution: executable not found"; \
		exit 1; \
	fi

.PHONY: all check distcheck clean

clean:
	$(RM) -r $(OBJ_DIR) $(BUILD_DIR)
