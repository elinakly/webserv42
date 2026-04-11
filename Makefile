NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -g
INCLUDES := -Isrcs -Iconfig/include

SRCS = 	srcs/main.cpp \
		config/ConfigParser.cpp \
		config/ServerNode.cpp \
		config/ParseLocationBlock.cpp \
		config/ErrorPageNode.cpp \
		config/ParseServerBlock.cpp 

OBJS_DIR    = obj
OBJS        = $(SRCS:%.cpp=$(OBJS_DIR)/%.o)


all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re