# Week 4 Tasks
## Part 1: Demonstrating Communications
### Running as-is

Below is the output from running the comm_test_mpi as-is with 6 processes.
```
mpirun -np 6 ~/bin/comm_test_mpi
Hello, I am 3 of 6. Sent 30 to Rank 0
Hello, I am 5 of 6. Sent 50 to Rank 0
Hello, I am 4 of 6. Sent 40 to Rank 0
Hello, I am 0 of 6. Received 10 from Rank 1
Hello, I am 0 of 6. Received 20 from Rank 2
Hello, I am 0 of 6. Received 30 from Rank 3
Hello, I am 0 of 6. Received 40 from Rank 4
Hello, I am 0 of 6. Received 50 from Rank 5
Hello, I am 1 of 6. Sent 10 to Rank 0
Hello, I am 2 of 6. Sent 20 to Rank 0

```

It can be seen that the order in which the different processes print is somewhat random; however, the rot (rank 0) receives messages from the other ranks in order.

### Functionalising

Functionalising and further development was done on a copy of the original code comm_test_own.c

The following functions were created and moved outside of main()

- root_task(): the code the root rank executes
- client_task(): the code the client pranks execute
- initialise_mpi(): a wrapper for the mpi initialisation steps
- check_ranks(): a wrapper for checking if the correct number of ranks are given, and error handling.

These changes are documented in the commit history.

### Experimenting With Send Types

The different version of send were implemented in comm_test_own. The different versions can be seen in the commit history.

#### Ssend()

The syntax of Ssend() (synchronous send) is identical to regular send(), so implementation was simple. In the case of this code, Ssend should not impact the result, as root is ready to receive from the start, and no process needs to be waited for. 

The output with 6 processes printed in the same random order as with Send().
```
mpirun -np 6 ~/bin/comm_test_own
Hello, I am 1 of 6. Sent 10 to Rank 0
Hello, I am 4 of 6. Sent 40 to Rank 0
Hello, I am 5 of 6. Sent 50 to Rank 0
Hello, I am 0 of 6. Received 10 from Rank 1
Hello, I am 0 of 6. Received 20 from Rank 2
Hello, I am 0 of 6. Received 30 from Rank 3
Hello, I am 0 of 6. Received 40 from Rank 4
Hello, I am 0 of 6. Received 50 from Rank 5
Hello, I am 2 of 6. Sent 20 to Rank 0
Hello, I am 3 of 6. Sent 30 to Rank 0

```

#### Bsend()

Buffered send required slight modifications to the code, as memory has to be allocated to the buffer and handed to mpi. This was done by adding the following lines to client_task() before MPI_Bsend():
```
// set up buffer and allocate memory
int buffer_size = sizeof(int) + MPI_BSEND_OVERHEAD;
void *buffer = malloc(buffer_size);

// hand buffer to mpi
MPI_Buffer_attach(buffer, buffer_size);
```
Like with Ssend(), the output from this was expected to be the same as regular Send() as root has no other processes to run before receiving.

Like before, the output with 6 processors printed in a seemingly random order.
```
mpirun -np 6 ~/bin/comm_test_own
Hello, I am 0 of 6. Received 10 from Rank 1
Hello, I am 0 of 6. Received 20 from Rank 2
Hello, I am 1 of 6. Sent 10 to Rank 0
Hello, I am 2 of 6. Sent 20 to Rank 0
Hello, I am 4 of 6. Sent 40 to Rank 0
Hello, I am 5 of 6. Sent 50 to Rank 0
Hello, I am 0 of 6. Received 30 from Rank 3
Hello, I am 0 of 6. Received 40 from Rank 4
Hello, I am 0 of 6. Received 50 from Rank 5
Hello, I am 3 of 6. Sent 30 to Rank 0
```

#### Rsend()

Like Ssend() the syntax for Rsend() is identical as regular send(). Implementation was therefore trivial.

Like with all the previous sends, Rsend() is expected to behave the same, however, as it relies on root being ready, it may throw an error if a small timing mismatch occurs from all the processes trying to send at once. In practice, with this code, this was not observed over the 10 or so test runs, and the output was the same as for all the previous versions.
```
mpirun -np 6 ~/bin/comm_test_own
Hello, I am 4 of 6. Sent 40 to Rank 0
Hello, I am 3 of 6. Sent 30 to Rank 0
Hello, I am 2 of 6. Sent 20 to Rank 0
Hello, I am 5 of 6. Sent 50 to Rank 0
Hello, I am 0 of 6. Received 10 from Rank 1
Hello, I am 0 of 6. Received 20 from Rank 2
Hello, I am 0 of 6. Received 30 from Rank 3
Hello, I am 0 of 6. Received 40 from Rank 4
Hello, I am 0 of 6. Received 50 from Rank 5
Hello, I am 1 of 6. Sent 10 to Rank 0

```

#### Isend()

For the Isend() implementation, an extra lines needed to be added to client_task() after MPI_Isend():
```
MPI_Wait(&request, MPI_STATUS_IGNORE);
```
This line blocks any further election until the root returns confirmation that it received the message.

As the client process in this code does nothing after sending its message, this should not change the functioning of the code. The output from running it with 6 processes is shown below.
```
mpirun -np 6 ~/bin/comm_test_own
Hello, I am 2 of 6. Sent 20 to Rank 0
Hello, I am 3 of 6. Sent 30 to Rank 0
Hello, I am 4 of 6. Sent 40 to Rank 0
Hello, I am 5 of 6. Sent 50 to Rank 0
Hello, I am 0 of 6. Received 10 from Rank 1
Hello, I am 0 of 6. Received 20 from Rank 2
Hello, I am 0 of 6. Received 30 from Rank 3
Hello, I am 0 of 6. Received 40 from Rank 4
Hello, I am 0 of 6. Received 50 from Rank 5
Hello, I am 1 of 6. Sent 10 to Rank 0

```
#### Comparison

All four versions of send were found to function the same in this application. As such, for a simple programme like this, regular Send() is likely the simplest and most reliable choice.

### Implementing Timing

Timing was implemented by using timing functions defined in code from week 2. Both send and receive functions were timed. The results are shown below with 6 processors.
```
mpirun -np 6 ~/bin/comm_test_own
Rank 5 took 0.000009 seconds to send
Rank 1 took 0.000011 seconds to send
Rank 2 took 0.000013 seconds to send
Rank 3 took 0.000009 seconds to send
Rank 4 took 0.000013 seconds to send
Rank 0 took 0.000080 seconds to read from 1
Rank 0 took 0.000003 seconds to read from 2
Rank 0 took 0.000002 seconds to read from 3
Rank 0 took 0.000002 seconds to read from 4
Rank 0 took 0.000002 seconds to read from 5
```
The runtime for the message sending is tiny, on the order of 1e-5, 1e-6 seconds with some seemingly random variation. However, receive generally appears to be faster.

## Part 2: Benchmarking Latency and Bandwidth

### Ping-pong Message Latency

The ping-pong logic was implemented in ping_pong.c. The programme requires exactly 2 ranks (a ping and a pong) and exactly 1 integer argument that sets how many ping-pong cycles to run, e.g.:
```
mpirun -np 2 ~/bin/ping_pong 100
```
The plot below shows the runtime against the number of ping-pong cycles. It can be seen that time increases approximately linearly with the number of massage cycles.

![ping-pong plot](figs/ping-pong.png)

