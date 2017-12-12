// Author: Anthony Anthony.
#include <sched.h>      // SCHED_RR
#include <stdio.h>
#include <cstdlib>
#include <errno.h>      // errno
#include <iostream>     // cout cerr
#include <semaphore.h>  // semaphore
#include <fcntl.h>      // O_CREAT
#include <unistd.h>     // fork
#include <string.h>     // cpp string
#include <sys/types.h>  // 
#include <sys/wait.h>   // wait()

#define n_child 2

sem_t* sem[ n_child ] = { NULL };
const char* sem_names[ n_child ] = { NULL };

int set_scheduler( int pid, int prio ) {
   int old_sched_policy = sched_getscheduler( pid );    
   struct sched_param param; 
   param.sched_priority = prio;
   int rc = sched_setscheduler( pid, SCHED_RR, &param );
   if (rc == -1) {                                               
      printf("sched_setscheduler call is failed\n");
      printf("errno: %d\n", errno);             
   }                                                             
   else {                                                        
      printf("Old Scheduler: %d\n", old_sched_policy);           
      printf("Current Scheduler: %d\n", sched_getscheduler( pid ));
   }                                                             

}

int init_semaphore(){ 
   for ( int i=0; i < n_child; i++  ) {
      std::string sname = "/SEM_CORE" + std::to_string( i );
      sem_names[ i ] = sname.c_str();
      unsigned init_val = 0;
      if( i == 0 ) {
         init_val = 1;
      } 
      printf( "Creating semaphore: %s. init_val: %u. \n", sname.c_str(), init_val );
      fflush( stdout );
      sem[ i ] = sem_open ( sname.c_str(), O_CREAT, 0644, init_val );
      sem_init( sem[ i ], 0, init_val );
      if ( sem[ i ] == SEM_FAILED ) { 
         perror( "sem_open failed!." );
         return -1;
      }
   }
   return 0;
}

// Fork and exec rr-task. 
// Return pid of child
int fork_and_exec( std::string pname, char* cpuid ){
   int pid = fork();
   if ( pid == 0) {
      // Child
      char* const params[] = { "./rr-task", "99", strdup( pname.c_str() ),
                               cpuid, NULL };
      execv( params[0], params );
      exit(0);
   } 
   else {
      // Parent
      return pid;
   }  
}

int main( int argc, char* argv[] ) {
   if ( argc <= 1 )
      printf( "Usage ./set_scheduler <cpuid> \n" );

   char* cpuid = argv[1];
   std::string pnames[ n_child ];
   
   int childid[ n_child ] = { 0 };
   int i;

   init_semaphore();

   for( i=0; i < n_child; i++ ){
      pnames[ i ] = std::to_string( i );
      childid[ i ] = fork_and_exec( pnames[i].c_str(), cpuid );
//      set_scheduler( childid[ i ], 99 );
   }
   
   for ( i=0; i < n_child; i++ )
      if ( waitpid( childid[i], NULL, 0 ) < 0 )
         perror( "waitpid() failed.\n" );

   for ( i=0; i < n_child; i++ ) {
      sem_close( sem[ i ] );
      sem_unlink( sem_names[ i ] );
   }

   return 0;
}
