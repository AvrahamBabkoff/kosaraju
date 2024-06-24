# Define the compiler
CXX = g++

# Define compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Define the target executable
TARGET = bin/kosaraju

# List of source files
SRCS = kosaraju.c pollserver.c

# List of header files
HEADERS = kosaraju.h pollserver.h

# Define the directory for object files
OBJDIR = bin

# List of object files (replace .cpp with .o and add OBJDIR prefix)
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Rule to link object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^



# Rule to compile source files into object files
$(OBJDIR)/%.o: %.c $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)
	rmdir $(OBJDIR) 2>/dev/null || true
