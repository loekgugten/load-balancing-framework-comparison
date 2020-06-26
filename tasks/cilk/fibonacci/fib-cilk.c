#include <stdio.h>
#include <stdlib.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <sys/time.h>

__int64_t fibcilk(__int64_t n);

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

__int64_t fibcilk(__int64_t n) {
  if (n < 2) return n;
  int x, y;
  x = cilk_spawn fibcilk(n - 1);
  y = fibcilk(n - 2);
  cilk_sync;
  return x + y;
}

__int64_t fibcilkcutoff(__int64_t n, __int64_t cutoff) {
  if (n < 2) return n;
  if (n < cutoff) return fibseq(n);
  int x, y;
  x = cilk_spawn fibcilkcutoff(n - 1, cutoff);
  y = fibcilkcutoff(n - 2, cutoff);
  cilk_sync;
  return x + y;
}

void usage(void) {
  fprintf(stderr, "\nUsage: fib [-w <amount of workers>] [-c (cutoff)] <n>\n\n");
  fprintf(stderr, "Calculates the nth fibonacci number.\n");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: fib [-w <amount of workers>] [-c <cutoff>] <fibonacci number>\n");
    return 0;
  }
  
  __int64_t n = -1;
  __int64_t c = -1;
  char* workers = "1";

  int i = 1;
  while (i < argc) {
    if (argv[i][0] == '-' && argv[i][1] == 'w') {
      workers = argv[i+1];
      i++;
    } else if (argv[i][0] == '-' && argv[i][1] == 'c') {
      c = atoi(argv[i+1]);
    } else {
      n = atoi(argv[i]);  
    }
    i++;
  }

  if (n == -1) {
    usage();
    return 0;
  }

  __cilkrts_end_cilk();
  __cilkrts_set_param("nworkers", workers);

  double t1, t2;
  __int64_t r;
  if (c == -1) {
    // Without Cutoff
    t1 = wctime();
    r = fibcilk(n);
    t2 = wctime();
  } else {
    // With Cutoff
    printf("Running with cutoff...\n");
    t1 = wctime();
    r = fibcilkcutoff(n, c);
    t2 = wctime();
  }

  if (c == -1) {
    printf("Cilk Benchmark: Fib \n\nInput n: %ld \nOutcome: %ld \nTime: %f\n", n, r, t2-t1);
  } else {
    printf("Cilk Benchmark: Fib \n\nInput n: %ld \nCutoff: %ld \nOutcome: %ld \nTime: %f\n", n, c, r, t2-t1);
  }
  return 0;
}