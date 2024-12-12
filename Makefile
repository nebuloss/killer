# Recursive Wildcard function
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d))

# Remove duplicate function
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1))) 

# Compile / Link Flags
CFLAGS = -c -Wall -std=c99 -O3

CC = gcc

LDFLAGS =-mwindows

# Main target and filename of the executable
OUT = killer

# Build Directory
BUILD_DIR = build

# List of all the .c source files to compile
SRC = $(call rwildcard,,*.c)

# List of all the .o object files to produce
OBJ = $(patsubst %,$(BUILD_DIR)/%,$(SRC:%.c=%.o))
OBJ_DIR = $(call uniq, $(dir $(OBJ)))

# List of all includes directory
INCLUDES = $(patsubst %, -I %, $(call uniq, $(dir $(call rwildcard,,*.h))))
LIBS =

all: $(OBJ_DIR) $(OUT)
	@$(MAKE) run

$(OBJ_DIR):
	@echo Creating directory $@
	@if not exist "$(OBJ_DIR)" md "$(OBJ_DIR)"

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $< $(INCLUDES) $(LIBS) -o $@

$(OUT): $(OBJ)
	@echo "Linking $@"
	@$(CC) -o $(OUT) $^ $(LDFLAGS)

clean:
	@echo Cleaning Build
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist $(OUT).exe del /q $(OUT).exe

rebuild: clean
	@$(MAKE) all

run:
	@strip $(OUT).exe
	./$(OUT).exe