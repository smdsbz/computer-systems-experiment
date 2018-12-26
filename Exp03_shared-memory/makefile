CC		= gcc
GREEN		= \033[0;32m
NC		= \033[0m
LOG_PREFIX	= $(GREEN)[buildsys]$(NC)
SOURCE		= main.c semaphore_utils.c
OUTFILE		= main.exe
TESTFILE	= ./testinput

main: $(OUTFILE)
	@echo "$(LOG_PREFIX) Start running. Showing command:"
	time -f "Total time elapsed: %e, CPU percentage: %P, Context switched: %w" ./$(OUTFILE) $(TESTFILE)
	@echo "$(LOG_PREFIX) Finished running!"
	@echo "$(LOG_PREFIX) Showing diff..."
	@diff $(TESTFILE) out

$(OUTFILE): $(SOURCE)
	@echo "$(LOG_PREFIX) Source outdated, re-building..."
	@$(CC) $(SOURCE) -o $(OUTFILE)
	@echo "$(LOG_PREFIX) Re-build complete without error!"
