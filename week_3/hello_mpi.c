#include <stdio.h>
#include <mpi.h>

/*
This programme was written based on the sample hello_mpi.c and proof.c.
The root process prints to terminal the order in which messages are
recived from the client processes.
*/

// Function declarations
int root_task(int size);
void client_task(int rank);

int main(int argc, char **argv)
{
    // declare and initialise error handling variable
	int ierror = 0;
	
	// declare and initialise rank and size varibles
	int my_rank, uni_size;
	my_rank = uni_size = 0;

	// intitalise MPI
	ierror = MPI_Init(&argc, &argv);

	// gets the rank and world size
	ierror = MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	ierror = MPI_Comm_size(MPI_COMM_WORLD,&uni_size);

    // check if root or client
    if (my_rank == 0) {
        root_task(uni_size);
    } else {
        client_task(my_rank);
    }

    // finalise MPI
	ierror = MPI_Finalize();
	return 0;
}

// root process
int root_task(int size)
{
    printf("I am root, 0 out of %d processes\n", size);
    printf("Processes reported in this order:\n");

    // creates and initialies transmission variables
	int recv_message, count, source, tag;
	recv_message = source = tag = 0;
	count = 1;
	MPI_Status status;

    for (source = 1; source < size; source++) 
    {
        MPI_Recv(&recv_message, count, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);

        printf("%d ", recv_message);
    }
    return 0;
}

// client processes
void client_task(int rank)
{
    // creates and initialies transmission variables
	int send_message, count, dest, tag;
	send_message = dest = tag = 0;
	count = 1;

	// sets the destination for the message
	dest = 0; // destination is root

	// creates the message
	send_message = rank;

	// sends the message
	MPI_Send(&send_message, count, MPI_INT, dest, tag, MPI_COMM_WORLD);
}