#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function declarations
int check_args(int argc, char **argv);
void initialise_vector(int vector[], int start, int size);
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
    // calculate chunk size and remainder
    int chunk = num_arg / uni_size;
    int remainder = num_arg % uni_size;

    // creates its own vector chunk and allocates memory
    int *vector = malloc(chunk * sizeof(int));
	// assignes values to vetor
    initialise_vector(vector, 0, chunk);
	// calculates local sum
    int local_sum = vector_magnitude_squared(vector, chunk);
    

    // sum results
    int total_sum = local_sum;

    // create and initialise receive variables
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

    // add remainder to last process
    int local_size = chunk;
    if (my_rank == uni_size - 1) {
        local_size += remainder;
    }

    // calculate start index
    int start = my_rank * chunk;

	// create local vector and allocate mamory
    int *local_vector = malloc(local_size * sizeof(int));
	// assign values to vector
    initialise_vector(local_vector, start, local_size);

    // calculate local sum
    int local_sum = vector_magnitude_squared(local_vector, local_size);

    // create and initialise transmission variables
    int count = 1;
    int dest = 0;
    MPI_Status status;

    // send result to root
    MPI_Send(&local_sum, count, MPI_INT, dest, 0, MPI_COMM_WORLD);

    // free up memory
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
void initialise_vector(int vector[], int start, int size)
{
    // set random seed to fixed value for reproducibility
	srand(42);
    // itterate over unused random numbers
    for (int i = 0; i < start; i++) rand();
    // assign values to vector chunk
    for (int i = 0; i < size; i++)
    {
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

