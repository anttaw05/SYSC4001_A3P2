The following program simulates TA's marking exams concurrently

Program 1 -> main.c:
    Uses shared memory, allocated and appointed to each TA
    Race conditions can occur

Program 2 -> main2.c:
Uses shared memory and semaphores
    Access to the rubric is synchronized
    Race conditions can not occur

    To compile and run a):
    gcc main.c -o main
    ./main "n"      //for any number of TA's (n) desired

    To compile and run b):
    gcc main2.c -o main
    ./main "n" -pthread -lrt    //for any number of TA's (n) desired, "-pthread -lrt" needed for semaphores