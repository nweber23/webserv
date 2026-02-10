MAKEFLAGS	= -s

CPP			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -Iinc -Iinterfaces

NAME		= webserv

SRCDIR		= src
INCDIR		= inc
INTERFACES  = interfaces
OBJS_DIR	= objs

SRCS		= $(wildcard $(SRCDIR)/*.cpp)
OBJS		= $(patsubst $(SRCDIR)/%.cpp,$(OBJS_DIR)/%.o,$(SRCS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@printf '\033[32mExecutable %s created.\033[0m\n' "$(NAME)"

$(OBJS_DIR)/%.o: $(SRCDIR)/%.cpp | $(OBJS_DIR)
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

clean:
	rm -rf $(OBJS_DIR)
	@printf '\033[31mRemoved %s\033[0m\n' "$(OBJS_DIR)"

fclean: clean
	rm -f $(NAME)
	@printf '\033[31mRemoved %s\033[0m\n' "$(NAME)"

re: fclean all

party:
	@curl parrot.live

.PHONY: all clean fclean re party