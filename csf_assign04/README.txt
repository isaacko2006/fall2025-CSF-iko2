CONTRIBUTIONS

TODO: write a brief summary of how each team member contributed to
the project.
Isaac Ko (iko2) - Finished the Main function using the assignment4 instructions as a template, ran time testing and copied all of the times gathered into README
Gavin Simurdiak (gsimurd1) - Implemented the quicksort using child processes using assignment4 instructions as a template, analyzed time data and wrote 
    analysis into README

REPORT

TODO: add your report according to the instructions in the
"Experiments and analysis" section of the assignment description.

-time ./parsort /tmp/$(whoami)/test_16M.in 2097152
real    0m0.922s
user    0m0.871s
sys     0m0.036s
-time ./parsort /tmp/$(whoami)/test_16M.in 1048576
real    0m0.805s
user    0m0.764s
sys     0m0.034s
-time ./parsort /tmp/$(whoami)/test_16M.in 524288
real    0m0.850s
user    0m0.772s
sys     0m0.070s
-time ./parsort /tmp/$(whoami)/test_16M.in 262144
real    0m0.759s
user    0m0.687s
sys     0m0.067s
-time ./parsort /tmp/$(whoami)/test_16M.in 131072
real    0m0.910s
user    0m0.802s
sys     0m0.096s
-time ./parsort /tmp/$(whoami)/test_16M.in 65536
real    0m0.832s
user    0m0.704s
sys     0m0.121s
-time ./parsort /tmp/$(whoami)/test_16M.in 32768
real    0m1.010s
user    0m0.829s
sys     0m0.164s
-time ./parsort /tmp/$(whoami)/test_16M.in 16384
real    0m1.103s
user    0m0.854s
sys     0m0.242s

-Explanation for Times From Testing:
Our times generally follow the pattern displayed in the instructions "You should see that decreasing the threshold decreased the total time, 
although depending on the number of CPU cores available, eventually a point of dimimishing returns will be reached.". This makes sense because there is more parallelism
which helps split the work up by utilizing the CPU in a more efficient manner using fork/split. However, there definitely are some oddities in our time results, as 
real time dips down and back up for various tests (ex. threshold 1048576 has real time 0.805s, then threshold 524288 has real time 0.850s, and then threshold 262144 has
0.759 s). This is odd, as the threshold gets continually lower for these three tests, yet the real time start around ~0.8s, then jumps up to ~0.85s, then shoots back down
to ~0.76 s. This rollercoaster of times is no doubt a bit confusing, but most likely a result of the gradual shift from the benefits gained from splitting up the work into  
various processes for the CPU to handle to the CPU having too many processes to handle at once and the splitting causes significant overhead that actually slows fown time. 
This explains the overall pattern of our times listed above, as at first, the parallelism helps by splitting up work, but eventually, too many child processes exist creating 
far too much overhead for the CPU to handle efficiently. We can see that the user time dips sligtly as we begin to lower the threshold, but begins to spkie back up to what
it was at the beginning as the threshold gets much smaller. A really interesting observation is that the system time gets increasingly bigger as the threshold increases, which
is a great explanantion as to why there are diminishing returns as eventually the system time used to handle all of the processes involved like fork, waitpid, and mmap grows
to be a bit too much, and the plethora of processes actually bogs down the CPU a bit instead of giving it the time improvemements from previous thresholds. From our time data,
the best test we had in terms of time appeared to be the threshold of 262144, which had a real time of 0.759s, a user time of 0.687s, and a sys time of 0.067s. Larger thresholds 
than this 262144 appear to not utilize the CPU cores to the best ability, while thresholds lower than this appear to introduce too many processes and overhead for the CPU to
imrpove its time any further (diminishing returns).