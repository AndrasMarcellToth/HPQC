
# Week 2 Tasks

## Time and benchmakrking

Output from testing hte time command with ls:

```
totha4@cheetah:~/HPQC$ time ls
README.md

real	0m0.006s
user	0m0.000s
sys	0m0.005s
```

### C vs Python Hello World! Run Time

Below in the uotbut from funning both hello_world.py, and the compiled version of hello_world.c:

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

It can be seen above that the total runtime for the python version was 0.062s comaper to just 0.011s for the c version.

### C vs Python Repeat Adder



