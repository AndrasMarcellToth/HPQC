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
void root_task(int my_rank, int uni_size, int points, int chunk, int time_steps, double step_size, char *path);
void client_task(int my_rank, int uni_size, int chunk, int time_steps, double step_size);
void write_to_file(char *path, double *all_data, int time_steps, int points, int uni_size, int chunk, double *time_stamps);


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

	// calculate chunk size and remainder
    int chunk = num_arg / uni_size;
    int remainder = num_arg % uni_size;

	if (0 == my_rank)
        root_task(my_rank);
    else
        client_task(my_rank);

	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}

void root_task(int my_rank, int uni_size, int points, int chunk, int time_steps, double step_size, char *path)
{	


	// creates a vector variable for the current positions
	double* positions = (double*) malloc(points * sizeof(double));
	// and initialises every element to zero
	initialise_vector(positions, points, 0.0);

	// allocate memory for local vector chunk
    double *local_positions = malloc(chunk * sizeof(double));

    // scatter to all processes
    MPI_Scatter(positions, chunk, MPI_DOUBLE, local_positions, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// creates a vector for the time stamps in the data
	double* time_stamps = (double*) malloc(time_steps * sizeof(double));
	initialise_vector(time_stamps, time_steps, 0.0);
	generate_timestamps(time_stamps, time_steps, step_size);

	// allocate memory for all the local data
	double *all_local_data = malloc(time_steps * chunk * sizeof(double));
	// add the current position of the points to the full local data set
		for (int j = 0; j < chunk; j++) {
			all_local_data[j] = local_positions[j];
		}

	// iterates through each time step in the collection
	for (int i = 1; i < time_steps; i++)
	{	


		// updates the position using a function
		update_positions_root(local_positions, chunk, time_stamps[i]);
		// add new postiotns to array of some kind

		// add the current position of the points to the full local data set
		for (int j = 0; j < chunk; j++) {
			all_local_data[i * chunk + j] = local_positions[j];
		}

		// send last element to next rank
		MPI_Send(&local_positions[chunk-1], 1, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD);

	}


	// allocate memory for all data from all ranks
	double *all_data = malloc(time_steps * points * sizeof(double));

	// declare transmission variable
	int count = time_steps * chunk;
	int dest = 0;
	// gather data from all ranks
	MPI_Gather(all_local_data, count, MPI_DOUBLE, all_data, count, MPI_DOUBLE, dest, MPI_COMM_WORLD);

	// the data at this point is in a flattened 2D array. [rank0 time0, rank0 time1, ... rank1 time0, ...]
	// with each element (e.g. rank0 time0) containing every point in the chunk of the rank at the given time.

	// write out to file
	write_to_file(path, all_data, time_steps, points, uni_size, chunk, time_stamps);

	// if we use malloc, must free when done!
	free(time_stamps);
	free(positions);
	free(local_positions);
	free(all_local_data);
	free(all_data);


}

void client_task(int my_rank, int uni_size, int chunk, int time_steps, double step_size)
{

	// allocate memory for local vector chunk
    double *local_positions = malloc(chunk * sizeof(double));


    // receive scattered chunks
    MPI_Scatter(NULL, chunk, MPI_DOUBLE, local_positions, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// creates a vector for the time stamps in the data
	double* time_stamps = (double*) malloc(time_steps * sizeof(double));
	initialise_vector(time_stamps, time_steps, 0.0);
	generate_timestamps(time_stamps, time_steps, step_size);

	// initialise boudary
	double boundary = 0.0;

	// allocate memory for all the local data
	double *all_local_data = malloc(time_steps * chunk * sizeof(double));
	// add the current position of the points to the full local data set
	for (int j = 0; j < chunk; j++) {
		all_local_data[j] = local_positions[j];
	}
	
	// declare transmission variables
	MPI_Status status; 

	// iterates through each time step in the collection
	for (int i = 1; i < time_steps; i++)
	{
   			
		// recive strating position from previous rank
		MPI_Recv(&boundary, 1, MPI_DOUBLE, my_rank-1, 0, MPI_COMM_WORLD, &status);

		// updates the position using a function
		// need to change driver function
		update_positions_client(local_positions, chunk, boundary);

		// add the current position of the points to the full local data set
		for (int j = 0; j < chunk; j++) {
			all_local_data[i * chunk + j] = local_positions[j];
		}

		// check if last rank
		if (my_rank < uni_size - 1) {
			// send last element to next rank
			MPI_Send(&local_positions[chunk-1], 1, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD);
		}
		
	}


	// declare transmission variable
	int count = time_steps * chunk;
	int dest = 0;
	// gather data from all ranks
	MPI_Gather(all_local_data, count, MPI_DOUBLE, NULL, count, MPI_DOUBLE, dest, MPI_COMM_WORLD);

	// if we use malloc, must free when done!
	free(time_stamps);
	free(local_positions);
	free(all_local_data);
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

// defines a function to update the positions used for root only
void update_positions_root(double* positions, int points, double time)
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

// updates positions for client ranks. Does not include driver, but uses previous rank as starting point
void update_positions_client(double* positions, int points, double boundary)
{
    // creates a temporary vector variable for the new positions
    double* new_positions = (double*) malloc(points * sizeof(double));

	// initialises the index
	int i = 0;
    // first element set from previous rank boundary
    new_positions[i] = boundary;

    // creates new positions by setting value of previous element
    for (i = 1; i < points; i++)
    {
        new_positions[i] = positions[i-1];
    }

    // propagates these new positions to the old ones
    for (int i = 0; i < points; i++)
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

// initializes mpi settings
void initialise_mpi(int *argc, char ***argv, int *my_rank, int *uni_size)
{
    int ierror = 0;
    ierror = MPI_Init(argc, argv);
    ierror = MPI_Comm_rank(MPI_COMM_WORLD, my_rank);
    ierror = MPI_Comm_size(MPI_COMM_WORLD, uni_size);
}

// function ot write tou data to file
void write_to_file(char *path, double *all_data, int time_steps, int points, int uni_size, int chunk, double *time_stamps)
{
    // creates a file
    FILE *out_file;
		out_file = fopen(path, "w");
    print_header(&out_file, points);

    // iterates through each time step in the collection
    for (int i = 0; i < time_steps; i++)
    {
        // print index and timestamp
        fprintf(out_file, "%d, %lf", i, time_stamps[i]);

        // itterates over each rank
        for (int rank = 0; rank < uni_size; rank++) {

            // iterates over all of the points on the line
            for (int j = 0; j < chunk; j++) {

				// prints each y-position to a file
				// [rank * time_steps * chunk] -> the steaps to the start of each ranks data
				// [i * chunk] -> the steps to the start of each time step in the data of the rank
                fprintf(out_file, ", %lf", all_data[rank * time_steps * chunk + i * chunk + j]);
            }
        }
        // prints a new line
        fprintf(out_file, "\n");
    }

    // close file
    fclose(out_file);
}