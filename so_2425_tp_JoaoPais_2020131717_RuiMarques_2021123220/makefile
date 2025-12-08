all: feed manager

feed: feed.c utils.h
	gcc feed.c -o feed

manager: manager.c operacoesmemoria.c utils.c threads.c utils.h
	gcc manager.c operacoesmemoria.c utils.c threads.c -pthread -o manager

clean:
	rm -f feed manager

clean-feed:
	rm -f feed

clean-manager:
	rm -f manager
