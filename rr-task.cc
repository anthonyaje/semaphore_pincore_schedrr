// Author: Anthony Anthony.
#include <cstdlib>
#include <stdio.h>
#include <sched.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
//#include <unistd.h>
#include <semaphore.h>
#include <iostream>
#include <sys/types.h>
#include <fcntl.h>      // O_CREAT

sem_t* sm;

int set_cpu_affinity( int cpuid ) {
   pthread_t current_thread = pthread_self();                          
   cpu_set_t cpuset;                                                   
   CPU_ZERO( &cpuset );                                                  
   CPU_SET( cpuid, &cpuset );                                      
   return pthread_setaffinity_np( current_thread, 
                                  sizeof( cpu_set_t ), &cpuset ); 
}

int lookup_semaphore() {
   sm = sem_open( "/SEM_CORE", O_RDWR );
   if ( sm == SEM_FAILED ) {
      std::cerr << "sem_open failed!" << std::endl ;
      return -1;
   }
}

int main( int argc, char* argv[] ) {
   printf( "Usage: ./rr-task <PRIORITY> <PROCESS-NAME> <CPUID>\n" );
   printf( "Setting SCHED_RR and priority to %d\n", atoi( argv[1] ) );

   set_cpu_affinity( atoi( argv[3] ) );

   lookup_semaphore();

   int res;
   uint32_t n = 0; 
   while ( 1 ) { 
      n += 1;
      if ( !( n % 10000 ) ) {

         int val;
         sem_getvalue( sm, &val );
         res = sem_wait( sm );
         if( res != 0 ) {
            printf(" sem_wait %s. errno: %d\n", argv[2], errno);             
         }
         std::cout << argv[2] << " Val: " << val << std::endl;
                
         printf( "Inst:%s RR Prio %s running (n=%u)\n", argv[2], argv[1], n );
         fflush( stdout );

         sem_post( sm );

         sched_yield();
      }
      
      sched_yield();
   }
   
   sem_close( sm );
}
