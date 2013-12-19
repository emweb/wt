CC = g++
CFLAGS = -g
LDFLAGS = 

PROG1 = generateHorizontalFlapData
PROG2 = generateVerticalFlapData
HDRS = flapConstants.h v3math.h CompositeTexture.h
SRCS1 = horizontalFlapData.C CompositeTexture.C
SRCS2 = verticalFlapData.C CompositeTexture.C
OBJS1 = horizontalFlapData.o CompositeTexture.o
OBJS2 = verticalFlapData.o CompositeTexture.o

all: $(PROG1) $(PROG2)

$(PROG1) : $(OBJS1)
	$(CC) -o $(PROG1) $(LDFLAGS) $(OBJS1)

$(PROG2) : $(OBJS2)
	$(CC) -o $(PROG2) $(LDFLAGS) $(OBJS2)

clean:
	rm -f core $(PROG1) $(PROG2) $(OBJS1) $(OBJS2)


horizontalFlapData.o: flapConstants.h v3math.h CompositeTexture.h
verticalFlapData.o: flapConstants.h v3math.h CompositeTexture.h
CompositeTexture.o: CompositeTexture.h


