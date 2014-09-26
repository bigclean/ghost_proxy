### Makefile ---
##
### Code:

SHELL := /bin/sh

CC     = gcc
RM     = rm
HIGH  := \033[7m
LIGHT := \033[0m

all:
	@echo "------------------------------------------------------------"
	@echo -e "\t$(HIGH) Building... $(LIGHT)"
	$(CC) ghost.c common.c -o ghost
	@echo "------------------------------------------------------------"

clean:
	@echo "------------------------------------------------------------"
	@echo -e "\t$(HIGH) CLeaning... $(LIGHT)"
	$(RM) ghost
	@echo "------------------------------------------------------------"

help:
	@echo "------------------------------------------------------------"
	@echo -e "\t$(HIGH) Available targets: $(LIGHT)"
	@echo -e "available make targets"
	@echo "make help : Display this help message"
	@echo "make all : Compile the project"
	@echo "------------------------------------------------------------"

### Makefile ends here
