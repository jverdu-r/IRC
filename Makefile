CFLAGS = -std=c++98 -Wall -Wextra -Werror
TARGET = irc_server

SRC_DIR = src
INC_DIR = includes
OBJ_DIR = obj

SOURCES = $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/socket_manager.cpp \
            $(SRC_DIR)/command_handler.cpp \
            $(SRC_DIR)/user_manager.cpp \
            $(SRC_DIR)/event_handler.cpp
OBJECTS = $(OBJ_DIR)/main.o \
            $(OBJ_DIR)/socket_manager.o \
            $(OBJ_DIR)/command_handler.o \
            $(OBJ_DIR)/user_manager.o \
            $(OBJ_DIR)/event_handler.o

all: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) -lstdc++ # Modificado

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(TARGET)

re: fclean all

.PHONY: all clean fclean re