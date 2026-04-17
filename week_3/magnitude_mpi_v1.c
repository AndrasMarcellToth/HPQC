#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function declarations
int check_args(int argc, char **argv);
void initialise_vector(int vector[], int size);
int vector_magnitude_squared(int vector[], int size);
void root_task(int num_arg, int uni_size);
void client_task(int my_rank, int num_arg, int uni_size);

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

	// declare and initialise the numerical argument variable
	int num_arg = check_args(argc, argv);

    // check if root or client
    if (my_rank == 0) {
        root_task(num_arg, uni_size);
    } else {
        client_task(my_rank, num_arg, uni_size);
    }

    // finalise MPI
	ierror = MPI_Finalize();
	return 0;
}

// The function executed by root
void root_task(int num_arg, int uni_size)
{
	// creates a vector variable
    int *vector = malloc(num_arg * sizeof(int));
	// and initialises every element to random nunmbers
    initialise_vector(vector, num_arg);

	// calculate chunk size and remainder
    int chunk = num_arg / uni_size;
    int remainder = num_arg % uni_size;

    // root keeps its own chunk
    int local_sum = vector_magnitude_squared(vector, chunk);


    // send chunks to client processes
    for (int p = 1; p < uni_size; p++)
    {
		// calculate starting index for each process
        int p_start = p * chunk;

		// set size of chunk and add remiander to the last process
		int p_size = chunk;
		if (p == uni_size - 1) {
			p_size = chunk + remainder;
		}
		// send chunk to process p
        MPI_Send(&vector[p_start], p_size, MPI_INT, p, 0, MPI_COMM_WORLD);
    }

    // summ results
    int total_sum = local_sum;

	// creates and initialies transmission variables
	int recv_message = 0;
	int count = 1;
	MPI_Status status;

	// add result from each chunk
    for (int p = 1; p < uni_size; p++)
    {
        MPI_Recv(&recv_message, count, MPI_INT, p, 0, MPI_COMM_WORLD, &status);
        total_sum += recv_message;
    }

	// print final result
    printf("Magnitude Squared: %d\n", total_sum);

	// free up memory
    free(vector);
}

// The function executed by the cliet processes
void client_task(int my_rank, int num_arg, int uni_size)
{

	// calculate chunk size and remainder
    int chunk = num_arg / uni_size;
    int remainder = num_arg % uni_size;

	 // set local size and add remainder to the last process
    int local_size = chunk;
    if (my_rank == uni_size - 1) {
		local_size += remainder;
	}
        
	// create loacl vector and allocate memory
    int *local_vector = malloc(local_size * sizeof(int));

	// creates and initialies transmission variables
	int source = 0;
	int dest = 0;
	int count = 1;
	MPI_Status status;

    // receive chunk from root
    MPI_Recv(local_vector, local_size, MPI_INT, source, 0, MPI_COMM_WORLD, &status);

    // compute local sum
    int local_sum = vector_magnitude_squared(local_vector, local_size);
	
    // send result back to root
    MPI_Send(&local_sum, count, MPI_INT, dest, 0, MPI_COMM_WORLD);

    free(local_vector);
}


// defines a function to sum a vector of ints into another int
int vector_magnitude_squared(int vector[], int size)
{
	// creates a variable to hold the sum
	int sum = 0;

	// iterates through the vector
	for (int i = 0; i < size; i++)
	{
		// sets the elements of the vector to the initial value
		sum += vector[i] * vector[i];
	}

	// returns the magnitude squared
	return sum;
}

// defines a function to initialise all values in a vector to a given inital value
void initialise_vector(int vector[], int size)
{
    // set random seed to fixed value for reproducibility
	srand(42);
    // iterates through the vector
	for (int i = 0; i < size; i++)
	{
		// sets the elements of the vector to random numer between 0 and 99
		vector[i] = rand() % 100;
	}
}


// defines a function that checks your arguments to make sure they'll do what you need
int check_args(int argc, char **argv)
{
	// declare and initialise the numerical argument
	int num_arg = 0;

	// check the number of arguments
	if (argc == 2) // program name and numerical argument
	{
		// declare and initialise the numerical argument
		num_arg = atoi(argv[1]);
	}
	else // the number of arguments is incorrect
	{
		// raise an error
		fprintf(stderr, "ERROR: You did not provide a numerical argument!\n");
		fprintf(stderr, "Correct use: %s [NUMBER]\n", argv[0]);

		// and exit COMPLETELY
		exit (-1);
	}
	return num_arg;
}

