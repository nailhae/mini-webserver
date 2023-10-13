CXX 		=	c++

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address $(INCS)

NAME 		= ./webServ

SRCS_DIR	= $(PWD)/srcs

INCS = \
	-I$(SRCS_DIR)/util \
	-I$(SRCS_DIR)/networking \
	-I$(SRCS_DIR)/AMethod \

SRCS = \
	$(SRCS_DIR)/AMethod/AMethod.cpp \
	$(SRCS_DIR)/AMethod/MethodGet.cpp \
	$(SRCS_DIR)/AMethod/MethodHead.cpp \
	$(SRCS_DIR)/AMethod/MethodDelete.cpp \
	$(SRCS_DIR)/AMethod/MethodPost.cpp \
	$(SRCS_DIR)/networking/WebServer.cpp \
	$(SRCS_DIR)/networking/webServerParseFileOrNull.cpp \
	$(SRCS_DIR)/networking/ChangeList.cpp \
	$(SRCS_DIR)/networking/UserData.cpp \
	$(SRCS_DIR)/networking/httpRequestParsing.cpp \
	$(SRCS_DIR)/util/Colors.cpp \
	$(SRCS_DIR)/util/Error.cpp \
	$(SRCS_DIR)/util/MultiTree.cpp \
	$(SRCS_DIR)/util/MultiTreeNode.cpp \
	$(SRCS_DIR)/networking/main.cpp \
	$(SRCS_DIR)/networking/waitForClientConnection.cpp \
	$(SRCS_DIR)/networking/initServer.cpp \

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

debug: $(NAME)

.PHONY: all clean fclean re
