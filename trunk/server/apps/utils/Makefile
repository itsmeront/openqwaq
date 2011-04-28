CFLAGS = -Wall
CC = gcc
ifndef WRAPPER_NAME
$(error WRAPPER_NAME not set)
endif
ifndef INSTALL_DIR
$(error INSTALL_DIR not set)
endif

#This is needed by the C wrapper program
FULL_SCRIPT_PATH = "\"${INSTALL_DIR}/$(WRAPPER_NAME).sh\""

all: $(WRAPPER_NAME)

$(WRAPPER_NAME):
	${CC} ${CFLAGS} -D FULL_PATH=$(FULL_SCRIPT_PATH) -o $(INSTALL_DIR)/$@ setuid-prog.c
