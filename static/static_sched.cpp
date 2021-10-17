#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <chrono>
#include <math.h>
#include <stdlib.h>
#include <string.h>

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

float globalResult = 0;
float globalXVal = 0, globalXInt;
pthread_mutex_t mut_result;

struct arguments
{
	float a, b;
	unsigned long n, start, end;
	float result, x_val = 0, x_int;
	int intensity, func;
};

float getSum(void *argument, float i) {
	struct arguments *arg = (struct arguments *)argument;
	globalXInt = arg->a + ((float)i + 0.5) * ((arg->b - arg->a) / (float)arg->n);
	float returnResult = 0.0;

	switch (arg->func) {
		case 1: {
			globalXVal = f1(globalXInt, arg->intensity);
			break;
		}
		case 2: {
			globalXVal = f2(globalXInt, arg->intensity);
			break;
		}
		case 3: {
			globalXVal = f3(globalXInt, arg->intensity);
			break;
		}
		case 4: {
			globalXVal = f4(globalXInt, arg->intensity);
			break;
		}
		default:
			std::cout << "\nWrong function id!!" << std::endl;
	}
	returnResult = globalXVal * ((arg->b - arg->a) / (float)arg->n);
	return returnResult;
}

void *threadLevelInt(void *argument)
{
	struct arguments *arg = (struct arguments *)argument;

	for (unsigned long i = arg->start; i < arg->end; i++)
	{
		arg->result += getSum(argument, (float)i);
	}
	pthread_exit(NULL);
}

void *iterationLevelInt(void *argument)
{
	struct arguments *arg = (struct arguments *)argument;

	for (unsigned long i = arg->start; i < arg->end; i++)
	{
		pthread_mutex_lock(&mut_result);
		globalResult += getSum(argument, (float)i);
		pthread_mutex_unlock(&mut_result);
	}

	pthread_exit(NULL);
}

struct arguments getArgArr(float a, float b, unsigned long start, unsigned long end, int intensity, int func, float n) {
  	struct arguments obj;
	obj.a = a;
	obj.b = b;
	obj.start = start;
	obj.end = end;
	obj.intensity = intensity;
	obj.func = func;
	obj.n = n;

	return obj;
}

int main(int argc, char *argv[])
{

	float final_result = 0, x_val = 0, x_int;
	float a, b;
	unsigned long n;
	double cpu_time;
	int func, intensity, nbthreads;
	char *sync;
	func = atoi(argv[1]);
	a = atof(argv[2]);
	b = atof(argv[3]);
	n = atof(argv[4]);
	intensity = atoi(argv[5]);
	nbthreads = atoi(argv[6]);
	sync = argv[7];

	pthread_t *threads;

	struct arguments *arg;

	threads = (pthread_t *)malloc(nbthreads * sizeof(pthread_t));
	arg = (struct arguments *)malloc(nbthreads * sizeof(struct arguments));

	pthread_mutex_init(&mut_result, NULL);

	auto clock_start = std::chrono::system_clock::now();

	if (strcmp(sync, "thread") == 0)
	{
		for (int j = 0; j < nbthreads; j++)
		{
			arg[j] = getArgArr(a, b, j * (n / nbthreads), j * (n / nbthreads)+(n / nbthreads), intensity, func, n);
			pthread_create(&threads[j], NULL, threadLevelInt, (void *)&(arg[j]));
		}
		for (int i = 0; i < nbthreads; i++)
		{
			pthread_join(threads[i], NULL);
		}
		for (int k = 0; k < nbthreads; k++)
		{
			globalResult += arg[k].result;
		}
	}
	else
	{
		for (int j = 0; j < nbthreads; j++)
		{
			arg[j] = getArgArr(a, b, j * (n / nbthreads), j * (n / nbthreads)+(n / nbthreads), intensity, func, n);
			pthread_create(&threads[j], NULL, iterationLevelInt, (void *)&(arg[j]));
		}

		for (int j = 0; j < nbthreads; j++)
		{
			pthread_join(threads[j], NULL);
		}
	}

	auto clock_end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = clock_end - clock_start;
	std::cout << globalResult;
	std::cerr << diff.count();

	return 0;
}
