#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void root_task(int my_rank, int num_pings);
void client_task(int my_rank, int num_pings);
void initialise_mpi(int *argc, char ***argv, int *my_rank, int *uni_size);
void check_ranks(int uni_size);
double to_second_float(struct timespec in_time);
struct timespec calculate_runtime(struct timespec start_time, struct timespec end_time);

int main(int argc, char **argv) 
{
	// declare and initialise error handling variable
	int ierror = 0;
	
	// declare and initialise rank and size varibles
	int my_rank, uni_size;
	my_rank = uni_size = 0;

	// intitalise MPI
	initialise_mpi(&argc, &argv, &my_rank, &uni_size);
	// check if correct number of ranks
	check_ranks(uni_size);

    int num_pings;

    if (argc == 2)
    {
        num_pings = atoi(argv[1]);
    }
    else
    {
        fprintf(stderr, "Incorrect arguments. Usage: pingpong [NUM_PINGS]\ne.g.\n pingpong 100\n");
        MPI_Finalize();
        exit(-1);
    }

    if (0 == my_rank)
        root_task(my_rank, num_pings);
    else
        client_task(my_rank, num_pings);

	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}


void root_task(int my_rank, int num_pings)
{
	// creates and initialies transmission variables
    int tag, dest, source, count, current_pings;
    tag = source = count, current_pings = 0;
    dest, count = 1;
    MPI_Status status;

    // set up timing variables
    struct timespec start_time, end_time, time_diff;
    double elapsed_time, avg_time;
    elapsed_time = avg_time = 0.0;

    //start timing
    timespec_get(&start_time, TIME_UTC);

    while(current_pings < num_pings) {

        printf("Root with current count %d out of %d", current_pings, num_pings);
        // send the currrent ping count
        MPI_Send(&current_pings, count, MPI_INT, dest, tag, MPI_COMM_WORLD);

        // recive ping count from client
        MPI_Recv(&current_pings, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

        // increment ping count
        current_pings++;
    }

    // stop timing
	timespec_get(&end_time, TIME_UTC);
	// calculate runtime
	time_diff = calculate_runtime(start_time, end_time);
	elapsed_time = to_second_float(time_diff);

	// prints the message from the sender
	printf("Took %lf seconds for %d ping-pongs\n", elapsed_time, num_pings);

}

void client_task(int my_rank, int num_pings)
{
	// creates and initialies transmission variables
    int tag, dest, source, count, current_pings;
    tag = source = count, current_pings = 0;
    dest, count = 1;
    MPI_Status status;
    // recive ping count from client
    MPI_Recv(&current_pings, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

    printf("Client with current count %d out of %d", current_pings, num_pings);

    // increment ping count
    current_pings++;

    // send the currrent ping count
    MPI_Send(&current_pings, count, MPI_INT, dest, tag, MPI_COMM_WORLD);



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