MAKEFLAGS	= -s

CPP			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -Iinc -Iinterfaces

NAME		= webserv

SRCDIR		= src
INCDIR		= inc
INTERFACES  = interfaces
OBJS_DIR	= objs

SRCS		= $(shell find $(SRCDIR) -name '*.cpp' -type f)
OBJS		= $(patsubst $(SRCDIR)/%.cpp,$(OBJS_DIR)/%.o,$(SRCS))

# ─── Test config ──────────────────────────────────────────────────────
TEST_NAME	= test_runner
TEST_DIR	= tests
TEST_INCDIR	= -Itests/include -Iinc -Iinterfaces
TEST_OBJS_DIR = objs/tests

TEST_SRCS	= $(shell find $(TEST_DIR) -name '*.cpp' -type f)
TEST_OBJS	= $(patsubst $(TEST_DIR)/%.cpp,$(TEST_OBJS_DIR)/%.o,$(TEST_SRCS))
MAIN_OBJS	= $(filter-out $(OBJS_DIR)/main.o,$(OBJS))
# ──────────────────────────────────────────────────────────────────────


all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@printf '\033[32mExecutable %s created.\033[0m\n' "$(NAME)"

$(OBJS_DIR)/%.o: $(SRCDIR)/%.cpp | $(OBJS_DIR)
	mkdir -p $(dir $@)
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

clean:
	rm -rf $(OBJS_DIR)
	@printf '\033[31mRemoved %s\033[0m\n' "$(OBJS_DIR)"

fclean: clean
	rm -f $(NAME)
	rm -f $(TEST_NAME)
	@printf '\033[31mRemoved %s\033[0m\n' "$(NAME)"

re: fclean all


$(TEST_OBJS_DIR)/%.o: $(TEST_DIR)/%.cpp | $(TEST_OBJS_DIR)
	mkdir -p $(dir $@)
	$(CPP) $(CXXFLAGS) $(TEST_INCDIR) -c $< -o $@

$(TEST_OBJS_DIR):
	mkdir -p $(TEST_OBJS_DIR)

build-test: $(OBJS) $(TEST_OBJS)
	$(CPP) $(CXXFLAGS) $(MAIN_OBJS) $(TEST_OBJS) -o $(TEST_NAME)
	@printf '\033[32mTest executable %s created.\033[0m\n' "$(TEST_NAME)"

test: build-test
	@printf '\033[34mRunning tests...\033[0m\n'
	./$(TEST_NAME)
	rm -fr $(TEST_NAME)
	rm -fr $(TEST_OBJS_DIR)

party:
	@curl parrot.live

.PHONY: all clean fclean re party build-test