#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

class arg_var
{
	public:
	int i;
    int nbthreads;
};

void * printFunct(void* t)
{
	arg_var* tempVar=(arg_var *)t;
	std::cout<<"I am Thread "<<tempVar->i <<" In Nthread "<<tempVar->nbthreads<<"\n";
	return NULL; 
}


int main (int argc, char* argv[]) {

  if (argc < 2) {
    std::cerr<<"usage: "<<argv[0]<<"<nbthreads>"<<std::endl;
    return -1;
  }

	int nbthreads = atoi(argv[1]);
	pthread_t threads[nbthreads];
	arg_var t[nbthreads];
	
	for(int i=0;i<nbthreads;i++)
	{
		t[i].i=i;
		t[i].nbthreads=nbthreads;
		pthread_create(&threads[i],NULL, printFunct,(void*)&t[i]);
	}
	
	for(int i=0;i<nbthreads;i++)
	{
		pthread_join(threads[i],NULL);
	}
  
  return 0;
}