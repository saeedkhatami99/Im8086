ifeq ($(OS),Windows_NT)
    CXX = g++
    RM = del /Q /F
    RM_DIR = rmdir /Q /S
    MKDIR = if not exist $@ mkdir $@
    EXE = .exe
else
    CXX = g++
    RM = rm -f
    RM_DIR = rm -rf
    MKDIR = mkdir -p
    EXE =
endif

CXXFLAGS = -Wall -std=c++11 -I./include

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
EXECUTABLE = $(BUILD_DIR)/emulator$(EXE)

# $(info SOURCES: $(SOURCES))
# $(info OBJECTS: $(OBJECTS))

all: make_directories $(EXECUTABLE)

make_directories: $(BUILD_DIR) $(OBJ_DIR)

$(BUILD_DIR):
	@$(MKDIR)

$(OBJ_DIR):
	@$(MKDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXECUTABLE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_DIR)" $(RM_DIR) "$(BUILD_DIR)"
else
	$(RM_DIR) $(BUILD_DIR)
endif

.PHONY: all clean make_directories
