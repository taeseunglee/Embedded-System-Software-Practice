CC := arm-none-linux-gnueabi-gcc

app: app.c
	$(CC) --static -c app.c
	$(CC) --static app.o -o app

clean : 
	rm -rf app.o app
