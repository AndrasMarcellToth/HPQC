
# Week 2 Tasks

## Time and benchmarking

Below is the output from testing the time command with ls:

```
totha4@cheetah:~/HPQC$ time ls
README.md

real	0m0.006s
user	0m0.000s
sys	0m0.005s
```

### C vs Python Hello World! Run Time

Below is the output from running both hello_world.py, and the compiled version of hello_world.c:

```
totha4@cheetah:~/HPQC/week_2$ time python3 week_2_sample_code/hello_world.py
Hello, World!

real	0m0.032s
user	0m0.026s
sys	0m0.004s
totha4@cheetah:~/HPQC/week_2$ time ~/bin/hello_world
Hello, World!

real	0m0.007s
user	0m0.000s
sys	0m0.004s
```

It can be seen above that the total runtime for the python version was 0.062s comapered to just 0.011s for the c version.

### C vs Python Repeat Adder

Note: The sample python code had indent errors. In the version used here, indent were corrected, but nothing else was changed.

The figure below shows the runtime of the python and c repeat adder multiplying 11 by 10, 100, 1000, etc. The runtime of both increases approximately linearly (note the fig uses a log scale); however, the python version is significantly slower, taking ~33 s compared to ~2 s at 10e9.

![Runtime plot](figs/runtime_plot.png)

At 10e9 the c version returned a negative number, indicating an integer overflow.

### Internal Timing

Below is the output from both time_print programmes with an argument of 10.

```
andris@andris-ubuntu:~/DCU/high_performance_and_quantum/HPQC/week_2$ python3 time_print.py 10
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 

Time for loop: 1.239776611328125e-05 seconds

andris@andris-ubuntu:~/DCU/high_performance_and_quantum/HPQC/week_2$ ~/bin/time_print 10
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 

Runtime for core loop: 0.000061 seconds.
```






