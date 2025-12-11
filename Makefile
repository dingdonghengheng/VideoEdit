CFLAGS = `pkg-config --cflags opencv`
LIBS   = `pkg-config --libs opencv` -lpthread
OBJS   = main.o thread.o

CC = gcc

video_app: $(OBJS)
	$(CC) -o video_app $(OBJS) $(LIBS)

%.o: %.c thread.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o video_app

.PHONY: clean
