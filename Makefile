imgwmon: main.c data.c conf.c
	     gcc -o imgwmon main.c data.c conf.c `pkg-config --cflags --libs glib-2.0` -lcurl -lyajl -std=gnu99
