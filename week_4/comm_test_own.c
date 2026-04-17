#include <stdio.h>
#include <mpi.h>

void root_task(int my_rank, int uni_size);
void client_task(int my_rank, int uni_size);
void initialise_mpi(int *argc, char ***argv, int *my_rank, int *uni_size);
void check_ranks(int uni_size);

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

	// iterates through all the other ranks
    for (int their_rank = 1; their_rank < uni_size; their_rank++)
    {

		// sets the source argument to the rank of the sender
        source = their_rank;

		// receives the messages
        MPI_Recv(&recv_message, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

		// prints the message from the sender
        printf("Hello, I am %d of %d. Received %d from Rank %d\n",
                my_rank, uni_size, recv_message, source);
    }
}

void client_task(int my_rank, int uni_size)
{
	// creates and initialies transmission variables
    int send_message, dest, tag, count;
    send_message = dest = tag = 0;
    count = 1;

	// creates the message
	send_message = my_rank * 10;

	// sends the message
	MPI_Send(&send_message, count, MPI_INT, dest, tag, MPI_COMM_WORLD);

	// prints the message from the sender
	printf("Hello, I am %d of %d. Sent %d to Rank %d\n", my_rank, uni_size, send_message, dest);

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