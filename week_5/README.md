# Week 5 Tasks

## Part 1: Using and Updating the Code

string_wave.c was run with an input of 10, and a GIF was made from the resulting data as shown below.

![string gif 1](animate_string_file.gif)

A copy of sting_wave.c was made, string_wave_custom.c, to implement any modifications.

The hard coded variable were removed, and check_args() was updated to check for four arguments, points, cycles, sample, path. The calculation of time_step and step size was also moved to this function to clean up main().
```
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
    ...
}
```
The format for running the updated code is:
```
~/bin/string_wave_custom [points] [cycles] [steps] [path]

e.g.

~/bin/string_wave_custom 50 10 5 ~/data/temp/test_outpu.csv
```

A copy of the python file animate_line_file.py was also made, animate_line_custom.py to make the modification.

## Part 2: Parallelising

### Implementation strategy

The wave problem in its current form in, fundamentally, serial. The motion of the points is calculated from left to right, with each element being calculated based on the previous one. So, while the string can be broken up into segments, each segment needs the last element from its left neighbour before it can begin to calculate, and the last element it needs to pass to its right neighbour will be the last element it can calculate.

As such, without changing the logic of the simulation, it cannot truly be parallelised. Splitting it among several ranks will only add messaging overhead.

Regardless, a simple implementation using scatter and gather was made.