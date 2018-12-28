CC	= gcc
SOURCE	= main.c
OUTFILE	= main.exe

main: $(OUTFILE)
	@./$(OUTFILE)

$(OUTFILE): $(SOURCE)
	@echo "Source outdated, re-building..."
	@$(CC) $(SOURCE) -o $(OUTFILE)
