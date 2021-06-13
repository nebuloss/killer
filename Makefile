# Recursive Wildcard function
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d))

# Remove duplicate function
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1))) 


# Compile / Link Flags
CFLAGS = -c -Wall -std=c99 -O3


#CC= x86_64-w64-mingw32-gcc
CC = gcc

ifeq ($(OS),Windows_NT)
	LDFLAGS =-mwindows
else
	LDFLAGS = 
endif

DLL_FOLDER=dll

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

# Number of therads available 
CPUS = $(nproc)

multi:
	@$(MAKE) -j$(CPUS) --no-print-directory all

all: $(OBJ_DIR) $(OUT)
	@$(MAKE) run

$(OBJ_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $< $(INCLUDES) $(LIBS) -o $@

$(OUT): $(OBJ)
	@echo "Linking $@"
	@$(CC) -o $(OUT) $^ $(LDFLAGS)

clean:
	@echo "Cleaning Build"
	@rm -rf $(BUILD_DIR) $(OUT) $(OUT).exe

rebuild: clean
	@$(MAKE) -j$(CPUS) --no-print-directory alls

run:
	@strip $(OUT).exe
	./$(OUT)