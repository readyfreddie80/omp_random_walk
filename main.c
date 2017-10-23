#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <assert.h>
#include<time.h>

typedef struct scalar_ctx_t {
	int a;
	int b;
	int x;
	int N;
	double p;
} scalar_ctx_t;

void random_walk(void *context, FILE *f)
{
	scalar_ctx_t *ctx = context;
	int right_end_counter = 0; // points that got to right end
	int walking_time = 0; // counter of point's steps
	int * seed = (int*) malloc(ctx->N * sizeof(int));
	int s = (int)(time(NULL));
	for(int i = 0; i < ctx->N; i++) {
		seed[i] = rand_r(&s);
	}

	struct timeval start, end;
	assert(gettimeofday(&start, NULL) == 0);
	#pragma omp parallel for reduction (+: walking_time)
	for (int i = 0; i < ctx->N; i++) {
		int x = ctx->x;
		while (x != ctx->a && x != ctx->b) {
			walking_time++;
			double p = (double)rand_r(&seed[i]) / RAND_MAX;
			if (p <= ctx->p) {
				x++;
			} else {
				x--;
			}
       		 }
        	if(x == ctx->b) {
			#pragma omp atomic
            		right_end_counter++;
        	}
	}
	assert(gettimeofday(&end, NULL) == 0);
	double delta = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
	double p = (double)right_end_counter / ctx->N;
	double mean_time = (double)walking_time / ctx->N;
	fprintf(f, "%f %f %fs %d %d %d %d %f \n", p, mean_time, delta, ctx->a, ctx->b, ctx->x, ctx->N, ctx->p);
	free(seed);
}

int main(int argc, char **argv) {
	if( argc == 7 ) {
		int a = atoi(argv[1]);
		int b = atoi(argv[2]);
		int x = atoi(argv[3]);
		int N = atoi(argv[4]);
		double p = atof(argv[5]);
		int P = atoi(argv[6]);

		scalar_ctx_t ctx = {
			.a = a,
			.b = b,
			.x = x,
			.N = N,
			.p = p,
		};
		
		if(a >= b || N == 0 || x < a || x > b || P == 0 || p > 1 || p < 0)
			return 0;

		omp_set_num_threads(P);

		FILE *f = fopen("stats.txt", "w");
		if (f != NULL) {
			random_walk(&ctx, f);
		}
		fclose(f);
	}
	return 0;
}
