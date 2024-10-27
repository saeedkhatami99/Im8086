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

CXXFLAGS = -Wall -std=c++11 -I./include

ifeq ($(ARCH), 64)
    CXXFLAGS += -m64
    ARCH_NAME = x86-64
else ifeq ($(ARCH), 32)
    CXXFLAGS += -m32
    ARCH_NAME = x86
else ifeq ($(ARCH), arm)
    CXXFLAGS += -arch arm64
    ARCH_NAME = arm64
endif

ifeq ($(OS),Windows_NT)
    OS_NAME = win
else ifeq ($(shell uname),Darwin)
    OS_NAME = mac
else
    OS_NAME = linux
endif

ZIP_NAME = emulator_$(ARCH_NAME)_$(OS_NAME).zip

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
EXECUTABLE = $(BUILD_DIR)/emulator$(EXE)

ifeq ($(OS),Darwin)
    ifeq ($(ARCH), 32)
        CXXFLAGS += -arch i386
    else ifeq ($(ARCH), 64)
        CXXFLAGS += -arch x86_64
    else ifeq ($(ARCH), arm)
        CXXFLAGS += -arch arm64
    endif
endif

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXECUTABLE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

check:
	@echo "Running tests..."
	# Add commands to run your tests here

distcheck:
	@echo "Creating distribution package..."
	$(ZIP)
	@echo "Distribution package created: $(BUILD_DIR)/$(ZIP_NAME)"

.PHONY: all check distcheck
