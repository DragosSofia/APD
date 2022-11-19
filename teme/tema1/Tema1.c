#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <pthread.h>

using namespace std;

struct my_arg {
	long id;
	int N;
	int P;
  int *nextFile;
  int nrOfFiles;
  pthread_mutex_t *mutex;
  vector<string> *files;
};

void get_args(int & N, int & P , char * startFile, char *argv[]){
  N = atoi(argv[1]);
	P = atoi(argv[2]);
  strcpy(startFile, argv[3]);
}


void *f(void *arg) {
  string file;
  struct my_arg* data = (struct my_arg*) arg;

  if( data -> id < data -> N){

    while( *(data -> nextFile) < data -> nrOfFiles){

      pthread_mutex_lock(data -> mutex);
      if(*(data -> nextFile) < data -> nrOfFiles){
        file = (*data -> files)[*(data -> nextFile)];
        *(data -> nextFile) += 1;
        pthread_mutex_unlock(data -> mutex);
      }else{
        pthread_mutex_unlock(data -> mutex);
        break;
      }

      cout << data -> id << " " << file << "\n";
      sleep(1);
    }
  }

  pthread_exit(NULL);
}

void setUpArgs(int N, int P, char *startFile, vector<string> &files, int& nrOfFiles){
  int n, i;
  string name;
  fstream fin(startFile);

  fin >> n;
  nrOfFiles = n;

  for(i = 0; i < n; i++){
    fin >> name;
    files.push_back(name);
  }
}


int main(int argc, char *argv[]){
  int N, P, nrOfFiles, r, nextFile = 0;
  char startFile[100];
  vector <string> files ;
  pthread_t *threads;
  pthread_mutex_t mutex;
  long id;
  void *status;
  struct my_arg *arguments;

  get_args(N, P, startFile, argv);
  setUpArgs(N, P, startFile, files, nrOfFiles);

  pthread_mutex_init(&mutex, NULL);

  threads = (pthread_t*) malloc(( N + P ) * sizeof(pthread_t));
  arguments = (struct my_arg*) malloc(( N + P ) * sizeof(struct my_arg));

  for (id = 0; id < N + P; id++) {
        arguments[id].id = id;
        arguments[id].N = N;
        arguments[id].P = P;
        arguments[id].nrOfFiles = nrOfFiles;
        arguments[id].files = &files;
        arguments[id].nextFile = &nextFile;
        arguments[id].mutex = &mutex;

        r = pthread_create(&threads[id], NULL, f, (void *) &arguments[id]);
        if (r) {
            printf("Eroare la crearea thread-ului %ld\n", id);
            exit(-1);
        }
  }

  for (id = 0; id < N + P ; id++) {
		r = pthread_join(threads[id], &status);

		if (r) {
			//printf("Eroare la asteptarea thread-ului %d\n", id);
			exit(-1);
		}
	}

  pthread_mutex_destroy(&mutex);
	free(threads);
	free(arguments);
  return 0;
}