Chaos controller v1.0 by agentOfChaos

This project realizes an idea I had some time ago:
what if we could influence a program's random number generator, so that it
generate nubers of our choice? (just like having an improbability drive in operation ;-)

Basically it modifies the memory region of the target process in which libc resides,
replacing the 'call rand_r' instruction with a nice 'move your_value to eax',
bypassing completely the random number generation and returning your chosen value
instead.

The executable itself is called 'chaos_control' because it's shorter and cooler.

Tested on ubuntu linux 64 bit