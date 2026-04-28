CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lsqlite3
TARGET = schedule_system

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET_PATH = $(BIN_DIR)/$(TARGET)$(EXE_EXT)

ifdef OS
    EXE_EXT = .exe
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = mkdir
else
    EXE_EXT = 
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
endif

.PHONY: all clean dirs

all: dirs $(TARGET_PATH)

dirs:
	@$(MKDIR) $(BUILD_DIR) 2>nul || echo "" >nul
	@$(MKDIR) $(BIN_DIR) 2>nul || echo "" >nul

$(TARGET_PATH): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RMDIR) $(BUILD_DIR) 2>nul || echo "" >nul
	@$(RMDIR) $(BIN_DIR) 2>nul || echo "" >nul

run: all
	@cd $(BIN_DIR) && $(TARGET)$(EXE_EXT)
