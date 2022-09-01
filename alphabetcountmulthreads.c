/*
 * alphabetcountmulthreads.c - this file implements the alphabetcountmulthreads function.
 */

#include <stdio.h> 
#include <dirent.h> 
#include "count.h"
#include <pthread.h>


//universal lock to guarentee no invalid accesses
pthread_mutex_t lock;





//check if file in some directory is a text file
int isTextFile(char* file_name) {
  int i = 0;
  while (file_name[i] != '\0' && file_name[i] != '.') {i+=1;}
  if (file_name[i] == '.') {
    if (file_name[i+1] != 't')   {return 0;}
    if (file_name[i+2] != 'x') {return 0;}
    if (file_name[i+3] != 't') {return 0;}
    return 1;
  }
  else {
    return 0;
  }
}

//struct used to pas arguments to each thread
struct arguments_struct{
  char (*files)[500];
  int start;
  int end;
  long *alphabetfreq;
};







int count_number_of_files(char* path) {

  int number_of_files =0;
  DIR* dir = opendir(path);
  struct dirent *dr;
  while ((dr = readdir(dir)) != NULL) {
    if(isTextFile(dr->d_name)) {
      number_of_files += 1;
    }
  }
  return number_of_files;
}



void* alphabetlettercount(void *arguments) {
  //pull our our argument struct
  struct arguments_struct *args = arguments;

  //print corresponding thread as required
  printf("Thread id = %d starts processing files with index from %d to %d!\n", (int) pthread_self(), args->start, args->end);

  //look only at file names defined for this particular thread
  for (int i = args->start; i < args->end; i++) {
    //get the file
    FILE* file = fopen(args->files[i],"r");
    int c;
    //print file this thread is looking at as required
    printf("Thread id = %d is processing file %s\n",(int) pthread_self() ,args->files[i] );
    //check the ascii location of each character to decide where it should go
    while ((c = getc(file)) != EOF ) {
      if (c < 97) {
        c+= 32;
      }
      if (c >= 97 && c <= 122) {
          //lock our thread to add to alphabetfrex
          pthread_mutex_lock(&lock);
          args->alphabetfreq[c-'a'] += 1;
          pthread_mutex_unlock(&lock);
          //unlock after completion

      }
    }
   }
  //print thread id as required
  printf("Thread id = %d is done\n", (int) pthread_self());

return NULL;
 
    
}

/**
  The alphabetcountmulthreads function counts the frequency of each alphabet letter (a-z, case insensitive) in all the .txt files under
  directory of the given path and write the results to a file named as filetowrite. Different with programming assignment#0, you need to implement the program using mutithreading.
  
  Input: 
      path - a pointer to a char string [a character array] specifying the path of the directory; and
      filetowrite - a pointer to a char string [a character array] specifying the file where results should be written in.
      alphabetfreq - a pointer to a long array storing the frequency of each alphabet letter from a - z, which should be already up-to-date;
      num_threads - number of the threads running in parallel to process the files
      
       
  Output: a new file named as filetowrite with the frequency of each alphabet letter written in
  
  Requirements:
1)	Multiple threads are expected to run in parallel to share the workload, i.e. suppose 3 threads to process 30 files, then each thread should process 10 files;
2)	When a thread is created, a message should be print out showing which files this thread will process, for example:
Thread id = 274237184 starts processing files with index from 0 to 10!
3)	When a file is being processed, a message should be print out showing which thread (thread_id = xxx) is processing this file, for example:
Thread id = 265844480 is processing file input_11.txt
4)	When a thread is done with its workload, a message should be print out showing which files this thread has done with work, for example:
      Thread id = 274237184 is done !
5)	The array: long alphabetfreq[ ] should always be up-to-date, i.e. it always has the result of all the threads counted so far.  [You may need to use mutexes to protect this critical region.]


You should have the screen printing should be similar as follows:

 Thread id = 274237184 starts processing files with index from 0 to 10!
 Thread id = 265844480 starts processing files with index from 11 to 22!
 Thread id = 257451776 starts processing files with index from 23 to 31!

 Thread id = 265844480 is processing file input_11.txt
 Thread id = 257451776 is processing file input_22.txt
 Thread id = 274237184 is processing file input_00.txt
  … … 

 Thread id = 274237184 is done !
 Thread id = 265844480 is done !
 Thread id = 257451776 is done !

 The results are counted as follows:
 a -> 2973036
 b -> 556908
 c -> 765864
 …  … 
*/




void alphabetcountmulthreads(char *path, char *filetowrite, long alphabetfreq[], int num_threads)
{



  //find the number of files we are going to split among the threads
	int number_of_files = count_number_of_files(path);

  //get files and save them into a two dim array
  DIR* dir = opendir(path);
  //files array we will use for each file path
  char files [number_of_files][500];
  struct dirent *dr;
  int k = 0;
  //go through each file
  while (k < number_of_files) {
    dr =  readdir(dir);
    //read the file and only add if addable
    if (isTextFile(dr->d_name)) {
      int i =0 ;
      //str copy file path
      while (path[i] != '\0') {
        files[k][i] = path[i];
        i += 1;
      }
      files[k][i] = '/';
      int j = i+1;
      i = 0;
      //string copy file name
      while (dr->d_name[i] != '\0') {
         files[k][j] = dr->d_name[i];
         i += 1;
         j+=1;
      }
      //null terminate file
      files[k][j]= '\0';
      k+=1;
    }
  }

  //get the ratio to find out how many files each individual thread will handle
  int number_of_files_per_thread = number_of_files/num_threads;


  //create specific argument structure used by each individual thread
  struct arguments_struct arguments[num_threads];
  for (int i = 0; i < num_threads; i++) {
    struct arguments_struct a;
    a.start = (i) * number_of_files_per_thread;
    a.end = (i + 1) * number_of_files_per_thread;
    a.alphabetfreq = alphabetfreq;
    a.files = files;
    arguments[i] = a;
  }

  //add remaining operations to last thread if they don't match exactly that is we can't evenly splt
  arguments[num_threads-1].end  += number_of_files % num_threads;
  




  //create our threads
  pthread_t tid[num_threads];

  //initial our mutex lock
  pthread_mutex_init(&lock, NULL);
  
  //create each thread  and run specific operation
  for (int i =0; i < num_threads; i++) {
       pthread_create(&(tid[i]), NULL, &alphabetlettercount, (void *) &(arguments[i]));
  }

  //end each thread
  for (int i =0; i < num_threads; i++) {
        pthread_join(tid[i], NULL);
  }

  //write info to output file
  FILE* write_file = fopen(filetowrite, "w");
  for (int k = 0; k < ALPHABETSIZE; k++) {
     fprintf(write_file, "%c -> %ld\n",'a' + k, alphabetfreq[k]);
  }  

	
}
