ALL:swclient swserver
OBJONE=swclient.c swsocketfunc.c
OBJTWO=swserver.c swsocketfunc.c
swclient:$(OBJONE) 
	gcc $(OBJONE) -o swclient  
swserver:$(OBJTWO)
	gcc $(OBJTWO) -o swserver 
clean:
	rm swclient swserver

