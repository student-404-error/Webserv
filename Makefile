SRCS = main.cpp \
       src/http/HttpRequest.cpp \
       src/http/HttpResponse.cpp \
       src/parse/Config.cpp \
       src/parse/ConfigTokenizer.cpp \
       src/server/Connection.cpp \
       src/server/Server.cpp

OBJS = $(SRCS:.cpp=.o)
NAME = webserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes -Isrc/server

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
