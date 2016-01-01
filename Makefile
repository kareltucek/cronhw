FLAGS+=-ggdb -std=c99 -Wall

all : mycrond

common.o : src/common.c src/common.h
	${CC} src/common.c -c -O ${FLAGS} ${LDFLAGS}

mycrond : src/main.c executor.o loader.o common.o stolen.o
	${CC} src/main.c stolen.o executor.o loader.o common.o -o mycrond ${FLAGS}  ${LDFLAGS}

executor.o : src/executor.c src/executor.h Makefile
	${CC} src/executor.c -c -O ${FLAGS}  ${LDFLAGS}

loader.o : src/loader.h src/loader.c src/parse_ctx.h src/task.h Makefile
	${CC} src/loader.c -c -O ${FLAGS}  ${LDFLAGS}

stolen.o : third-party-src/stolen.h third-party-src/stolen.c Makefile
	${CC} third-party-src/stolen.c -c -O ${FLAGS}  ${LDFLAGS}

clean :
	rm *.o mycrond test*/*.out
