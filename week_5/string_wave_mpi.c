#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// declares the functions that will be called within main
// note how declaration lines are similar to the initial line
// of a function definition, but with a semicolon at the end;
void check_args(int argc, char **argv, int *points, int *cycles, int *samples, char **path, int *time_steps, double *step_size);
void initialise_vector(double vector[], int size, double initial);
void print_vector(double vector[], int size);
int sum_vector(int vector[], int size);
void update_positions(double* positions, int points, double time);
int generate_timestamps(double* time_stamps, int time_steps, double step_size);
double driver(double time);
void print_header(FILE** p_out_file, int points);
void initialise_mpi(int *argc, char ***argv, int *my_rank, int *uni_size);

int main(int argc, char **argv)
{	
	// declare and initialise error handling variable
	int ierror = 0;
	
	// declare and initialise rank and size varibles
	int my_rank, uni_size;
	my_rank = uni_size = 0;

	// intitalise MPI
	initialise_mpi(&argc, &argv, &my_rank, &uni_size);

	// declare and initialise the numerical argument variable
	int points, cycles, samples, time_steps;
	char *path;
	double step_size;
	// chack umber of arguments and calculate parameters
	check_args(argc, argv, &points, &cycles, &samples, &path, &time_steps, &step_size);

	if (0 == my_rank)
        root_task(my_rank);
    else
        client_task(my_rank);

	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}

void root_task(int my_rank, int num_pings)
{	


	// creates a vector for the time stamps in the data
	double* time_stamps = (double*) malloc(time_steps * sizeof(double));
	initialise_vector(time_stamps, time_steps, 0.0);
	generate_timestamps(time_stamps, time_steps, step_size);

	// creates a vector variable for the current positions
	double* positions = (double*) malloc(points * sizeof(double));
	// and initialises every element to zero
	initialise_vector(positions, points, 0.0);

	// creates a file
	FILE* out_file;
     	out_file = fopen(path,"w");
	print_header(&out_file, points);

	// iterates through each time step in the collection
	for (int i = 0; i < time_steps; i++)
	{
		// updates the position using a function
		update_positions(positions, points, time_stamps[i]);

		// prints an index and time stamp
		fprintf(out_file, "%d, %lf", i, time_stamps[i]);

		// iterates over all of the points on the line
		for (int j = 0; j < points; j++)
		{
			// prints each y-position to a file
			fprintf(out_file, ", %lf", positions[j]);
		}
		// prints a new line
		fprintf(out_file, "\n");
	}

	// if we use malloc, must free when done!
	free(time_stamps);
	free(positions);

	// closes the file
	fclose(out_file);

	// creates and initialies transmission variables
    int tag, dest, source, count, current_pings;
    tag = current_pings = 0;
    dest = count = source = 1;
    MPI_Status status;

    // set up timing variables
    struct timespec start_time, end_time, time_diff;
    double elapsed_time, avg_time;
    elapsed_time = avg_time = 0.0;

    //start timing
    timespec_get(&start_time, TIME_UTC);

    while(current_pings < num_pings) {

        // print for debug
        // printf("Root with current count %d out of %d\n", current_pings, num_pings);

        // send the currrent ping count
        MPI_Send(&current_pings, count, MPI_INT, dest, tag, MPI_COMM_WORLD);

        // recive ping count from client
        MPI_Recv(&current_pings, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

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
    tag = dest = source = count = current_pings = 0;
    count = 1;
    MPI_Status status;

    while (current_pings < num_pings) {
        // recive ping count from client
        MPI_Recv(&current_pings, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

        // increment ping count
        current_pings++;

        // print for debug
        // printf("Client with current count %d out of %d\n", current_pings, num_pings);

        // send the currrent ping count
        MPI_Send(&current_pings, count, MPI_INT, dest, tag, MPI_COMM_WORLD);
    }
}

// prints a header to the file
// double-pointer used to allow this function to move the file pointer
void print_header(FILE** p_out_file, int points)
{
	fprintf(*p_out_file, "#, time");
	for (int j = 0; j < points; j++)
	{
		fprintf(*p_out_file, ", y[%d]", j);
	}
	fprintf(*p_out_file, "\n");
}

// defines a simple harmonic oscillator as the driving force
double driver(double time)
{
	double value = sin(time*2.0*M_PI);
	return(value);
}

// defines a function to update the positions
void update_positions(double* positions, int points, double time)
{
	// creates a temporary vector variable for the new positions
        double* new_positions = (double*) malloc(points * sizeof(double));

	// initialises the index
	int i = 0;
	new_positions[i] = driver(time);
	// creates new positions by setting value of previous element 
	for (i = 1; i < points; i++)
	{
		new_positions[i] = positions[i-1];
	}
	// propagates these new positions to the old ones
	for (i = 0; i < points; i++)
        {
                positions[i] = new_positions[i];
        }

	// frees the temporary vector
	free(new_positions);
}

// defines a set of timestamps
int generate_timestamps(double* timestamps, int time_steps, double step_size)
{
	for (int i = 0; i < time_steps ; i++)
	{
		timestamps[i]=i*step_size;
	}	
	return time_steps;
}

// defines a function to sum a vector of ints into another int
int sum_vector(int vector[], int size)
{
	// creates a variable to hold the sum
	int sum = 0;

	// iterates through the vector
	for (int i = 0; i < size; i++)
	{
		// sets the elements of the vector to the initial value
		sum += vector[i];
	}

	// returns the sum
	return sum;
}

// defines a function to initialise all values in a vector to a given inital value
void initialise_vector(double vector[], int size, double initial)
{
	// iterates through the vector
	for (int i = 0; i < size; i++)
	{
		// sets the elements of the vector to the initial value
		vector[i] = initial;
	}
}

// defines a function to print a vector of ints
void print_vector(double vector[], int size)
{
	// iterates through the vector
	for (int i = 0; i < size; i++)
	{
		// prints the elements of the vector to the screen
		printf("%d, %lf\n", i, vector[i]);
	}
}

// checks number of input variable and calculautes all parameters.
void check_args(int argc, char **argv, int *points, int *cycles, int *samples, char **path, int *time_steps, double *step_size)
{

	// check the number of arguments
	if (argc == 5) // program name and numerical argument
	{
		// declare and initialise the numerical argument
		*points = atoi(argv[1]);
		*cycles = atoi(argv[2]);
		*samples = atoi(argv[3]);
		*path = argv[4];

		// creates variables for the vibration
		*time_steps = *cycles * *samples + 1;
		*step_size = 1.0 / *samples;
	}
	else // the number of arguments is incorrect
	{
		// raise an error
		fprintf(stderr, "ERROR: wrong number of parameters!\n");
		fprintf(stderr, "Correct use: %s [points] [cycles] [samples] [path]\n", argv[0]);

		// and exit COMPLETELY
		exit (-1);
	}
}

void initialise_mpi(int *argc, char ***argv, int *my_rank, int *uni_size)
{
    int ierror = 0;
    ierror = MPI_Init(argc, argv);
    ierror = MPI_Comm_rank(MPI_COMM_WORLD, my_rank);
    ierror = MPI_Comm_size(MPI_COMM_WORLD, uni_size);
}
