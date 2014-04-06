CC=clang
LD=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ld

all: processorcontrol server client

processorcontrol: processorcontrol.o
	$(LD) $< -macosx_version_min 10.8 -framework System -o $@

server: server.o Info.plist
	$(LD) $(filter %.o, $<) -sectcreate __TEXT __info_plist Info.plist -macosx_version_min 10.8 -framework System -o $@

client: client.o
	$(LD) $< -macosx_version_min 10.8 -framework System -o $@

%.o: %.c
	$(CC) -c $< -o $@ -Wall -Werror

.PHONY: all clean

clean:
	rm -f processorcontrol processorcontrol.o server.o client.o server client
