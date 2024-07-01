# Define the compiler
CXX = gcc

# Define compiler flags
CXXFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

# Define the target executable
TARGET = bin/kosaraju

# List of source files
SRCS = kosaraju.c pollserver.c tcp_reactor.c tcp_dup.c listner.c tcp_threads.c

AR = ar

LIB_NAME = bin/async_engine.a

# List of library source files
LIB_SRCS = poll_reactor.c proactor.c

# LIB_OBJ = $(LIB_SRC:.c=.o)

# List of header files
HEADERS = kosaraju.h pollserver.h tcp_reactor.h tcp_dup.h listner.h tcp_threads.h poll_reactor.h proactor.h

# Define the directory for object files
OBJDIR = bin

# List of object files (replace .c with .o and add OBJDIR prefix)
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

LIB_OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(LIB_SRCS))
# Default target
all: $(LIB_NAME) $(TARGET)

$(LIB_NAME): $(LIB_OBJS)
	$(AR) rcs $@ $^


# Rule to link object files to create the executable
$(TARGET): $(OBJS) $(LIB_NAME)
	$(CXX) $(CXXFLAGS) -o $@ $^ 



# Rule to compile source files into object files
$(OBJDIR)/%.o: %.c $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET) $(LIB_OBJS) $(LIB_NAME)
	rmdir $(OBJDIR) 2>/dev/null || true
