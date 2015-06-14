### Makefile ---
##
### Code:

SHELL := /bin/sh

CC     = gcc
RM     = rm
HIGH  := \033[7m
LIGHT := \033[0m

.PHONY: all server client

all: server client

server:
	@echo "------------------------------------------------------------"
	@echo -e "\t$(HIGH) Server Building... $(LIGHT)"
	$(CC) server/ghosted.c server/common.c -o ghosted
	@echo "------------------------------------------------------------"

client:
	@echo "------------------------------------------------------------"
	@echo -e "\t$(HIGH) Client Building... $(LIGHT)"
	$(CC) client/ghost.c client/common.c -o ghost
	@echo "------------------------------------------------------------"

clean:
	@echo "------------------------------------------------------------"
	@echo -e "\t$(HIGH) CLeaning... $(LIGHT)"
	$(RM) ghost
	$(RM) ghosted
	@echo "------------------------------------------------------------"

help:
	@echo "------------------------------------------------------------"
	@echo -e "\t$(HIGH) Available targets: $(LIGHT)"
	@echo -e "available make targets"
	@echo "make help : Display this help message"
	@echo "make all : Compile the project"
	@echo "------------------------------------------------------------"

### Makefile ends here
