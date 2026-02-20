SRCS = main.cpp \
       src/http/HttpRequest.cpp \
       src/http/HttpRequestValidator.cpp \
       src/http/HttpResponse.cpp \
       src/parse/Config.cpp \
       src/parse/ConfigTokenizer.cpp \
       src/parse/ConfigUtils.cpp \
       src/parse/ServerConfig.cpp \
       src/parse/LocationConfig.cpp \
       src/server/Connection.cpp \
       src/server/Server.cpp

OBJS = $(SRCS:.cpp=.o)
NAME = webserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: all clean fclean re
