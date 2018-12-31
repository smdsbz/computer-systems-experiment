GREEN		= \033[0;32m
NC		= \033[0m
LOG_PREFIX	= $(GREEN)💬$(NC)

CC		= gcc
SOURCE		= main.c
OUTFILE		= main.exe
RUN_COMMAND	= ./$(OUTFILE) ../

main: $(OUTFILE)
	@echo "$(LOG_PREFIX) Start running: $(RUN_COMMAND)"
	@$(RUN_COMMAND)
	@echo "$(LOG_PREFIX) All done!"

$(OUTFILE): $(SOURCE)
	@echo "$(LOG_PREFIX) Source outdated! Re-building..."
	@$(CC) $(SOURCE) -o $(OUTFILE)

