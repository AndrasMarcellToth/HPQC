# Week 1 Tasks

## Proof.c

I could not find "Demonstrate access to system" on loop.
On inspection, the code appears to takea single integer as the argument, num_arg, and for each
rank in the system, calculates *order of rank x num_arg* then sums it all up.

E.g., with 4 ranks and num_arg = 3, it gives: *3x3 + 2x3 + 1x3 = 18*

After compiling and running the code on cheeta, this was the output:

```
totha4@cheetah:~/HPQC$ mpirun -np 4 bin/proof 3  
No protocol specified  
The combined result is 18
```


This confirms the funtion of the code and shows access to both mpicc and mpirun on cheeta.

## hello_world.c

After compiling with gcc, hello_world.c gave this output:

```
totha4@cheetah:~/HPQC$ bin/hello_world
Hello, World!
```
confirming aceess to gcc.