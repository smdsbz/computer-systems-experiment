CC	= gcc
OUTFILE	= main.exe
SOURCE	= main.c
SELL_SOURCE	= sell.c
EXTRA	= -lpthread

main:
	@$(CC) $(SOURCE) $(EXTRA) -o $(OUTFILE)
	@./$(OUTFILE)

sell:
	@$(CC) $(SELL_SOURCE) $(EXTRA) -o $(OUTFILE)
	@./$(OUTFILE)

