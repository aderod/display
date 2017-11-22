leddisplay: leddisplay.o
	$(CC) $(LDFLAGS) leddisplay.o timer.o render.o gpio.o -o leddisplay

leddisplay.o: leddisplay.c
	$(CC) $(CFLAGS) -c gpio.c
	$(CC) $(CFLAGS) -c render.c
	$(CC) $(CFLAGS) -c timer.c
	$(CC) $(CFLAGS) -c leddisplay.c

clean:
	rm *.o leddisplay
