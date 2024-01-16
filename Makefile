
PROGRAM=main
TESTER=as1_testbench

CFLAGS=-O2 -g -Wall -pedantic


all: ${TESTER} ${PROGRAM}

${TESTER}: ${TESTER}.c
	${CC} ${CFLAGS} -o ${TESTER} ${TESTER}.c

${PROGRAM}: ${PROGRAM}.c lahetys.c vastaanotto.c morse.c
	${CC} ${CFLAGS} -o ${PROGRAM} ${PROGRAM}.c lahetys.c vastaanotto.c morse.c

.PHONY: test
test: all
	./${TESTER} ${PROGRAM}

.PHONY: clean
clean:
	rm -rf *.o *~ ${TESTER} ${PROGRAM}
