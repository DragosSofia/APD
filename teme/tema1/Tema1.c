#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <pthread.h>

using namespace std;

struct my_arg {
	long id;
	int N;
	int P;
  int *nextFile;
  int nrOfFiles;
  pthread_mutex_t *mutex;
  pthread_barrier_t *barrier;
  vector<vector<vector<long long>>> *elem;
  vector<string> *files;
};

void get_args(int & N, int & P , char * startFile, char *argv[]){
  N = atoi(argv[1]);
	P = atoi(argv[2]);
  strcpy(startFile, argv[3]);
}

bool isXpow(long long X, long long pow){
  long long l = 2, r = sqrt(X), m;
  long long i, prod;

  if(X < 0){
    return false;
  }

  if( X == 1 ){
    return true;
  }

  while( l <= r ){
    m = (l + r) / 2;
    prod = 1;
    for( i = 1 ; i <= pow ; i++ ){
      prod *= m;
    }
    if(prod == X){
      return true;
    }
    if(prod < X){
      l = m + 1;
    }
    else{
      r = m -1;
    }
  }
  return false;

}


void *f(void *arg) {
  string file;
  long long n, i, j;
  long long X, pow;
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

      fstream fin(file);
      fin >> n;
      for(i = 0; i < n; i++){
        fin >> X;
        for(pow = 2 ; pow <= data -> P + 1; pow++){
          if(isXpow(X, pow)){
            (*data -> elem)[data -> id][pow - 2].push_back(X);
          }
        }
      }
    }
  }
  pthread_barrier_wait(data -> barrier);




  if( data -> id >= data -> N){
    int i,j;
    set<long long> uniq;
    int pow = data -> id -  data -> N ;

    for(i = 0; i < data -> N; i++){
      for(j = 0 ; j <  (*data -> elem)[i][pow].size(); j++){
        uniq.insert((*data -> elem)[i][pow][j]);
      }
    }
    string name = "out" + to_string(data -> id - data -> N + 2 )+ ".txt";
    ofstream fout(name);
    fout<< uniq.size();
  }


  pthread_exit(NULL);
}

void setUpArgs(int N, int P, char *startFile, vector<string> &files, int& nrOfFiles, vector<vector<vector<long long>>> &elem){
  int n, i, j;
  string name;
  fstream fin(startFile);

  fin >> n;
  nrOfFiles = n;

  for(i = 0; i < n; i++){
    fin >> name;
    files.push_back(name);
  }

  for(i = 0; i < N; i++){
    vector<vector<long long>> forIMapper;
    elem.push_back(forIMapper);
    for(j = 0; j < P; j++){
      vector<long long> forPowJ;
      elem[i].push_back(forPowJ);
    }
  }


}


int main(int argc, char *argv[]){
  int N, P, nrOfFiles, r, nextFile = 0;
  char startFile[100];
  long id;
  void *status;
  struct my_arg *arguments;

  pthread_t *threads;
  pthread_mutex_t mutex;
  pthread_barrier_t barrier;

  vector<vector<vector<long long>>> elem;
  vector <string> files ;

  get_args(N, P, startFile, argv);
  setUpArgs(N, P, startFile, files, nrOfFiles, elem);

  pthread_barrier_init(&barrier, NULL, N + P);
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
        arguments[id].barrier = &barrier;
        arguments[id].elem = &elem;

        r = pthread_create(&threads[id], NULL, f, (void *) &arguments[id]);
        if (r) {
            printf("Eroare la crearea thread-ului %ld\n", id);
            exit(-1);
        }
  }

  for (id = 0; id < N + P ; id++) {
		r = pthread_join(threads[id], &status);

		if (r) {
			exit(-1);
		}
	}

  // int I,J,K;
  //   for(I = 0 ; I < N ; I++){
  //     for(J = 0 ; J < P ; J++){
  //       cout << "Pow " << J + 2 << "\n";
  //       for( K = 0 ; K < elem[I][J].size(); K++){
  //         cout <<elem[I][J][K] << " ";
  //       }
  //       cout << "\n";
  //     }
  //   }

  pthread_barrier_destroy(&barrier);
  pthread_mutex_destroy(&mutex);

	free(threads);
	free(arguments);

  return 0;
}