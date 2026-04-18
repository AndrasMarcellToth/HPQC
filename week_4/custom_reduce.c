#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function declarations
int check_args(int argc, char **argv);
void initialise_vector(int vector[], int start, int size);
int vector_magnitude_squared(int vector[], int size);
void root_task(int num_arg, int uni_size);
void client_task(int my_rank, int num_arg, int uni_size);
void custom_sum(void *in, void *out, int *count, MPI_Datatype *datatype);

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

    // creates full vector
    int *vector = malloc(num_arg * sizeof(int));
	// assignes values to vetor
    initialise_vector(vector, 0, num_arg);
    
    // allocate memory for local vector chunk
    int *local_vector = malloc(chunk * sizeof(int));

    // scatter to all processes
    MPI_Scatter(vector, chunk, MPI_INT, local_vector, chunk, MPI_INT, 0, MPI_COMM_WORLD);


	// calculates local sum
    int local_sum = vector_magnitude_squared(local_vector, chunk);
    
    // root now needs to hadle remainder as well os needs to sum the end of the vector
    int remainder_sum = vector_magnitude_squared(vector + (num_arg- remainder), remainder);

    // sum results
    local_sum = local_sum + remainder_sum;

    // create custom sum operation
    MPI_Op custom_op;
    MPI_Op_create(&custom_sum, 1, &custom_op);

    // create and initialise receive variables
    int recv_message = 0;
    int count = 1;
    MPI_Status status;

    // crate sun variable
    int total_sum = 0;
    // get results from all ranks with custom op
    MPI_Reduce(&local_sum, &total_sum, count, MPI_INT, custom_op, 0, MPI_COMM_WORLD);
    
    // print final result
    printf("Magnitude Squared: %d\n", total_sum);
	// free up memory
	free(vector);
    free(local_vector);
}

// The function executed by the cliet processes
void client_task(int my_rank, int num_arg, int uni_size)
{

    // calculate chunk size and remainder
    int chunk = num_arg / uni_size;
    int remainder = num_arg % uni_size;

    // allocate memory for local vector
    int *local_vector = malloc(chunk * sizeof(int));
    // receive scattered chunks
    MPI_Scatter(NULL, chunk, MPI_INT, local_vector, chunk, MPI_INT, 0, MPI_COMM_WORLD);

    // remainder is now handelled by root

    // calculate local sum
    int local_sum = vector_magnitude_squared(local_vector, chunk);

    // create custom sum operation
    MPI_Op custom_op;
    MPI_Op_create(&custom_sum, 1, &custom_op);

    // create and initialise transmission variables
    int count = 1;
    int dest = 0;
    MPI_Status status;

    // send result to root
    MPI_Reduce(&local_sum, NULL, count, MPI_INT, custom_op, 0, MPI_COMM_WORLD);

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

void custom_sum(void *in, void *out, int *count, MPI_Datatype *datatype)
{

    //convert in and out to integer arrays
    int *in_ints = (int *)in;
    int *out_ints = (int *)out;

    // get the actual length of count, not just memory address
    int length = *count;

    // itterate over all elements
    for (int i = 0; i < length; i++)
    {
        // add in value to out value for each element of the array
        out_ints[i] = out_ints[i] + in_ints[i];
    }
}