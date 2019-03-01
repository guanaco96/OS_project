CC		=	gcc
CFLAGS +=	-std=c99 -Wall  -g -D MAKE_VALGRIND_HAPPY -DDEBUG
OPTLAGS =	#-03
LIBS	=	-pthread
INCLUDES =	-I.

TARGETS =	chatty \
			client

OBJECTS	=	chatty.o \
			queue.o \
			icl_hash.o \
			nickname.o \
			message.o \
			hashtable.o \
			connections.o \
			stat.o \
			worker.o 


INLUCDE_FILES =	ops.h \
				config.h \
				queue.h \
				icl_hash.h \
				nickname.h \
				message.h \
				hashtable.h \
				connections.h \
				stat.h \
				parser.h


.PHONY: cleanall all

%.o: %.cm $(INDCLUDE_FILES)
	 $(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

client: client.c connections.c message.c
	gcc -g -Wall -o client client.c connections.c message.c

chatty: chatty.o  $(OBJECTS) $(INCLUDE_FILES)
	 $(CC) $(CFLAGS) $(INCLUDES)  -o $@ $(OBJECTS) $(LIBS)

test1: test1.o  $(OBJECTS) $(INCLUDE_FILES)
	 $(CC) $(CFLAGS) $(INCLUDES)  -o $@ $(OBJECTS) $(LIBS)

test_stat: test_stat.o stat.o stat.h 
	$(CC) $(CFLAGS) $(INCLUDES)  -o $@ test_stat.o stat.o $(LIBS)
	
all: $(TARGETS)

cleanall:
	rm $(OBJECTS)
