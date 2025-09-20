DEFAULT_PRESET ?= default
BUILD_DIR ?= build

# Colors for output
GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
NC := \033[0m # No Color

.PHONY: help configure build clean list-presets debug release test install

# Default target
all: configure build

help:
	@echo "$(GREEN)Available targets:$(NC)"
	@echo "  help         - Show this help message"
	@echo "  list-presets - List all available CMake presets"
	@echo "  configure    - Configure using default preset ($(DEFAULT_PRESET))"
	@echo "  build        - Build using default preset"
	@echo "  debug        - Configure and build debug version"
	@echo "  release      - Configure and build release version"
	@echo "  clean        - Clean build directory"
	@echo "  test         - Run tests"
	@echo "  install      - Install the project"
	@echo ""
	@echo "$(YELLOW)Environment variables:$(NC)"
	@echo "  DEFAULT_PRESET - Set default preset (current: $(DEFAULT_PRESET))"
	@echo "  BUILD_DIR      - Set build directory (current: $(BUILD_DIR))"

list-presets:
	@echo "$(GREEN)Available CMake presets:$(NC)"
	@cmake --list-presets=all 2>/dev/null || echo "No presets found or CMake version < 3.20"

configure:
	@echo "$(GREEN)Configuring with preset: $(DEFAULT_PRESET)$(NC)"
	@cmake --preset $(DEFAULT_PRESET)

build: configure
	@echo "$(GREEN)Building with preset: $(DEFAULT_PRESET)$(NC)"
	@cmake --build --preset $(DEFAULT_PRESET)

debug:
	@echo "$(GREEN)Configuring and building debug version$(NC)"
	@cmake --preset debug
	@cmake --build --preset debug

release:
	@echo "$(GREEN)Configuring and building release version$(NC)"
	@cmake --preset release-make
	@cmake --build --preset release-make

clean:
	@echo "$(YELLOW)Cleaning build directory$(NC)"
	@rm -rf $(BUILD_DIR)
