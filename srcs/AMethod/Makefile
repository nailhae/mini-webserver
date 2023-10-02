NAME 		=	./a.out

CXX 		=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98

INCS		= \
./AMethod.hpp \

SRCS		= \
./AMethod.cpp \
./main.cpp \

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