NAME 		=	./TreeTester

CXX 		=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98

INCS		= \
./srcs/util/ \
./srcs/parser/ \

SRCS		= \
./srcs/util/Colors.cpp \
./srcs/util/MultiTree.cpp \
./srcs/util/MultiTreeNode.cpp \
\
./srcs/parser/HttpBlock.cpp \
./srcs/parser/LocationBlock.cpp \
./srcs/parser/main.cpp \
./srcs/parser/parser.cpp \
./srcs/parser/ServerBlock.cpp \

OBJS		=	$(SRCS:%.cpp=%.o)

RM			=	rm -f

all:	$(NAME)

$(NAME): $(OBJS) $(INCS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%o:	%.cpp $(INCS)
	$(CXX) $(CXXFLAGS) $(INCS) -c $^ -o $@

clean: 
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re