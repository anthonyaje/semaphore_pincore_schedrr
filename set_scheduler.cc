// Author: Anthony Anthony.

#include <sched.h>      // SCHED_RR
#include <stdio.h>
#include <cstdlib>
#include <errno.h>      // errno
#include <iostream>     // cout cerr
#include <fcntl.h>      // O_CREAT
#include <unistd.h>     // fork
#include <stdlib.h>
#include <string.h>     // cpp string
#include <semaphore.h>
#include <sys/types.h>  // 
#include <sys/wait.h>   // wait()

#define n_child 3
sem_t* sm; 

int set_scheduler( int pid, int prio ) {
   int old_sched_policy = sched_getscheduler( pid );    
   struct sched_param param; 
   param.sched_priority = prio;
   int rc = sched_setscheduler( pid, SCHED_RR, &param );
   if (rc == -1) {                                               
      perror( "sched_setscheduler call is failed." );             
      exit (-1);
   }                                                             
   else {                                                        
      printf("Old Scheduler: %d\n", old_sched_policy);           
      printf("Current Scheduler: %d\n", sched_getscheduler( pid ));
      return 0;
   }                                                             

}

int init_semaphore( std::string sem_name, int val ){ 
   printf( "creating semaphore: %s\n", sem_name.c_str() );
   sm = sem_open ( sem_name.c_str(), O_CREAT, 0644, val );
   if ( sm == SEM_FAILED ) { 
      perror( "sem_open failed!" );
      return -1;
   }

   sem_init( sm, 0, val );
   return 0;
}

// Fork and exec rr-task. 
// Return pid of child
int fork_and_exec( std::string pname, char* cpuid ){
   int pid = fork();
   if ( pid == 0) {
      // Child
      std::string next_pname = std::to_string( ( stoi( pname ) + 1 ) % n_child );
      char* const params[] = { "./rr-task", "99", strdup( pname.c_str() ),
                               cpuid, strdup( next_pname.c_str() ) , NULL };
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
   int childid[ n_child ] = { 0 };
   
   for ( int i=0; i < n_child; i++ ) {
      std::string sem_name = "/SEM" + std::to_string( i );
      if ( i == 0 )
         init_semaphore( sem_name, 1 );
      else
         init_semaphore( sem_name, 0 );
   }

   for ( int i=0; i < n_child; i++ ){
      std::string pname =  std::to_string( i );
      std::cout << "PNAME: " << pname << std::endl;
      
      childid[ i ] = fork_and_exec( pname, cpuid );
      //set_scheduler( childid[ i ], 99 );
   }
   
   for ( int i=0; i < n_child; i++ )
      if ( waitpid( childid[i], NULL, 0 ) < 0 )
         perror( "waitpid() failed.\n" );

   return 0;
}
