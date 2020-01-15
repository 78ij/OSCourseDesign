all:
	gcc `pkg-config --cflags gtk+-3.0` -o first first.c `pkg-config --libs gtk+-3.0` -lpthread&&gcc `pkg-config --cflags gtk+-3.0` -o third third.c `pkg-config --libs gtk+-3.0` -lpthread&&gcc `pkg-config --cflags gtk+-3.0` -o second second.c `pkg-config --libs gtk+-3.0` -lpthread&&gcc bootstrap.c -o bootstrap
clean:
	rm GUI