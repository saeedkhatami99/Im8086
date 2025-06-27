ifeq ($(OS),Windows_NT)
    CXX = g++
    RM = del /Q /F
    EXE = .exe
    ZIP = powershell -Command "Compress-Archive -Path '$(EXECUTABLE)' -DestinationPath '$(BUILD_DIR)/$(ZIP_NAME)'"
else
    CXX = g++
    RM = rm -f
    EXE =
    ZIP = zip -j $(BUILD_DIR)/$(ZIP_NAME) $(EXECUTABLE)
endif

CXXFLAGS = -Wall -std=c++14 -I./include

ifeq ($(ARCH), 64)
    CXXFLAGS += -m64
    ARCH_NAME = 64
else ifeq ($(ARCH), arm)
    CXXFLAGS += -arch arm64
    ARCH_NAME = arm
endif

ifeq ($(OS),Windows_NT)
    OS_NAME = windows-latest
else ifeq ($(shell uname),Darwin)
    OS_NAME = macos-latest
else
    OS_NAME = ubuntu-latest
endif

ZIP_NAME = 8086emu_$(ARCH_NAME)_$(OS_NAME).zip

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/instructions/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
EXECUTABLE = $(BUILD_DIR)/8086emu$(EXE)

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
	@echo "Running tests..."

distcheck:
	@echo "Creating distribution package..."
	@echo "Target ZIP file: $(BUILD_DIR)/$(ZIP_NAME)"
	$(ZIP)
	@echo "Distribution package created: $(BUILD_DIR)/$(ZIP_NAME)"

.PHONY: all check distcheck clean

clean:
	$(RM) -r $(OBJ_DIR) $(BUILD_DIR)
