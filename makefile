# Based partly on example in https://stackoverflow.com/questions/2394609/makefile-header-dependencies
#
# NOTE: This version need to make clean when switching between
# Windows and Linux, mainly for .d dependencies.

CC = gcc
LDFLAGS = -flto -lm  -Xlinker -Map=link.map

.PHONY : all clean foo

# sources except those that don't work in Windows
SRCDIR = ./src

SRC = $(wildcard $(SRCDIR)/*.c)

ifeq ($(OS),Windows_NT)
# Binaries (executable and object files)
BINDIR = ./w
#CFLAGS = -O3 -Wall -Wextra -std=c99
# Add for profiling: -pg -fprofile-arcs -ftest-coverage
#CFLAGS = -O3 -Wall -Wextra -std=c99   -DCOUNTSWAPS -pg -fprofile-arcs -ftest-coverage
CFLAGS = -O3 -Wall -Wextra -std=c99   -DCOUNTSWAPS
o = obj
SRCDIRNONWIN =
SRCNONWIN =
OBJNONWIN =
# Final binary
BIN = test_sorts.exe
else
############################### FOR LINUX ########
# Binaries (executable and object files)
BINDIR = ./x
#CFLAGS = -O3 -Wall -Wextra -std=gnu99
CFLAGS = -O3 -Wall -Wextra -std=gnu99 -DCOUNTSWAPS
o = o
# non-Windows sources (Linux/Unix etc)
SRCDIRNONWIN = ./src_nonwin
# List of non-Windows sources (Linux/Unix etc)
SRCNONWIN = $(wildcard $(SRCDIRNONWIN)/*.c)
OBJNONWIN = $(SRCNONWIN:$(SRCDIRNONWIN)/%.c=$(BINDIR)/%.$(o))
# Final binary
BIN = test_sorts
endif

# Default target.
all : $(BIN)

# All .o/.obj files go to build dir.
OBJ = $(SRC:$(SRCDIR)/%.c=$(BINDIR)/%.$(o)) $(OBJNONWIN)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.$(o)=%.d)

# Target named after the binary.
$(BIN) : $(BINDIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
# Create build directories - same structure as sources.
# Just link all the object files.
$(BINDIR)/$(BIN) : $(OBJ)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Include all .d files
-include $(DEP)

# Build object files.
# I can't see how to merge these two rules.
$(BINDIR)/%.$(o) : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BINDIR)/%.$(o) : $(SRCDIRNONWIN)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean :
	-rm $(BINDIR)/$(BIN)
	-rm $(OBJ) $(DEP)

foo:
	@echo cpp: $(SRC)
	@echo obj: $(OBJ)
	@echo dep: $(DEP)
	@echo srcnonwin: $(SRCNONWIN)
	@echo objnonwin: $(OBJNONWIN)

#######################
