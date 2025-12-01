// Anthony Tawil - 101275351
//Joeseph Dereje - 101316700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

#define EXAM_QUESTIONS 5
#define MAX_EXAMS 20

typedef struct {
    char student_number[12];
    int marked[EXAM_QUESTIONS];
} Exam;

typedef struct {
    char rubric[EXAM_QUESTIONS];
} Rubric;

typedef struct {
    Exam current_exam;
    Rubric current_rubric;
} SharedMemory;

void random_sleep(int min_ms, int max_ms) {
    int delay = min_ms + rand() % (max_ms - min_ms + 1);
    usleep(delay * 1000);
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("Usage: %s <num_TAs>\n", argv[0]);
        return 1;
    }
    int num_TAs = atoi(argv[1]);
    srand(time(NULL));

    int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);
    if(shmid < 0){ perror("shmget"); exit(1);}
    SharedMemory *shm = (SharedMemory*) shmat(shmid, NULL, 0);
    if(shm == (void*)-1){ perror("shmat"); exit(1);}

    snprintf(shm->current_exam.student_number, sizeof(shm->current_exam.student_number), "0001");
    for(int i=0; i<EXAM_QUESTIONS; i++){
        shm->current_exam.marked[i]=0;
        shm->current_rubric.rubric[i]='A'+i;
    }

    sem_t *rubric_sem = sem_open("/rubric_sem", O_CREAT | O_EXCL, 0666, 1);
    sem_t *exam_sem   = sem_open("/exam_sem", O_CREAT | O_EXCL, 0666, 1);

    for(int i=0;i<num_TAs;i++){
        if(fork()==0){
            int q;
            while(1){
                for(q=0;q<EXAM_QUESTIONS;q++){

                    random_sleep(500,1000);
                    if(rand()%2){
                        sem_wait(rubric_sem);
                        if(shm->current_rubric.rubric[q] < 'Z'){
                            shm->current_rubric.rubric[q]++;
                            printf("TA %d corrected rubric question %d to %c\n", i+1, q+1, shm->current_rubric.rubric[q]);
                        }
                        sem_post(rubric_sem);
                    }

                    if(!shm->current_exam.marked[q]){
                        shm->current_exam.marked[q]=1;
                        random_sleep(1000,2000);
                        printf("TA %d marked student %s question %d\n", i+1, shm->current_exam.student_number, q+1);
                    }
                }

                sem_wait(exam_sem);
                int sn = atoi(shm->current_exam.student_number);
                if(sn >= MAX_EXAMS){
                    sem_post(exam_sem);
                    break;
                }
                sn++;
                snprintf(shm->current_exam.student_number, sizeof(shm->current_exam.student_number), "%04d", sn);
                for(q=0;q<EXAM_QUESTIONS;q++) shm->current_exam.marked[q]=0;
                sem_post(exam_sem);
            }
            shmdt(shm);
            exit(0);
        }
    }

    // Parent waits for child termination
    for(int i=0;i<num_TAs;i++) wait(NULL);

    shmctl(shmid, IPC_RMID, NULL);
    sem_unlink("/rubric_sem");
    sem_unlink("/exam_sem");

    printf("All exams finished. Program terminated.\n");
    return 0;
}
