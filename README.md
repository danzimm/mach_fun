# Mach Fun
## Overview
I was interested in how mach ports work on Mac OSX Mavericks and here are my results. Info about each bin below. No copyright here, no warranty, use at will, mention me if you want, enjoy!
### processorcontrol
Must run under root, or optionally (how I run it) set `+xs` for everyone and the owner `root`. Allows you to turn on and off processors at will. I strongly suggest to never turn off the master processor (your computer will stop responding...). Flags:

- `-a` - Select all processors
- `-i <index>` - Select the processor at `index`, starts with `0`
- `-n` - Show the number of processors
- `-e <enable>` - Enable or disable the selected processors, `1 == true` otherwise `false`

If a processor is selected then it will show that processors information. Using `-a` and `-e` will result in ignoring the master processor.
### server
A sample mach server that registers itself as the service `zimm_server`. Simply displays data that is received.
### client
A sample mach client for the above server. Simply takes data from `stdin` and sends it to the server.
### exceptionhandler
A sample program which uses the exception handler API to jump over exceptions to the next instruction. The first interesting thing here is you have to use MIG to generate some code to be used for the exception handlers (granted you could do this manually, but apple already wrote it for you, why rewrite the wheel just not to use mig?). Next I use [capstone](http://www.capstone-engine.org) to disassemble the current instruction and get that instruction's length and then add that to the current `rip` to effectively jump over the bad instruction. I then tell the kernel it's ok to go on with this thread and it is resumed. Those are fancy words for I jumped over the bad code ;D. WARNING: I don't suggest putting this in a normal program, yes it won't crash anymore, but it won't work necessarily either ;P
### notifications
A sample program to hook into the kernel user notification center. Must be ran under root (couldn't figure out how to elminate that, albeit the docs I found said to use `CFUserNotification` for userland notifications ;D). Check the source for the options.
#### usernotif
This is kind of a sub sample program. It uses a notification port thats available to all users. I didn't add options to the binary but they could easily be c/p'd over from notifications. Meow.
### inject
My own version of a mach injector. This uses the `task_for_pid` syscall to gain access over another process. In order to do this you either need to specify `SecTaskAccess` and properly sign the binary with a certificate that's in your System keychain or just run it as root. By default here I have the binary being signed with a certificate called `inject_codesign` which I created and put in my System keychain. Note that to finish properly setting up this certificate you need to do the following:
- Create a certificate using the `Keychain Access` app (click on the `Keychain Access` menu item in the top left, pull down to `Certificate Assistant`->`Create Certificate` (ensure you're not highlighting a private key otherwise it will try to use that private key to link up with the certificate you're creating). Specify `Let me override defaults` and make sure it gets placed in the System keychain (for some reason it wouldn't work for me if I just created it in my login keychain and imported then into my System keychain, I had to create it directly in my System keychain).

- Quit Keychain Access
- `sudo killall taskgated` in order to restart `taskgated` (you could also reboot here)
- `rm -f inject && make` to ensure that inject is remade and resigned with this new certificate. Here it should ask you to type in an admins username/password in order to sign the binary
- inject away!

Please let me know if you have any issues with this. I personally spent an entire night figuring out the beast that is signing for `task_for_pid` and have tried to document above everything I've learned. I do wish that taskgated was open source, but one can only wish I suppose. I also wish that the kernel gave more precise errors than simply `KERN_FAILURE` when `task_for_pid` fails (if you look at the source I'm not sure if I even fail before the `check_task_access` call!). Anyways, the two other bins that are made, `libtstlib.dylib`, `injectee`, are my personal testing binaries.I'm still pretty curious as to exactly how the kernel/taskgated decides whether or not the process is good to go, so if you know please clue me in! I currently believe that I get past `task_for_pid_posix_check` in `./bsd/vm/vm_unix.c` (relative to the root of the xnu source code) so I believe I'm failing the call to `check_task_access` but again I have no idea what witchcraft is happening on the other side of that taskgated port. Anyways I'm done rambling, enjoy!
Oh right I forgot to mention, what makes my injection a little bit different is I believe that I use less memory to actually inject. I don't actually create a proper stack but use a method called ROP in order to call different functions (so I simply set up the stack properly and then call my initial function and I'm good to go). If you have questions about all this do contact me!

## Road Map
At some point I plan to mess around with the `mig` preprocessor. I want to see if I can create raw messages to communicate with apple apps (for instance with a general xpc server or something of the such). At some point I'll look into the `vm` as well, but currently more interested in `launchd` and `ipc`.
