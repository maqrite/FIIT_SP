BUILD_DIR = build
TARGET = my_tree_app

.PHONY: all config build copy_commands run clean rebuild

all: config build copy_commands

config:
	cmake -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

copy_commands:
	cp $(BUILD_DIR)/compile_commands.json .

run: all
	./$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f compile_commands.json

rebuild: clean all
