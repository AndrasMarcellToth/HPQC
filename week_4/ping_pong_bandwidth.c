#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void root_task(int my_rank, int num_pings, int *data, int num_ints);
void client_task(int my_rank, int num_pings, int *data, int num_ints);
void initialise_mpi(int *argc, char ***argv, int *my_rank, int *uni_size);
void check_ranks(int uni_size);
void check_args(int argc, char **argv, int *num_pings, int *num_bytes);
double to_second_float(struct timespec in_time);
void initialise_vector(int vector[], int size, int initial);
struct timespec calculate_runtime(struct timespec start_time, struct timespec end_time);

int main(int argc, char **argv) 
{
	// declare and initialise error handling variable
	int ierror = 0;
	
	// declare and initialise rank and size varibles
	int my_rank, uni_size, num_pings, num_bytes;
	my_rank = uni_size = num_pings = num_bytes = 0;

	// intitalise MPI
	initialise_mpi(&argc, &argv, &my_rank, &uni_size);
	// check if correct number of ranks
	check_ranks(uni_size);
    // check if correct number of arguments and assign to num_pings and num_bytes
    check_args(argc, argv, &num_pings, &num_bytes);

    // calculate required number of ints for requested data size
    int num_ints = num_bytes / sizeof(int);

    // allocate mamory
    int *data = malloc(num_ints* sizeof(int));
    // initialse data to zeros
    initialise_vector(data, num_ints, 0);
 
    if (0 == my_rank)
        root_task(my_rank, num_pings, data, num_ints);
    else
        client_task(my_rank, num_pings, data, num_ints);

    // free memory
    free(data);
	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}


void root_task(int my_rank, int num_pings, int *data, int num_ints)
{
	// creates and initialies transmission variables
    int tag, dest, source;
    tag = 0;
    dest = source = 1;
    MPI_Status status;

    // set up timing variables
    struct timespec start_time, end_time, time_diff;
    double elapsed_time, avg_time;
    elapsed_time = avg_time = 0.0;

    //start timing
    timespec_get(&start_time, TIME_UTC);

    while(data[0] < num_pings) {

        // print for debug
        // printf("Root with current count %d out of %d\n", current_pings, num_pings);

        // send the currrent ping count
        MPI_Send(data, num_ints, MPI_INT, dest, tag, MPI_COMM_WORLD);

        // recive ping count from client
        MPI_Recv(data, num_ints, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

    }

    // stop timing
	timespec_get(&end_time, TIME_UTC);
	// calculate runtime
	time_diff = calculate_runtime(start_time, end_time);
	elapsed_time = to_second_float(time_diff);

	// prints the message from the sender
	printf("Took %lf seconds for %d ping-pongs with %d bytes\n", elapsed_time, num_pings, num_ints * (int)sizeof(int));

}

void client_task(int my_rank, int num_pings, int *data, int num_ints)
{
	// creates and initialies transmission variables
    int tag, dest, source;
    tag = dest = source = 0;
    MPI_Status status;

    while (data[0] < num_pings) {
        // recive ping count from client
        MPI_Recv(data, num_ints, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

        // increment ping count
        data[0]++;

        // print for debug
        // printf("Client with current count %d out of %d\n", current_pings, num_pings);

        // send the currrent ping count
        MPI_Send(data, num_ints, MPI_INT, dest, tag, MPI_COMM_WORLD);
    }
}


void initialise_mpi(int *argc, char ***argv, int *my_rank, int *uni_size)
{
    int ierror = 0;
    ierror = MPI_Init(argc, argv);
    ierror = MPI_Comm_rank(MPI_COMM_WORLD, my_rank);
    ierror = MPI_Comm_size(MPI_COMM_WORLD, uni_size);
}


void check_ranks(int uni_size)
{
    if (uni_size != 2)
    {
        printf("Ping pond requires exactly 2 processors. MPI communicator size = %d\n", uni_size);
        MPI_Finalize();
        exit(1);
    }
}


void check_args(int argc, char **argv, int *num_pings, int *num_bytes)
{
    if (argc == 3)
    {
        *num_pings = atoi(argv[1]);
        *num_bytes = atoi(argv[2]);
    }
    else
    {
        printf("Incorrect arguments. Ping-pong required exactly 2 integer input, [numer of pings] [number of bytes]. E.g., mpiru -np 2 ~/bin/ping_pong 100 1024\n");
        MPI_Finalize();
        exit(-1);
    }

}


double to_second_float(struct timespec in_time)
{
	// creates and initialises the variables
	float out_time = 0.0;
	long int seconds, nanoseconds;
	seconds = nanoseconds = 0;

	// extracts the elements from in_time
	seconds = in_time.tv_sec;
	nanoseconds = in_time.tv_nsec;

	// calculates the time in seconds by adding the seconds and the nanoseconds divided by 1e9
	out_time = seconds + nanoseconds/1e9;

	// returns the time as a double
	return out_time;
}


struct timespec calculate_runtime(struct timespec start_time, struct timespec end_time)
{
	// creates and initialises the variables
	struct timespec time_diff;
	long int seconds, nanoseconds;                                                                                                       seconds = nanoseconds = 0;
	double runtime = 0.0;

	// extracts the elements from start_time and end_time
	seconds = end_time.tv_sec - start_time.tv_sec;
	nanoseconds = end_time.tv_nsec - start_time.tv_nsec;

	// if the ns part is negative
	if (nanoseconds < 0)
	{
		// "carry the one!"
		seconds = seconds - 1;
		nanoseconds = ((long int) 1e9) - nanoseconds;
	}

	// creates the runtime
	time_diff.tv_sec = seconds;
	time_diff.tv_nsec = nanoseconds;

	return time_diff;
}


void initialise_vector(int vector[], int size, int initial)
{
	// iterates through the vector
	for (int i = 0; i < size; i++)
	{
		// sets the elements of the vector to the initial value
		vector[i] = initial;
	}
}