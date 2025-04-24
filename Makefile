
NAME = test

SRC_DIR = sources/http

SRC = $(SRC_DIR)/main.cpp\
	$(SRC_DIR)/HTTPRequest.cpp\
	$(SRC_DIR)/HTTPResponse.cpp\
	$(SRC_DIR)/RequestHandler.cpp

OBJ_DIR = objects

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

COMP = c++

CFLAGS = -Wall -Wextra -Werror -Wshadow -std=c++20 -g

all: $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(COMP) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ_DIR) $(OBJ)
	@$(COMP) $(CFLAGS) $(OBJ) -o $(NAME)
	@echo "\n\033[0;32mAdministration is in work\033[0m\n"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@rm -f $(OBJ)
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)
	@echo "\n\033[0;31mAll is gone\033[0m\n"

re: fclean all

.PHONY: all clean re fclean
