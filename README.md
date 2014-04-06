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

## Road Map
At some point I plan to mess around with the `mig` preprocessor. I want to see if I can create raw messages to communicate with apple apps (for instance with a general xpc server or something of the such). At some point I'll look into the `vm` as well, but currently more interested in `launchd` and `ipc`.