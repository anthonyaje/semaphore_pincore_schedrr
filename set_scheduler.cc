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
   std::string sname = "/SEM_CORE";
   sem_t* sem = sem_open ( sname.c_str(), O_CREAT, 0644, 1 );
   if ( sem == SEM_FAILED ) { 
      std::cerr << "sem_open failed!\n";
      return -1;
   }

   sem_init( sem, 0, 1 );
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
   std::string pnames[2] = { "p111", "p222" };

   init_semaphore();
   
   int childid[ 2 ] = { 0 };
   int i = 0;
   for( std::string pname : pnames ){
      childid[ i ] = fork_and_exec( pname, cpuid );
      set_scheduler( childid[ i++ ], 99 );
   }
   
   for ( i=0; i<2; i++ )
      if ( waitpid( childid[i], NULL, 0 ) < 0 )
         perror( "waitpid() failed.\n" );

   return 0;
}
