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
real    0m0.399s
user    0m0.383s
sys     0m0.009s
-time ./parsort /tmp/$(whoami)/test_16M.in 1048576
real    0m0.216s
user    0m0.374s
sys     0m0.027s
-time ./parsort /tmp/$(whoami)/test_16M.in 524288
real    0m0.159s
user    0m0.434s
sys     0m0.036s
-time ./parsort /tmp/$(whoami)/test_16M.in 262144
real    0m0.129s
user    0m0.453s
sys     0m0.047s
-time ./parsort /tmp/$(whoami)/test_16M.in 131072
real    0m0.138s
user    0m0.452s
sys     0m0.079s
-time ./parsort /tmp/$(whoami)/test_16M.in 65536
real    0m0.109s
user    0m0.477s
sys     0m0.069s
-time ./parsort /tmp/$(whoami)/test_16M.in 32768
real    0m0.114s
user    0m0.500s
sys     0m0.106s
-time ./parsort /tmp/$(whoami)/test_16M.in 16384
real    0m0.123s
user    0m0.513s
sys     0m0.170s

-Explanation for Times From Testing:

Our times generally follow the pattern displayed in the instructions "You should see that decreasing the threshold decreased the total time, 
although depending on the number of CPU cores available, eventually a point of dimimishing returns will be reached.". This makes sense because there is more parallelism
which helps split the work up by utilizing the CPU in a more efficient manner using fork/split (the OS schedules parent and child to work on different CPU cores). 

There is a gradual shift from the benefits gained from splitting up the work into  
various processes for the CPU to handle to the CPU having too many processes to handle at once and the splitting causes significant overhead that actually slows down time. 
Variation between the tests can also be explained by the fact that the OS scheduling or background processes that can cause flucations in real time.

This explains the overall pattern of our times listed above, as at first, the parallelism helps by splitting up work, but eventually, too many child processes exist creating 
far too much overhead for the CPU to handle efficiently. In other words, when the threshold begins to become too small, more processes are created than actually necessary, causing 
the overhead to waste time. We can see that the user time dips sligtly as we begin to lower the threshold, but begins to spike back up to what it was at the beginning as the 
threshold gets much smaller.

A really interesting observation is that the system time gets increasingly bigger as the threshold increases, which is a great explanantion as to why there are diminishing returns 
as eventually the system time used to handle all of the processes involved like fork, waitpid, and mmap grows to be a bit too much, and the plethora of processes actually bogs 
down the CPU a bit instead of giving it the time improvemements from previous thresholds. 

From our time data, the best test we had in terms of time appeared to be the threshold of 65,536, which had a real time of 0.109s, a user time of 0.477s, and a sys time of 0.170s. Larger thresholds 
than this appear to not utilize the CPU cores to the best ability, while thresholds lower than this appear to introduce too many processes and overhead for the CPU to
imrpove its time any further (diminishing returns).