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
TOTAL_FILES	= $(words $(SRCS))

# Progress tracking
COUNTER_FILE	= .compile_counter
CURRENT_FILE	= 0

# ─── Test config ──────────────────────────────────────────────────────
TEST_NAME	= test_runner
TEST_DIR	= tests
TEST_INCDIR	= -Itests/include -Iinc -Iinterfaces
TEST_OBJS_DIR = objs/tests

TEST_SRCS	= $(shell find $(TEST_DIR) -name '*.cpp' -type f)
TEST_OBJS	= $(patsubst $(TEST_DIR)/%.cpp,$(TEST_OBJS_DIR)/%.o,$(TEST_SRCS))
MAIN_OBJS	= $(filter-out $(OBJS_DIR)/main.o,$(OBJS))
# ──────────────────────────────────────────────────────────────────────


all: setup $(NAME)

setup:
	@echo 0 > $(COUNTER_FILE)

$(NAME): $(OBJS)
	@rm -f $(COUNTER_FILE)
	@printf '\n'
	$(CPP) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@printf '\033[32m  Executable %s created.\033[0m\n' "$(NAME)"

$(OBJS_DIR)/%.o: $(SRCDIR)/%.cpp | $(OBJS_DIR)
	@mkdir -p $(dir $@)
	@CURRENT=$$(cat $(COUNTER_FILE) 2>/dev/null || echo 0); \
	CURRENT=$$((CURRENT + 1)); \
	echo $$CURRENT > $(COUNTER_FILE); \
	PERCENT=$$((CURRENT * 100 / $(TOTAL_FILES))); \
	BAR_SIZE=30; \
	FILLED=$$((PERCENT * BAR_SIZE / 100)); \
	EMPTY=$$((BAR_SIZE - FILLED)); \
	BAR=""; \
	for i in $$(seq 1 $$FILLED); do BAR="$$BAR█"; done; \
	for i in $$(seq 1 $$EMPTY); do BAR="$$BAR░"; done; \
	FILENAME=$$(basename $<); \
	printf '\r\033[36m[%3d/%3d]\033[0m \033[33m%-40s\033[0m \033[36m%s\033[0m \033[32m%3d%%\033[0m' \
		"$$CURRENT" "$(TOTAL_FILES)" "$$FILENAME" "$$BAR" "$$PERCENT"
	@$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

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
	@mkdir -p $(dir $@)
	@CURRENT=$$(cat $(COUNTER_FILE) 2>/dev/null || echo 0); \
	CURRENT=$$((CURRENT + 1)); \
	echo $$CURRENT > $(COUNTER_FILE); \
	TOTAL=$$(($(TOTAL_FILES) + $(words $(TEST_SRCS)))); \
	PERCENT=$$((CURRENT * 100 / TOTAL)); \
	BAR_SIZE=30; \
	FILLED=$$((PERCENT * BAR_SIZE / 100)); \
	EMPTY=$$((BAR_SIZE - FILLED)); \
	BAR=""; \
	for i in $$(seq 1 $$FILLED); do BAR="$$BAR█"; done; \
	for i in $$(seq 1 $$EMPTY); do BAR="$$BAR░"; done; \
	FILENAME=$$(basename $<); \
	printf '\r\033[36m[%3d/%3d]\033[0m \033[33m%-40s\033[0m \033[36m%s\033[0m \033[32m%3d%%\033[0m' \
		"$$CURRENT" "$$TOTAL" "$$FILENAME" "$$BAR" "$$PERCENT"
	@$(CPP) $(CXXFLAGS) $(TEST_INCDIR) -c $< -o $@

$(TEST_OBJS_DIR):
	mkdir -p $(TEST_OBJS_DIR)

build-test: CXXFLAGS += -g
build-test: $(OBJS) $(TEST_OBJS)
	@rm -f $(COUNTER_FILE)
	@printf '\n'
	$(CPP) $(CXXFLAGS) $(MAIN_OBJS) $(TEST_OBJS) -o $(TEST_NAME)
	@printf '\033[32mTest executable %s created.\033[0m\n' "$(TEST_NAME)"

test: build-test
	@printf '\033[34mRunning tests...\033[0m\n'
	./$(TEST_NAME)

party:
	@curl parrot.live

.PHONY: all clean fclean re party build-test test