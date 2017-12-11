// Author: Anthony Anthony.
#include <cstdlib>
#include <stdio.h>
#include <sched.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>      // O_CREAT

int set_cpu_affinity( int cpuid ) {
   pthread_t current_thread = pthread_self();                          
   cpu_set_t cpuset;                                                   
   CPU_ZERO( &cpuset );                                                  
   CPU_SET( cpuid, &cpuset );                                      
   return pthread_setaffinity_np( current_thread, 
                                  sizeof( cpu_set_t ), &cpuset ); 
}

int lookup_vsem( char* pathtokey ) {
   key_t sem_key = ftok( pathtokey, 100 );
   int sem_flag = 0666;
   int sem_id;
   sem_id = semget( sem_key, 0, sem_flag );
   if ( sem_id < 0 ) { 
      std::cerr << "sem_get() failed!\n";
      std::cerr << "errno: " << strerror( errno ) << std::endl;
   } 
   return sem_id;
}

int wait_vsem( int* semid, int idx ) {
   struct sembuf sb;
   sb.sem_num = idx;
   sb.sem_op = -1; 
   sb.sem_flg = 0;
   return semop( *semid, &sb, 1 );
}

int post_vsem( int* semid, int idx ) {
   struct sembuf sb;
   sb.sem_num = idx;
   sb.sem_op = 1;
   sb.sem_flg = 0;
   return semop( *semid, &sb, 1 );
}

int main( int argc, char* argv[] ) {
   printf( "Usage: ./rr-task <PRIORITY> <PROCESS-NAME> <CPUID> <NEXT_SEM_ID>\n" );

   set_cpu_affinity( atoi( argv[3] ) );
   int pname = atoi( argv[2] );
   int next_pname = atoi( argv[4] );
   int semid = lookup_vsem( "/home/h4bian/aqua10/sched_study/VSEM" );

   int res;
   uint32_t n = 0; 
   while ( 1 ) { 
      res = wait_vsem( &semid, pname );
      if( res != 0 ) {
         printf(" sem_wait %s. errno: %d\n", argv[2], errno);             
      }
                
      n += 1;
      if ( !( n % 100000 ) ) {
         printf( "Inst:%s RR Prio %s running (n=%u)\n", argv[2], argv[1], n );
         fflush( stdout );
         //sched_yield();
      }

      res = post_vsem( &semid, next_pname );
      if( res != 0 ) {
         printf(" sem_post %s. errno: %d\n", argv[2], errno);             
      }
      
      sched_yield();
   }
}
