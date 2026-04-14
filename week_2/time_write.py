import sys
import time

'''
The programme takes one integer, N, as the argument and writes every integer
unto N-1 to a file 'i_o_time_py.txt', and prints the processing time.
'''

def main(): 
    # checks if there are the right number of arguments
    try:
        # converts second argument to integer
        in_arg = int(sys.argv[1])
    except: # (argc != 2)
        # raises an error
        raise Exception("Incorrect arguments.\nUsage: python time_write.py [NUM]\ne.g.\npython time_print.py 23")

    # gets the start time for the loop
    start_time = time.time()

    # open write file
    with open('i_o_time_py.txt', 'w', encoding='utf-8') as f:
        # iterates over all numbers up to the input
        for i in range(in_arg):
            # append the next number to the output file
            f.write(str(i) + '\n')

    # gets the end time for the loop
    end_time = time.time()
    # gets the total time
    write_time = end_time - start_time


    # prints the runtime
    print("\n\nTime for write: {} seconds\n".format(write_time))


if __name__ == "__main__":
    main()
