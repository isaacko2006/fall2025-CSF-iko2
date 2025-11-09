CONTRIBUTIONS

TODO: write a brief summary of how each team member contributed to
the project.

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
