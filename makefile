CC=clang
LD=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ld
MIG=mig

all: processorcontrol server client lsserv notifications

processorcontrol: processorcontrol.o
	$(LD) $< -macosx_version_min 10.8 -framework System -o $@

server: server.o Info.plist
	$(LD) $(filter %.o, $<) -sectcreate __TEXT __info_plist Info.plist -macosx_version_min 10.8 -framework System -o $@

client: client.o
	$(LD) $< -macosx_version_min 10.8 -framework System -o $@

exceptionhandler: exceptions.c /usr/include/mach/mach_exc.defs
	$(MIG) /usr/include/mach/mach_exc.defs
	$(CC) -c exceptions.c -o exceptions.o -Wall -Werror
	$(CC) -c mach_excServer.c -o mach_excServer.o
	$(LD) exceptions.o mach_excServer.o -macosx_version_min 10.8 -framework System -lcapstone -o $@

notifications: notifications.c include/UserNotification/UNDRequest.defs
	$(MIG) -I./include/ -user UNDRequest.c include/UserNotification/UNDRequest.defs
	rm UNDRequestServer.c
	$(CC) -c notifications.c -o notifications.o -I./include/ -Wall -Werror
	$(CC) -c UNDRequest.c -o UNDRequest.o -I./include/
	$(LD) UNDRequest.o notifications.o -macosx_version_min 10.8 -framework System -o $@

lsserv: lsserv.o
	$(LD) $< -macosx_version_min 10.8 -framework System -o $@

%.o: %.c
	$(CC) -c $< -o $@ -Wall -Werror

.PHONY: all clean

clean:
	rm -f processorcontrol processorcontrol.o server.o client.o server client
