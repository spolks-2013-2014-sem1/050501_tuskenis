#include "semaphore_wrap.h"

Semaphore::Semaphore()
{
    this->sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT);

    if (this->sem_id == -1) {
        perror("semget()");
    }
}

Semaphore::~Semaphore()
{
    if (semctl(this->sem_id, 0, IPC_RMID) == -1) {
        perror("semctl()");
    }
}

void Semaphore::Wait()
{
    struct sembuf sem_buf;
    sem_buf.sem_num = 0;
    sem_buf.sem_op = 0;
    sem_buf.sem_flg = 0;

    if (semop(this->sem_id, &sem_buf, 1) == -1) {
        perror("wait semop()");
    }

    this->Set();
}

void Semaphore::Set()
{
    struct sembuf sem_buf;
    sem_buf.sem_num = 0;
    sem_buf.sem_op = 1;
    sem_buf.sem_flg = 0;

    if (semop(this->sem_id, &sem_buf, 1) == -1) {
        perror("set semop()");
    }
}

void Semaphore::Reset()
{
    struct sembuf sem_buf;
    sem_buf.sem_num = 0;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = 0;

    if (semop(this->sem_id, &sem_buf, 1) == -1) {
        perror("set semop()");
    }
}

