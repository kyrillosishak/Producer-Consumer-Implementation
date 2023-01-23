#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <time.h>
using namespace std;
// Buffer data structures
#define MAX_BUFFERS 12
#define SHARED_MEMORY_KEY "key1"
#define MUTEX_KEY "key2"
#define BUFFER_COUNT_KEY "key3"
#define SPOOL_SIGNAL_KEY "key4"
#define PROJECT_ID 'S'

struct shared_memory {
    char buf [MAX_BUFFERS] [64];
    int producer_index;
    int consumer_index;
};

