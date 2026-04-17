# Week 3 Tasks

## Part 1: Hello World! MPI

A hello_mpi.c was written bas on the sample hello_mpi.c and proof.c provided. The programme takes the runs separate root and client processes. The clients send their rank to the root, and the rood prints them out in the order received. The root process uses MPI_SOURCE_ANY to not force any order on the processes, but allow them to report when ready.

Below is the output from running the custom hello_mpi with a rank size of 16.

```
totha4@cheetah:~/HPQC/week_3$ time mpirun -np 16 ~/bin/hello_mpi
No protocol specified
I am root, 0 out of 16 processes
Processes reported in this order:
4 1 3 5 6 7 8 9 10 11 12 13 14 15 2
real	0m0.492s
user	0m0.373s
sys	0m0.490s
```

It can be seen that the order of the processes are reporting to root is random. Furthermore, it can be seen that the real time is now considerably less than sys + user time.

Deliberately removing MPI_Init() from the code broke the execution completely, and the programme did not run at all. With MPI_Finalize() removed, the programme still executed, and the expected results were printed to terminal; however, a warning was given about 'improper process termination'.

## Part 2: proof.c Breakdown

The programme takes a single integer as an argument, and returns the sum or the product of the argument and the rank number from each process. 

E.g., with 4 ranks and an argument of 3, it returns: *3x3 + 2x3 + 1x3 = 18*

Below is a brief overview of the functions of the functions in the code. 

Only key logical elements are detailed, variable declarations, returns, error messages are largely skipped.

### Preamble
```
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
```
Imports any external libraries needed for the programme.
```
int root_task(int uni_size);
void client_task(int my_rank, int num_arg);
int check_args(int argc, char **argv);
void check_uni_size(int uni_size);
void check_task(int uni_size, int my_rank, int num_arg);
```
Initial function declarations, sets the data type the functions return thought the programme.


### main()
```
int main(int argc, char **argv) 
```
The main function of the programme that is executed at runtime, receives any inputs from terminal that are required for MPI to function.

```
ierror = MPI_Init(&argc, &argv);
```
Initialise the MPI process. Without this, the code will fail.

```
ierror = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
ierror = MPI_Comm_size(MPI_COMM_WORLD, &uni_size);
```
Gets the rank of the process and the size of the world i.e. the total number of processes.

```
check_uni_size(uni_size);
```
Helper function to ensure a minimum number of processes or 'universe size' is met. Minimum size was set to 1.

```
check_task(uni_size, my_rank, num_arg);
```
Helper function to check if process is root (0) or client (>0) and pass the correct variables to the appropriate function root_task() or client_task()

```
ierror = MPI_Finalize();
```
Finalises, or closes, the MPI process. Needed for a clean finish to the programme.


### root_task()

```
int root_task(int uni_size)
```
The function executed by the root process (0). Takes the universe size (total number of processes) as the argument.

```
for (source = 1; source < uni_size; source++)
	{
		MPI_Recv(&recv_message, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

		output_sum += recv_message;
	}
```

The bulk of the root logic. Iterate though every client process in order (from process 1 up to uni_size -1. Process 0 bing the root) and sums the integer sent by each client process.
```
MPI_Recv(&recv_message, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
```
The MPI communicator handling the receiving of messages from the client processes. It specifies where to store the received message (recv_message), the data type of the massage (MPI_INT), and the source process of message (source).

```
printf("The combined result is %d\n", output_sum);
```
Finally, it prints the result to terminal.

### client_task()

```
void client_task(int my_rank, int num_arg)
```
The function executed by the client processes. Takes the number of its own rank and the input integer as arguments.

```
dest = 0;

send_message = my_rank * num_arg;

MPI_Send(&send_message, count, MPI_INT, dest, tag, MPI_COMM_WORLD);
```

The core logic of the function. Calculated the product of its own rank and the input integer, and sends the result to the root process (dest = 0).

## Optimising proof.c

The programme essentially calculated the product of the input integer and the *n*-th triangular number, where *n* is the number of ranks used. As a single mathematical operation, this could be expressed as:

\[
\text{output} = \text{input integer} * \frac{n^2 + n}{2}
\]

## Part 3: MPI Vector Addition

### vector_serial.c

The programme vector_serial.c takes a single integer argument as input. It generates a vector the length of the input and calculates the sum of the elements.

Below is a brief overview of the functioning of the sample serial vector adding programme.

```
int main(int argc, char **argv)
```
The main function of the programme, takes arguments from the terminal.

```
int num_arg = check_args(argc, argv);
```
Helper functions that ensures exactly 1 integer argument was given after the programme name. If this is true, assigns the argument to the variable num_arg.

```
int* my_vector = malloc (num_arg * sizeof(int));
```
Creates an array for the vector and allocates memory to it. 

```
initialise_vector(my_vector, num_arg, 0);
```
Helper function that initialises every element of the vector to 0.

```
int my_sum = sum_vector(my_vector, num_arg);
```
Helper function to calculate the sum of the vector. Done my simple serial addition of the vector elements in a for loop:
```
int sum = 0;

for (int i = 0; i < size; i++)
{
    sum += vector[i];
}
```
The function that returns the sum.

```
printf("Sum: %d\n", my_sum);

free(my_vector);
```
Finally, the sum is printed to terminal and the memory is freed up.

The programme also includes print_vector() function that prints each element of the vector to the terminal. It is an unused debug feature.

In its current state, the vectors are always trivially filled with zeros. After compiling the code ran in about 0.004 seconds for a vector of 1000 elements.
```
time ~/bin/vector_serial 1000
Sum: 0

real	0m0.003s
user    0m0.000s
sys    0m0.003s

```
## Non-trivial Vector Operation

### Serial

vector_serial.c was modified to calculate the magnitude squared of vectors (magnitude_serial.c).


The function initialise_vector() was modified to generate vectors with random integer elements instead of only zeros. The random seed was set to a fixed value of 42 to ensure reproducibility for benchmarking.

The sum_vector function was replaced with vector_magnitude_squared(). The new function sums the product of each element with its self (i.e. its square). This is then the dot product of the vector with its self, that is, its magnitude squared. 

This simple change makes the calculation significantly more computationally challenging by introducing multiplication. It is also a more physically meaningful property of a vector, that might be of interest in a real application.

With an input of 10000, the programme took ~0.003 seconds to execute. It should be noted that the result here may be wrong due to integer overflow; however, this should not affect performance benchmarking.
```
time ~/bin/magnitude_serial 10000
Magnitude Squared: 32992300

real	0m0.003s
user	0m0.000s
sys	0m0.003s
```
The trivial vector_serial.c took ~6.7 seconds with a vector of the same size.

### Parallel

The first implementation of the mpi (magnitude_mpi_v1.c) calculated the vector once in the root process, then distributed 'chunks' of the vector to each client process. This implementation ran slower than the serial, taking ~0.752 seconds to complete.
```
time mpirun -np 8 ~/bin/magnitude_mpi_v1 10000
Magnitude Squared: 32992300

real	0m0.752s
user	0m0.328s
sys	0m0.878s

```
The worse performance could in part be due to the overhead of setting up the mpi processes. This can be seen in the increased sys time. However, the significantly longer user time suggest that the processing is also slower. This is likely due to the large volume of data being passed back and forth between root and the clients. This is further evidenced by the performance improving for fewer processors.
```
time mpirun -np 2 ~/bin/magnitude_mpi_v1 10000
Magnitude Squared: 32992300

real	0m0.568s
user	0m0.073s
sys	0m0.327s

```
The second implementation of mpi (manitude_mpi_v2.c) calculated the vector elements locally for each client, removing the large data transfer overhead. Although the elements of the vector are 'random', a fixed seed is used. Therefore, the exact same vector can be reproduced in each process.

The resulting performance was not much better.
```
time mpirun -np 8 ~/bin/magnitude_mpi_v2 10000
Magnitude Squared: 32992300

real	0m0.737s
user	0m0.273s
sys	0m0.941s

```
This suggests that the bulk of the runtime for this relatively light calculation is dominated by the setup overhead, and the serial solution is more efficient.