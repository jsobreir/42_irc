CC = c++
CFLAGS = -Wall -Wextra -Werror -g -std=c++98
ROOT = $(shell pwd)
INC = -I$(ROOT)/includes
VPATH = src includes
RM = rm -rf

NAME = ircserv 

NAME = IRC 
SRC =	Channel.cpp \
		Client.cpp \
		Commands.cpp \
		Operator.cpp \
		Server.cpp \
		Utils.cpp \
		main.cpp

OBJ_DIR = obj
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(OBJ_DIR):
	mkdir -p obj

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)

$(NAME): $(OBJ_DIR) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME) $(OBJ_DIR)

re: fclean all
