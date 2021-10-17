#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>
#include <chrono>
#include <cmath>

using namespace std;

using seconds = chrono::seconds;
using check_time = std::chrono::high_resolution_clock;

struct arg_structure
{
  string sync;
  float a, b, mid;
  int start, end, funct_id, intensity, n;
};

pthread_mutex_t mut, comp_mut;
float total_sum = 0.0, mid = 0.0;
int rem_comp = 0, n, global_begin = 0, global_end = 0, granularity = 0;

#ifdef __cplusplus
extern "C"
{
#endif

  float f1(float x, int intensity);
  float f2(float x, int intensity);
  float f3(float x, int intensity);
  float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

bool done()
{
  pthread_mutex_lock(&comp_mut);

  if (rem_comp == 0)
  {
    pthread_mutex_unlock(&comp_mut);
    return true;
  }
  else
  {
    if (rem_comp < granularity)
    {
      rem_comp = 0;
    }
    else
    {
      rem_comp = rem_comp - granularity;
    }
    pthread_mutex_unlock(&comp_mut);
    return false;
  }
}

void getnext(int *start, int *end)
{
  pthread_mutex_lock(&comp_mut);

  *start = global_begin;

  *end = global_end;

  if (granularity <= (n - global_end))
  {
    global_begin = *end;
    global_end = global_begin + granularity;
  }

  pthread_mutex_unlock(&comp_mut);
}

void *iteration_level_integration(void *p)
{

  int start, end;
  while (!done())
  {
    arg_structure *obj_struct = (arg_structure *)p;

    getnext(&start, &end);

    for (int i = start; i < end; i++)
    {

      float x = (obj_struct->a + (i + 0.5) * obj_struct->mid);

      switch (obj_struct->funct_id)
      {
      case 1:
        pthread_mutex_lock(&mut);
        total_sum += (f1(x, obj_struct->intensity) * obj_struct->mid);

        pthread_mutex_unlock(&mut);
        break;

      case 2:
        pthread_mutex_lock(&mut);
        total_sum += (f2(x, obj_struct->intensity) * obj_struct->mid);

        pthread_mutex_unlock(&mut);
        break;

      case 3:
        pthread_mutex_lock(&mut);
        total_sum += (f3(x, obj_struct->intensity) * obj_struct->mid);

        pthread_mutex_unlock(&mut);
        break;

      case 4:
        pthread_mutex_lock(&mut);
        total_sum += (f4(x, obj_struct->intensity) * obj_struct->mid);

        pthread_mutex_unlock(&mut);
        break;
      }
    }
  }
  pthread_exit(NULL);
}

float getsum(void *p, int i)
{

  arg_structure *obj_struct = (arg_structure *)p;
  float x = (obj_struct->a + (i + 0.5) * obj_struct->mid);

  float sum = 0.0;

  switch (obj_struct->funct_id)
  {
  case 1:
    sum = (f1(x, obj_struct->intensity));
    break;

  case 2:
    sum = (f2(x, obj_struct->intensity));
    break;

  case 3:
    sum = (f3(x, obj_struct->intensity));
    break;

  case 4:
    sum = (f4(x, obj_struct->intensity));
    break;
  }

  return sum;
}

void *chunk_level_integration(void *p)
{

  while (!done())
  {

    int start, end;
    float sum = 0.0;
    getnext(&start, &end);
    for (int i = start; i < end; i++)
    {
      sum += getsum(p, i);
    }
    pthread_mutex_lock(&mut);

    total_sum += sum;
    pthread_mutex_unlock(&mut);
  }
  pthread_exit(NULL);
}

void *thread_level_integration(void *p)
{

  float sum = 0.0;
  int start, end;
  while (!done())
  {
    getnext(&start, &end);
    for (int i = start; i < end; i++)
    {
      sum += getsum(p, i);
    }
  }

  pthread_mutex_lock(&mut);
  total_sum = total_sum + sum;
  pthread_mutex_unlock(&mut);

  pthread_exit(NULL);
}

struct arg_structure getobjstruct(float a, float b, int funct_id, int intensity, float mid, string sync, int n, int nbthreads, int i)
{
  struct arg_structure obj;
  obj.a = a;
  obj.b = b;
  obj.funct_id = funct_id;
  obj.intensity = intensity;
  obj.mid = mid;
  obj.sync = sync;
  obj.n = n;

  return obj;
}

int main(int argc, char *argv[])
{
  std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

  if (argc < 9)
  {
    std::cerr << "usage: " << argv[0] << " <funct_id> <a> <b> <n> <intensity> <nbthreads> <sync> <granularity>" << std::endl;
    return -1;
  }
  int funct_id = atoi(argv[1]);
  float a = atof(argv[2]);
  float b = atof(argv[3]);
  n = atoi(argv[4]);
  int intensity = atoi(argv[5]);
  float mid = ((b - a) / n);
  int nbthreads = atoi(argv[6]);
  string sync = argv[7];
  granularity = atoi(argv[8]);
  global_end = granularity;

  struct arg_structure obj_struct[nbthreads];
  pthread_t thread[nbthreads];
  rem_comp = n;

  pthread_mutex_init(&mut, NULL);
  pthread_mutex_init(&comp_mut, NULL);

  if (0 == sync.compare("iteration"))
  {
    for (int i = 0; i < nbthreads; i++)
    {
      obj_struct[i] = getobjstruct(a, b, funct_id, intensity, mid, sync, n, nbthreads, i);
      pthread_create(&thread[i], NULL, iteration_level_integration, (void *)&obj_struct[i]);
    }
  }
  else if (0 == sync.compare("thread"))
  {
    for (int i = 0; i < nbthreads; i++)
    {
      obj_struct[i] = getobjstruct(a, b, funct_id, intensity, mid, sync, n, nbthreads, i);
      pthread_create(&thread[i], NULL, thread_level_integration, (void *)&obj_struct[i]);
    }
  }
  else if (0 == sync.compare("chunk"))
  {
    for (int i = 0; i < nbthreads; i++)
    {
      obj_struct[i] = getobjstruct(a, b, funct_id, intensity, mid, sync, n, nbthreads, i);
      pthread_create(&thread[i], NULL, chunk_level_integration, (void *)&obj_struct[i]);
    }
  }

  for (int i = 0; i < nbthreads; i++)
  {
    pthread_join(thread[i], NULL);
  }

  pthread_mutex_destroy(&mut);
  pthread_mutex_destroy(&comp_mut);

  cout << total_sum * mid;
  ;

  std::chrono::time_point<std::chrono::system_clock> end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end_time - start_time;

  std::cerr << elapsed_seconds.count() << std::endl;

  return 0;
}