##########################################################################
# aqueue
#########################################################################

IDIR = -I. -I../include

# 기본 컴파일 옵션 설정
CC		= gcc
RM		= /bin/rm
AR		= /usr/bin/ar -cr
RANLIB		= /usr/bin/ranlib
CFLAGS		= -g -Wall $(DEFINES) $(IDIR)
LDFLAGS		= $(LDIR) $(LIBS)
MD		= /usr/X11R6/bin/makedepend

#-------------------------------------------------------------------------------- 이하 수정 필요
# 현재 디렉토리의 소스에 대한 지정
SRCS		= aqueue.c 
OBJS		= aqueue.o 
CPPSRCS		= 
CPPOBJS		= 
HFILES		= aqueue.h 

TARGET_LDIR	= ../lib
TARGET_IDIR	= ../include
TARGET_INC	= aqueue.h 
LIB_AQUEUE	= libaqueue.a

LIB		= $(LIB_AQUEUE)
EXE		=

all: install tags

install: $(LIB)
	mv $(LIB) $(TARGET_LDIR)
	cp $(TARGET_INC) $(TARGET_IDIR)

$(LIB_AQUEUE): $(OBJS)
	$(AR) $(LIB_AQUEUE) $(OBJS)
	$(RANLIB) $(LIB_AQUEUE)

.SUFFIXES: .o .c .cpp
.c.o : $(SRCS)
	$(CC) -c $(CFLAGS) $<

.cpp.o : $(SRCSCPP)
	g++ -c $(CFLAGS) $< 


clean:
	rm -f *.o core tags $(EXE)

depend:
	$(MD) -- $(IDIR) -- $(SRCS)

tags : $(SRCS) $(CPPSRCS) $(HFILES)
	ctags $^

test:
	echo $(CFLAGS)

# DO NOT DELETE
