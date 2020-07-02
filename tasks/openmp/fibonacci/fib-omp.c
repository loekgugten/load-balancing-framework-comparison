#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double wctime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec + 1E-6 * tv.tv_usec);
}

__int64_t fibseq(__int64_t n) {
  if (n < 2) return n;
  int x, y;
  x = fibseq(n - 1);
  y = fibseq(n - 2);
  return x + y;
}

__int64_t fibomp(__int64_t n) {
  if (n < 2) return n;
  int x, y;
  #pragma omp task untied shared(x) firstprivate(n)
  x = fibomp(n - 1);
  y = fibomp(n - 2);
  #pragma omp taskwait
  return x + y;
}

__int64_t fibompcutoff(__int64_t n, __int64_t cutoff) {
  if (n < 2) return n;
  if (n < cutoff) return fibseq(n);
  int x, y;
  #pragma omp task untied shared(x) firstprivate(n)
  x = fibompcutoff(n - 1, cutoff);
  #pragma omp task untied shared(y) firstprivate(n)
  y = fibompcutoff(n - 2, cutoff);
  #pragma omp taskwait
  return x + y;
}

void usage() {
  fprintf(stderr, "\nUsage: fibonacci [-w <amount of workers>] [-c <cutoff>] <n>\n\n");
  fprintf(stderr, "Calculates the nth fibonacci number.\n");
}

int main(int argc, char **argv) {
  double t1, t2;
  __int64_t n = -1;
  __int64_t c = -1;

  int workers = 1;

  int i = 1;
  while (i < argc) {
    if (argv[i][0] == '-' && argv[i][1] == 'w') {
      workers = atoi(argv[i+1]);
      i++;
    } else if (argv[i][0] == '-' && argv[i][1] == 'c') {
      c = atoi(argv[i+1]);
      i++;
    } else {
      n = atoi(argv[i]);  
    }
    i++;
  }

  if (n == -1) {
    usage();
    return 0;
  }

  __int64_t r;
  if (c == -1) {
    // Without Cutoff
    t1 = wctime();
    #pragma omp parallel num_threads(workers) shared (n)
    {
      #pragma omp single
      {
        r = fibomp(n);
      }
    }
    t2 = wctime();
  } else {
    // With Cutoff
    printf("Running with cutoff...\n");
    t1 = wctime();
    #pragma omp parallel num_threads(workers) shared (n)
    {
      #pragma omp single
      {
        r = fibompcutoff(n, c);
      }
    }
    t2 = wctime();
  }

  if (c == -1) {
    printf("OpenMP Benchmark: Fib \n\nInput n: %ld \nCutoff: %ld \nOutcome: %ld \nTime: %f\n", n, c, r, t2-t1);
  } else {
    printf("OpenMP Benchmark: Fib \n\nInput n: %ld \nOutcome: %ld \nTime: %f\n", n, r, t2-t1);
  }
  return 0;
}
