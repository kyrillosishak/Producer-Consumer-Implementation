#include "header.h"


int main (int argc, char **argv) {
    key_t s_key;
    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort array [1];
    } sem_attr;
    int shm_id;
    struct shared_memory *memory;
    int mutex_sem, buffer_count_sem, spool_signal_sem;

    //  mutual exclusion semaphore
    /* generate a key1 for creating semaphore  */
    if ((s_key = ftok (MUTEX_KEY, PROJECT_ID)) == -1) perror ("ftok");
    if ((mutex_sem = semget (s_key, 1, 0)) == -1) perror ("semget");

    // Get shared memory
    if ((s_key = ftok (SHARED_MEMORY_KEY, PROJECT_ID)) == -1) perror ("ftok");
    if ((shm_id = shmget (s_key, sizeof ( shared_memory), 0)) == -1) perror ("shmget");
    if ((memory = ( shared_memory *) shmat (shm_id, NULL, 0)) == ( shared_memory *) -1) perror ("shmat");

    //producer
    if ((s_key = ftok (BUFFER_COUNT_KEY, PROJECT_ID)) == -1) perror ("ftok");
    if ((buffer_count_sem = semget (s_key, 1, 0)) == -1) perror ("semget");

    // consumer
    if ((s_key = ftok (SPOOL_SIGNAL_KEY, PROJECT_ID)) == -1) perror ("ftok");
    if ((spool_signal_sem = semget (s_key, 1, 0)) == -1) perror ("semget");

    struct sembuf asem [1];

    asem [0].sem_num = 0;
    asem [0].sem_op = 0;
    asem [0].sem_flg = 0;

    char buf [200];

    const int nrolls=10000;  // number of experiment
    default_random_engine generator;
    normal_distribution<double> distribution(stoi(argv[2]),stoi(argv[3]));
    vector<double> p;
    for (int i=0; i<nrolls; ++i) {
        double number = distribution(generator);
        p.push_back(number);
    }
    int i =0;
    while (true) {
        time_t t = time(NULL);

        int max_buffer = stoi(argv[5]);
        string price = to_string(p[i]);i++;
        struct tm tm = *localtime(&t);
        printf("\033[;31m[%d/%02d/%02d %02d:%02d:%02d] %s: Generating a new value %s\033[0m\n" , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,argv[1] , price.c_str());
        string ns = argv[1];
        string s = ns + ',' + price+"->";
        for(int x = 0; x < s.length();x++){
            buf[x] = s[x];
        }
        buf[s.length()-1] = '\0';
        
        asem [0].sem_op = -1;
        if (semop (buffer_count_sem, asem, 1) == -1) perror ("semop: buffer_count_sem");

        
        printf("\033[;31m[%d/%02d/%02d %02d:%02d:%02d] %s: trying to get mutex on shared buffer\033[0m\n" , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, argv[1] );
        
        asem [0].sem_op = -1;
        if (semop (mutex_sem, asem, 1) == -1) perror ("semop: mutex_sem");

        // Critical section
        sprintf (memory -> buf [memory -> producer_index], "%s\n", buf);
        printf("\033[;31m[%d/%02d/%02d %02d:%02d:%02d] %s: placing %s on shared buffer\033[0m\n" , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, argv[1] , price.c_str());
        (memory -> producer_index)++;
        if (memory -> producer_index == MAX_BUFFERS) memory -> producer_index = 0;

        // Release mutex semaphore
        asem [0].sem_op = 1;
        if (semop (mutex_sem, asem, 1) == -1)
            perror ("semop: mutex_sem");

        // consumer semaphore
        asem [0].sem_op = 1;
        if (semop (spool_signal_sem, asem, 1) == -1)
            perror ("semop: spool_signal_sem");

        printf("\033[;31m[%d/%02d/%02d %02d:%02d:%02d] %s: sleeping for %s ms\033[0m\n" , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, argv[1] ,argv[4]);
        usleep(stoi(argv[4])*1000);
    }

    if (shmdt ((void *) memory) == -1)
        perror ("shmdt");
    exit (0);
}



