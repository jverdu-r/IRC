CC = c++
CFLAGS		=	-std=c++98 -Wall -Wextra -Werror
TARGET		=	irc_server

SRC_DIR		=	src
BONUS_DIR	=	src/bonus
INC_DIR		=	includes
OBJ_DIR		=	obj
BONUS_OBJ	=	$(BONUS_SRC:$(BONUS_DIR)/%.cpp=$(OBJ_DIR)/bonus/%.o)




SOURCES		=	$(SRC_DIR)/main.cpp \
        		$(SRC_DIR)/socket_manager.cpp \
        		$(SRC_DIR)/command_handler.cpp \
        		$(SRC_DIR)/user_manager.cpp \
        		$(SRC_DIR)/event_handler.cpp \
				$(SRC_DIR)/utils.cpp

BONUS_SRC	=	$(BONUS_DIR)/bot_bonus.cpp

OBJ			=	$(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
BONUS_OBJ 	=	$(BONUS_SRC:$(BONUS_DIR)/%.cpp=$(OBJ_DIR)/bonus/%.o)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/bonus/%.o: $(BONUS_DIR)/%.cpp
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@


all: $(OBJ_DIR) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) -lstdc++

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

bonus: CFLAGS += -DBONUS_MODE
bonus: $(OBJ_DIR) $(OBJ_DIR)/bonus $(OBJ) $(BONUS_OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(BONUS_OBJ) -o irc_server

$(OBJ_DIR)/bonus:
	mkdir -p $(OBJ_DIR)/bonus

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(TARGET)

re: fclean all

.PHONY: all clean fclean re