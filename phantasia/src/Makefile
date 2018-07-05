phantasia : main.o account.o character.o commands.o event.o fight.o \
	 file.o hack.o info.o init.o io.o itcombat.o md5c.o misc.o socket.o \
	 stats.o tags.o treasure.o

	/usr/bin/gcc -D_REENTRANT -g -o phantasia -pthread -lm main.o \
		account.o character.o commands.o event.o fight.o file.o \
		hack.o info.o init.o io.o itcombat.o md5c.o misc.o socket.o \
		stats.o tags.o treasure.o

     main.o : main.c
	/usr/bin/gcc -D_REENTRANT -c -g main.c
     account.o : account.c
	/usr/bin/gcc -D_REENTRANT -c -g account.c
     character.o : character.c
	/usr/bin/gcc -D_REENTRANT -c -g character.c
     commands.o : commands.c
	/usr/bin/gcc -D_REENTRANT -c -g commands.c
     event.o : event.c
	/usr/bin/gcc -D_REENTRANT -c -g event.c
     fight.o : fight.c
	/usr/bin/gcc -D_REENTRANT -c -g fight.c
     file.o : file.c
	/usr/bin/gcc -D_REENTRANT -c -g file.c
     hack.o : hack.c
	/usr/bin/gcc -D_REENTRANT -c -g hack.c
     info.o : info.c
	/usr/bin/gcc -D_REENTRANT -c -g info.c
     init.o : init.c
	/usr/bin/gcc -D_REENTRANT -c -g init.c
     io.o : io.c
	/usr/bin/gcc -D_REENTRANT -c -g io.c
     itcombat.o : itcombat.c
	/usr/bin/gcc -D_REENTRANT -c -g itcombat.c
     md5c.o : md5c.c
	/usr/bin/gcc -D_REENTRANT -c -g md5c.c
     misc.o : misc.c
	/usr/bin/gcc -D_REENTRANT -c -g misc.c
     socket.o : socket.c
	/usr/bin/gcc -D_REENTRANT -c -g socket.c
     stats.o : stats.c
	/usr/bin/gcc -D_REENTRANT -c -g stats.c
     tags.o : tags.c
	/usr/bin/gcc -D_REENTRANT -c -g tags.c
     treasure.o : treasure.c
	/usr/bin/gcc -D_REENTRANT -c -g treasure.c
     clean :
	cp phantasia phantasia.old
	rm phantasia *.o
