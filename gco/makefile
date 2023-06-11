# Makefile for GCO-V3.0 project

# Compiler settings
CC = g++
CFLAGS = -std=c++11 -O2

# Include directories
INCLUDES = -I/.

# Source files
SRCS = example.cpp \
       GCoptimization.cpp \
	   graph.cpp \
	   LinkedBlockList.cpp \
	   maxflow.cpp

# Object files
OBJS = $(addprefix build/, $(notdir $(SRCS:.cpp=.o)))

# Executable name
TARGET = build/gco_example

# Build rule
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

# Object file rule
build/%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)
