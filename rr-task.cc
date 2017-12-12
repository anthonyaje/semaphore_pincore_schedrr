// Author: Anthony Anthony.
#include <cstdlib>
#include <stdio.h>
#include <sched.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>
#include <sys/types.h>
#include <fcntl.h>      // O_CREAT
#include <cstdlib>
#include <string.h>

#define n_child 2

sem_t* sm;
sem_t* n_sm;

int set_cpu_affinity( int cpuid ) {
   pthread_t current_thread = pthread_self();                          
   cpu_set_t cpuset;                                                   
   CPU_ZERO( &cpuset );                                                  
   CPU_SET( cpuid, &cpuset );                                      
   return pthread_setaffinity_np( current_thread, 
                                  sizeof( cpu_set_t ), &cpuset ); 
}

int lookup_semaphore( const char* sname, const char* nsname ) {
   sm = sem_open( sname, 0 );
   if ( sm == SEM_FAILED ) {
      std::cerr << "sem_open failed!" << std::endl ;
      return -1;
   }

   n_sm = sem_open( nsname, 0 );
   if ( n_sm == SEM_FAILED ) {
      std::cerr << "sem_open failed!" << std::endl ;
      return -1;
   }
}

int main( int argc, char* argv[] ) {
   if ( argc <= 1 )
      printf( "Usage: ./rr-task <PRIORITY> <PROCESS-NAME> <CPUID>\n" );
   
   set_cpu_affinity( atoi( argv[3] ) );

   std::string sem_name = std::string("/SEM_CORE") + argv[2];
   std::string nsem_name = std::string("/SEM_CORE") + std::to_string( ( atoi( argv[ 2 ] ) + 1 ) % n_child );
   lookup_semaphore( sem_name.c_str(), nsem_name.c_str() );
   
   printf( "Id: %s. sem_name: %s. nsem_name: %s.\n", argv[2], sem_name.c_str(), nsem_name.c_str() );
   fflush( stdout );

   int res;
   uint32_t n = 0; 
   while ( 1 ) { 
      n += 1;
      if ( !( n % 10000 ) ) {

         int val;
         sem_getvalue( sm, &val );
         std::cout << "Id: " << argv[2] << " wait_sem Val: " << val << std::endl;
            
         res = sem_wait( sm );
         if( res != 0 ) {
            printf(" sem_wait %s. errno: %d\n", argv[2], errno);             
         }
    
         printf( "Inst:%s RR Prio %s running (n=%u)\n", argv[2], argv[1], n );
         fflush( stdout );

         sem_getvalue( n_sm, &val );
         std::cout << "Id: " << argv[2] << " before post_sem Val: " << val << std::endl;

         sem_post( n_sm );

         sem_getvalue( n_sm, &val );
         std::cout << "Id: " << argv[2] << " after post_sem Val: " << val << std::endl;

         sched_yield();
      }
      
//      sched_yield();
   }
   
   sem_close( sm );
   sem_close( n_sm );
}
