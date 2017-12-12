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
#include <semaphore.h>
#include <fcntl.h>      // O_CREAT

sem_t *sm;
sem_t *n_sm;

int set_cpu_affinity( int cpuid ) {
   printf( "Setting cpu affinity to core: %d\n", cpuid );
   pthread_t current_thread = pthread_self();                          
   cpu_set_t cpuset;                                                   
   CPU_ZERO( &cpuset );                                                  
   CPU_SET( cpuid, &cpuset );                                      
   return pthread_setaffinity_np( current_thread, 
                                  sizeof( cpu_set_t ), &cpuset ); 
}

int lookup_posix_sem( const char* sem_name, sem_t* sema ) {
   sema = sem_open( sem_name, O_RDWR );
   if ( sema == SEM_FAILED ) {
      perror( "sem_open failed!" );
      return -1;
   }

   return 0;
}

int main( int argc, char* argv[] ) {
   if ( argc <= 1 )
      printf( "Usage: ./rr-task <PRIORITY> <PROCESS-NAME> <CPUID> <NEXT_SEM_ID>\n" );
   set_cpu_affinity( atoi( argv[3] ) );
   std::string pname ( argv[2] );
   std::string next_pname ( argv[4] );
   std::string my_sem_name = "/SEM" + pname;
   std::string next_sem_name = "/SEM" + next_pname;

   int res = lookup_posix_sem( my_sem_name.c_str(), sm );
   if ( res < 0 )
      perror( "FAILED: lookup_posix_sem(). errno");

   res = lookup_posix_sem( next_sem_name.c_str(), n_sm );
   if ( res < 0 )
      perror( "FAILED: lookup_posix_sem(). errno");

   uint32_t n = 0; 
   printf( "Before while loop. my_sem_name: %s. next_sem_name: %s \n", my_sem_name.c_str(), next_sem_name.c_str() );
   fflush( stdout );
   while ( 1 ) {
      //std::cout << pname.c_str() << std::endl;
      n += 1;
      if ( !( n % 10000 ) ) {
         int val;
         sem_getvalue( sm, &val );
         
         std::cout << "begin pname: " <<pname.c_str() << "val: " << val  << std::endl;
        
         res = sem_wait( sm );
         std::cout << "res: " << res << std::endl;
         if( res < 0 ) {           
            perror( "Failed sem_wait. Errno:" );
         }      

         printf( "Inst:%s RR Prio %s running (n=%u)\n", argv[2], argv[1], n );
         fflush( stdout );

         res = sem_post( n_sm );
         if( res < 0 ) {           
            perror( "Failed sem_post. Errno:" );
         }      
         std::cout << "end pname: " <<pname.c_str() << std::endl;
         
         sched_yield();
      }
      //sched_yield();
   }

   sem_close( sm );
   sem_close( n_sm );
}
