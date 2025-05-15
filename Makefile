COMPILER = c++

TARGET = webserver
INC_DIR = includes
SRC = srcs/main.cpp\
	srcs/logger/Logger.cpp\
	srcs/configparser/Parser.cpp \
	srcs/HTTP/HTTPRequest.cpp\
	srcs/HTTP/HTTPResponse.cpp\
	srcs/HTTP/RequestHandler.cpp \
	srcs/epoll/Client.cpp \
	srcs/epoll/eventLoop.cpp \
	srcs/epoll/timeout.cpp
OBJ_DIR = objs
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEP = $(OBJ:.o=.d)
#-MMD flag makes depency file .d for every .cpp file
#-MP flag creates phony for every header file so if header file is deleted
#the making process will not throw an error missing file so it allows deleting and creating new header files
CFLAGS = -g -Wall -Wextra -Werror -std=c++20 -I$(INC_DIR) -MMD -MP

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(COMPILER) $(CFLAGS) -o $(TARGET) $(OBJ)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)/$(dir $<)
	@$(COMPILER) $(CFLAGS) -c $< -o $@

#This takes into account the .d depency files
-include $(DEP)

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -rf $(OBJ_DIR)
	@rm -f $(TARGET)

re: fclean all

.Phony: all clean fclean re