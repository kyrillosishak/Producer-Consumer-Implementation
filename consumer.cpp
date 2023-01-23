#include "header.h"
void error (string msg);
void func(unordered_map<string , deque<double>> m,double avg ,vector<pair<int,int>> &vp , string s , int i){
    float xx = avg *1.0/4.0;
    if(m[s].size() <= 4) xx = m[s].front();
    cout << "| " << s;
    for(int x = 0 ; x < 12 - s.length() - 1; x++)cout << " ";
    cout << "|";
    if(vp[i].first < m[s].front() && vp[i].second < avg*1.0/4.0) {
        printf("\033[;32m%7.2lf↑\033[0m\t|  \033[;32m%7.2lf↑\033[0m\t|\n" ,m[s].front(), xx);

    }else if(vp[i].first > m[s].front() && vp[i].second < avg*1.0/4.0){
        printf("\033[;31m%7.2lf↓\033[0m\t|  \033[;32m%7.2lf↑\033[0m\t|\n" ,m[s].front(), xx);
    }else if( vp[i].first < m[s].front() && vp[i].second > avg*1.0/4.0){
        printf("\033[;32m%7.2lf↑\033[0m\t|  \033[;31m%7.2lf↓\033[0m\t|\n" ,m[s].front(), xx);
    }else if (vp[i].first > m[s].front() && vp[i].second > avg*1.0/4.0){
        printf("\033[;31m%7.2lf↓\033[0m\t|  \033[;31m%7.2lf↓\033[0m\t|\n" ,m[s].front(), xx);
    }else{
        printf("%7.2lf\t|%7.2lf\t|\n" ,m[s].front(), xx);
    }
    vp[i].first = m[s].front();

    vp[i].second = xx;
}
int delete_segment(int seg_id){
    if ((shmctl(seg_id,IPC_RMID,0))==-1){
    std::cout<<" ERROR(C++)with shmctl(IPC_RMID): "<<strerror(errno)<<std::endl;
    return -1;
    }else//on success
        return 0;
}
int three , one , two;
int threee , onee , twoe;
void clean_segments(){

    struct shmid_ds shm_info;
    struct shmid_ds shm_segment;
    int max_id = shmctl(0,SHM_INFO,&shm_info);
    if (max_id>=0){
        for (int i=0;i<=max_id;++i) {
                int shm_id = shmctl(i , SHM_STAT , &shm_segment);
                if (shm_id<=0)
                    continue;
                else if (shm_segment.shm_nattch==0){
                    delete_segment(shm_id);
                }
        }
    }
}
void  INThandler(int sig)
{
    char  c;

    signal(sig, SIG_IGN);
    printf("OUCH, did you hit Ctrl-C?\n"
           "Do you really want to quit? [y/n] ");
    cin >> c;
    if (c == 'y' || c == 'Y') {
        if(semctl(three, 1, IPC_RMID)) perror("semctl:");
        if(semctl(one, 1, IPC_RMID)) perror("semctl:");
        if(semctl(two, 1, IPC_RMID)) perror("semctl:");
         if ((shmctl(threee,IPC_RMID,0))==-1) perror("shmctl:");
        exit(0);
    }else
        signal(SIGINT, INThandler);
    cin >> c; // Get new line character
}
int main (int argc, char **argv)
{

   int k = atoi(argv[1]);
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

    printf ("spooler: hello world\n");
    //  mutual exclusion semaphore
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (MUTEX_KEY, PROJECT_ID)) == -1) error ("ftok");
    if ((mutex_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1) error ("semget");
    // Giving initial value.
    sem_attr.val = 0;        // locked, till we finish initialization
    if (semctl (mutex_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // Get shared memory
    if ((s_key = ftok (SHARED_MEMORY_KEY, PROJECT_ID)) == -1) error ("ftok");
    if ((shm_id = shmget (s_key, sizeof (struct shared_memory), 0660 | IPC_CREAT)) == -1) error ("shmget");
    if ((memory = (struct shared_memory *) shmat (shm_id, NULL, 0))== (struct shared_memory *) -1)
        error ("shmat");
    // Initialize the shared memory
    memory -> producer_index = 0;
    memory -> consumer_index = 0;

    // counting semaphore, indicating the number of available buffers.
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (BUFFER_COUNT_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((buffer_count_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // giving initial values
    sem_attr.val = MAX_BUFFERS;    // MAX_BUFFERS are available
    if (semctl (buffer_count_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // counting semaphore, indicating the number of strings to be printed.
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (SPOOL_SIGNAL_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((spool_signal_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // giving initial values
    sem_attr.val = 0;    // 0 strings are available initially.
    if (semctl (spool_signal_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // Initialization complete; now we can set mutex semaphore as 1 to
    // indicate shared memory segment is available
    sem_attr.val = 1;
    if (semctl (mutex_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");
    three = mutex_sem;
    one = spool_signal_sem;
    two = buffer_count_sem;
    threee = shm_id;
    struct sembuf asem [1];
    asem [0].sem_num = 0;
    asem [0].sem_op = 0;
    asem [0].sem_flg = 0;
    unordered_map<string , deque<double>> m;
    vector<pair<int,int>> vp(11 , {0,0});
    m["ALUMINIUM"].push_front(0);m["COPPER"].push_front(0);
    m["NICKEL"].push_front(0);m["LEAD"].push_front(0);
    m["NATURALGAS"].push_front(0);
    m["MENTHAOIL"].push_front(0);m["COTTON"].push_front(0);
    m["CRUDEOIL"].push_front(0);m["GOLD"].push_front(0);
    m["SILVER"].push_front(0);m["ZINC"].push_front(0);
    signal(SIGINT, INThandler);
    while (true) {  // forever
        // Is there a string to print? P (spool_signal_sem);

        asem [0].sem_op = -1;
        if (semop (spool_signal_sem, asem, 1) == -1)
            perror ("semop: spool_signal_sem");

        printf ("%s", memory -> buf [memory -> consumer_index]);
        string st = memory -> buf [memory -> consumer_index];
        int brea = 0;
        int dot = 0 ;
        int end = 0;
        for(int i =0 ; i < st.size() ; i++){
            if(st[i] == ',') brea = i;
            if(st[i] == '.') dot = i ;
            if(st[i] == '-'){
                end = i;
                break;
            }
        }
        string commedity;
        for(int i =0 ; i < brea ; i++){
            commedity+= st[i];
        }
        int price = 0;
        double nextprice = 0 ;
        for(int i =brea+1 ; i < dot ; i++){
            price*=10;
            price+=(st[i]-'0');
        }
        int counter = 0 ;
        for(int i =dot+1 ; i < end ; i++){
            nextprice*=10;
            nextprice+=(st[i]-'0');
            counter++;
        }
        nextprice/= pow(10,counter);
        nextprice+=price;
        m[commedity].push_front(nextprice);
        double avg = 0;int t = 0;

        printf("\e[H\e[2J\e[3J");
        cout << "+---------------------------------------+" << endl;
        cout << "| Currency   |  Price   | AvgPrice  \t|" << endl;
        t = 5;
        for(auto it : m["ALUMINIUM"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"ALUMINIUM",0);
        t = 5;avg = 0;
        for(auto it : m["COPPER"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"COPPER",1);
        //printf("| COPPER        |   %7.2d   |  %7.2lf  |" , m["COPPER"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["NICKEL"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"NICKEL",2);
        //printf("| NICKEL        |   %7.2d   |  %7.2lf  |" , m["NICKEL"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["LEAD"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"LEAD",3);
        //printf("| LEAD          |   %7.2d   |  %7.2lf  |" , m["LEAD"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["MENTHAOIL"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"MENTHAOIL",4);
        //printf("| MENTHAOIL     |   %7.2d   |  %7.2lf  |" , m["MENTHAOIL"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["COTTON"]){
            avg += it;t--;
        }
        func(m,avg,vp,"COTTON",5);
        //printf("| COTTON        |   %7.2d   |  %7.2lf  |" , m["COTTON"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["NATURALGAS"]){
            avg += it;t--;
        }
        func(m,avg,vp,"NATURALGAS",6);
        //printf("| NATURALGAS    |   %7.2d   |  %7.2lf  |" , m["NATURALGAS"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["CRUDEOIL"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"CRUDEOIL",7);
        //printf("| CRUDEOIL      |   %7.2d   |  %7.2lf  |" , m["CRUDEOIL"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["GOLD"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"GOLD",8);
        //printf("| GOLD          |   %7.2d   |  %7.2lf  |" , m["GOLD"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["SILVER"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"SILVER",9);
        //printf("| SILVER        |   %7.2d   |  %7.2lf  |" , m["SILVER"].front() , avg*1.0/4.0);
        t = 5;avg = 0;
        for(auto it : m["ZINC"]){
            if(t <= 0) break;
            avg += it;t--;
        }
        func(m,avg,vp,"ZINC",10);
        //printf("| ZINC          |   %7.2d   |  %7.2lf  |" , m["ZINC"].front() , avg*1.0/4.0);
        cout << "+---------------------------------------+"  << endl;
        cout << "+---------------------------------------+" ;
        /* Since there is only one process (the spooler) using the
           consumer_index, mutex semaphore is not necessary */
        (memory -> consumer_index)++;
        if (memory -> consumer_index == MAX_BUFFERS)
            memory -> consumer_index = 0;

        /* Contents of one buffer has been printed.
           One more buffer is available for use by producers.
           Release buffer: V (buffer_count_sem);  */
        asem [0].sem_op = 1;
        if (semop (buffer_count_sem, asem, 1) == -1)
            perror ("semop: buffer_count_sem");

        //sleep(1);
    }
    
}

// Print system error and exit
void error (string msg)
{
    perror (msg.c_str());
    exit (1);
}

