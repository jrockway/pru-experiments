This repository contains a program to count how long a signal connected to
`PRU0_0` remains on.  I use it to see if the PPS output of my GPS makes any
sense.

You might be interested in reading the code if you want your Beaglebone Black to
use the PRU to time things or communicate with the host processor, though
honestly I just copied pretty much everything from TI's own examples.

Just connect something to `PRU0_0` (`P9_31`) and run `./run.sh`.  Some output
will appear showing you how many nanoseconds the cycle time is, and how long the
signal is on for.

In my case, my GPS generates a 1ms (1000000ns) pulse every second, and I see
output like:

    total=1000047630ns rise=999985ns
    total=1000047570ns rise=999985ns
    total=1000047585ns rise=999985ns
    total=1000047570ns rise=999985ns
    total=1000047615ns rise=999985ns
    total=1000047585ns rise=999965ns

The total time moves around because of quantization error in the GPS's clock
(this can be corrected by asking the GPS over a serial connection what the
offset is; it's a result of the GPS's internal clock not lining up with GPS
time).  The time the signal is high is always about a millisecond, though there
are some blips that I will have to investigate.  I am not sure anything is
guaranteeing me that it will always be exactly one millisecond.

More PRU information: https://gist.github.com/jrockway/03afa853940634abe588c30644ac9cbe


