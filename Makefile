ifeq "$(findstring debug, $(MAKECMDGOALS))" "debug"
	DFLAGS = -g3 -fsanitize=address
else
	ARG.DEBUG = 0
	DFLAGS =
endif

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 $(INCS)

NAME = ./a.out

SRCS_DIR = $(PWD)/srcs

INCS = \
	-I$(SRCS_DIR)/util \
	-I$(SRCS_DIR)/networking \
	-I$(SRCS_DIR)/AMethod \

SRCS = \
	$(SRCS_DIR)/networking/main.cpp \
	\
	$(SRCS_DIR)/AMethod/AMethod.cpp \
	$(SRCS_DIR)/networking/ChangeList.cpp \
	$(SRCS_DIR)/networking/UserData.cpp \
	$(SRCS_DIR)/networking/WebServer.cpp \
	$(SRCS_DIR)/networking/httpRequestParsing.cpp \
	$(SRCS_DIR)/networking/initServer.cpp \
	$(SRCS_DIR)/networking/waitForClientConnection.cpp \
	$(SRCS_DIR)/networking/webServerParseFileOrNull.cpp \
	$(SRCS_DIR)/util/Colors.cpp \
	$(SRCS_DIR)/util/Error.cpp \
	$(SRCS_DIR)/util/MultiTree.cpp \
	$(SRCS_DIR)/util/MultiTreeNode.cpp \

OBJS = $(SRCS:.cpp=.o)

RM = rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(DFLAGS) $(OBJS) -o $@ 

%o:	%.cpp
	$(CXX) $(CXXFLAGS) $(DFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean:
	make clean
	$(RM) $(NAME)

re:
	make fclean
	make all

debug: $(NAME)

.PHONY: all clean fclean re