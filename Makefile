CC		=	gcc
CFLAGS +=	-std=c99 -Wall  -g -D MAKE_VALGRIND_HAPPY
OPTLAGS =	#-03
LIBS	=	-pthread
INCLUDES =	-I.


OBJECTS	=	server.o \
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
	 $(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< $(LIBS)

server: server.o  $(OBJECTS) $(INCLUDE_FILES)
	 $(CC) $(CFLAGS) $(INCLUDES)  -o $@ $(OBJECTS) $(LIBS)

test1: test1.o  $(OBJECTS) $(INCLUDE_FILES)
	 $(CC) $(CFLAGS) $(INCLUDES)  -o $@ $(OBJECTS) $(LIBS)

all: $(TARGETS)

cleanall:
	rm $(TARGETS)
