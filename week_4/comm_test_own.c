#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void root_task(int my_rank, int uni_size);
void client_task(int my_rank, int uni_size);
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

	if (0 == my_rank) {
		root_task(my_rank, uni_size);
	}
	else {
		client_task(my_rank, uni_size);
	}
	

	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}


void root_task(int my_rank, int uni_size)
{
	// creates and initialies transmission variables
    int recv_message, source, tag, count;
    recv_message = source = tag = 0;
    count = 1;
    MPI_Status status;

	// set up timing variables
	struct timespec start_time, end_time, time_diff;
	double runtime = 0.0;

	// iterates through all the other ranks
    for (int their_rank = 1; their_rank < uni_size; their_rank++)
    {

		// sets the source argument to the rank of the sender
        source = their_rank;

		//start timing
		timespec_get(&start_time, TIME_UTC);

		// receives the messages
        MPI_Recv(&recv_message, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

		// stop timing
		timespec_get(&end_time, TIME_UTC);
		// calculate runtime
		time_diff = calculate_runtime(start_time, end_time);
		runtime = to_second_float(time_diff);
		// prints the message from the sender
        printf("Rank %d took %lf seconds to read from %d\n", my_rank, runtime, source);
    }
}

void client_task(int my_rank, int uni_size)
{
	// creates and initialies transmission variables
    int send_message, dest, tag, count;
    send_message = dest = tag = 0;
    count = 1;

	// set up timing variables
	struct timespec start_time, end_time, time_diff;
	double runtime = 0.0;

	// creates the message
	send_message = my_rank * 10;

	//start timing
	timespec_get(&start_time, TIME_UTC);

	// sends the message
	MPI_Send(&send_message, count, MPI_INT, dest, tag, MPI_COMM_WORLD);

	// stop timing
	timespec_get(&end_time, TIME_UTC);
	// calculate runtime
	time_diff = calculate_runtime(start_time, end_time);
	runtime = to_second_float(time_diff);

	// prints the message from the sender
	printf("Rank %d took %lf seconds to send\n", my_rank, runtime);

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
    if (uni_size < 2)
    {
        printf("Unable to communicate with less than 2 processes. MPI communicator size = %d\n", uni_size);
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