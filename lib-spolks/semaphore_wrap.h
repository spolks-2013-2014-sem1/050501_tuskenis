// Semaphore wrap lib. Header file
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

class Semaphore
{
    private:
        int sem_id;
    public:
        Semaphore();
        ~Semaphore();
        void Wait();
        void Set();
        void Reset();
};

