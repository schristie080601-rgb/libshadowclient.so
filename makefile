# Compiler
CXX = g++
CXXFLAGS = -Wall -fPIC -O2

# Automatically find all .cpp files in the current directory
SRC = $(wildcard *.cpp)

# Object files
OBJ = $(SRC:.cpp=.o)

# Output shared library
TARGET = libshadowclient.so

# Default build
all: $(TARGET)

# Rule to build the .so
$(TARGET): $(OBJ)
    $(CXX) -shared -o $@ $(OBJ)

# Rule to build object files
%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts (cross-platform)
clean:
ifeq ($(OS),Windows_NT)
    del $(OBJ) $(TARGET)
else
    rm -f $(OBJ) $(TARGET)
endif
