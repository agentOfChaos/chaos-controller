Chaos controller v1.3 by agentOfChaos<br>

This project realizes an idea I had some time ago:
what if we could influence a program's random number generator, so that it
generate numbers of our choice? (just like having an improbability drive in operation,
but more controlled ;-)<br>

Basically it modifies the memory region of the target process in which libc resides,
replacing the 'call random' instruction with a nice 'move your_value to eax',
bypassing completely the random number generation and returning your chosen value
instead.<br>

The executable itself is called 'chaos_control' because it's shorter and cooler.<br>

The mrmaps shell script must stay in the same directory as the executable<br>

Before compiling:
the mrmaps shell script contains a variable named `setup_libc`, make sure that
its value define unambiguously the libc shared object used by your system.
Thake a look in your /proc/$somepid/maps and look for "libc-<version>.so"
That should be setup_libc 's value.

Compiling: <br>
make release

Usage:
./chaos_control --help <br>
will provide instructions and examples

Tested on ubuntu linux x86_64 <br>
Tested on Arch linux x86_64