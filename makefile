# Compiler
CXX = g++
CXXFLAGS = -Wall -fPIC -O2

# Source files (add all your .cpp files here)
SRC = modmenu.cpp hooks.cpp utils.cpp

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
