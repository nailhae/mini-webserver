ifeq "$(findstring debug, $(MAKECMDGOALS))" "debug"
	DFLAGS = -g -fsanitize=address
else
	ARG.DEBUG = 0
	DFLAGS =
endif

NAME 		=	./TreeTester

SRCS = $(wildcard *.cpp ../util/*.cpp)
OBJS = ${SRCS:.cpp=.o}

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
RM = rm -f

all:	${NAME}

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(DFLAGS) -o $@ $^

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
		${RM} ${OBJS}

fclean:	clean
		${RM} ${NAME}

re:		fclean all

debug: $(NAME)

.PHONY: all clean fclean re debug