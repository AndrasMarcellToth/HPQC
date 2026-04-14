import sys
import time

'''
The programme read the file 'i_o_time_py.txt' created by time_write.py back into 
memory, and prints the processing time.
'''

def main(): 

    # gets the start time for the loop
    start_time = time.time()

    # open write file
    with open('i_o_time_py.txt', 'r', encoding='utf-8') as f:
        data = f.read()

    # gets the end time for the loop
    end_time = time.time()
    # gets the total time
    read_time = end_time - start_time

    # prints the runtime
    print("\nTime for read: {} seconds\n".format(read_time))

if __name__ == "__main__":
    main()