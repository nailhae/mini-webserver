ifeq "$(findstring debug, $(MAKECMDGOALS))" "debug"
	DFLAGS = -g3 -fsanitize=address
else
	ARG.DEBUG = 0
	DFLAGS =
endif

CXX 		=	c++

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 $(INCS)

NAME 		= ./a.out

SRCS_DIR := $(shell dirname $(PWD))

INCS = \
	-I$(SRCS_DIR)/util \
	-I$(SRCS_DIR)/parser \

SRCS = \
	$(SRCS_DIR)/parser/HttpBlock.cpp \
	$(SRCS_DIR)/parser/LocationBlock.cpp \
	$(SRCS_DIR)/parser/Parser.cpp \
	$(SRCS_DIR)/parser/ServerBlock.cpp \
	$(SRCS_DIR)/parser/main.cpp \
	$(SRCS_DIR)/util/Colors.cpp \
	$(SRCS_DIR)/util/MultiTree.cpp \
	$(SRCS_DIR)/util/MultiTreeNode.cpp \

OBJS		=	$(SRCS:%.cpp=%.o)

RM			=	rm -f

all:	$(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%o:	%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: 
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re