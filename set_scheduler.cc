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
#include <sys/sem.h>
#include <sys/types.h>  // 
#include <sys/wait.h>   // wait()

#define n_child 3

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

int init_vsem( char* pathtokey, int nsems ) { 
   key_t sem_key = ftok( pathtokey, 100 );   // 100 is proj_id
   int sem_flag = IPC_CREAT | 0666;
   int sem_id;
   sem_id = semget( sem_key, nsems, sem_flag );
   if ( sem_id < 0 ) { 
      std::cerr << "sem_get() failed!\n";
      std::cerr << "errno: " << strerror( errno ) << std::endl;
      return sem_id;
   } else {
      semctl( sem_id, 0, SETVAL, 1 ); // initialize the semaphore
      for ( int i=1; i < nsems; i++ ) { 
         semctl( sem_id, i, SETVAL, 0 ); // initialize the semaphore
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

   init_vsem(  "/home/h4bian/aqua10/sched_study/VSEM", n_child );
   
   int childid[ n_child ] = { 0 };
   for ( int i=0; i < n_child; i++ ){
      std::string pname =  std::to_string( i );
      std::cout << "PNAME: " << pname << std::endl;
      childid[ i ] = fork_and_exec( pname, cpuid );
      set_scheduler( childid[ i ], 99 );
   }
   
   for ( int i=0; i < n_child; i++ )
      if ( waitpid( childid[i], NULL, 0 ) < 0 )
         perror( "waitpid() failed.\n" );

   return 0;
}
