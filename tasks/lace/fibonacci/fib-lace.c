#include "lace.h"
#include <stdio.h> 
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

double wctime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec + 1E-6 * tv.tv_usec);
}

int fibseq(int n) {
  if (n < 2) return n;
  int i, j;
  i = fibseq(n-1);
  j = fibseq(n-2);
  return i+j;
}

TASK_1(int, fiblace, int, n)
{
  if (n < 2) return n;
  int i, j;
  SPAWN(fiblace, n-1);
  j = CALL(fiblace, n-2);
  i = SYNC(fiblace);
  return i+j;
}

TASK_2(int, fiblacecutoff, int, n, int, c)
{
  if (n < 2) return n;
  if (n < c) return fibseq(n);
  int i, j;
  SPAWN(fiblacecutoff, n-1, c);
  j = CALL(fiblacecutoff, n-2, c);
  i = SYNC(fiblacecutoff);
  return i+j;
}

void usage(char *s)
{
    fprintf(stderr, "%s -w <workers> [-q dqsize] [-c cutoff] <n>\n", s);
}

int main(int argc, char **argv) {
  int workers = 1;
  int dqsize = 100000;
  int cutoff = -1;

  char c;
  while ((c=getopt(argc, argv, "w:q:c:h")) != -1) {
    switch (c) {
      case 'w':
        workers = atoi(optarg);
        break;
      case 'q':
        dqsize = atoi(optarg);
        break;
      case 'c':
        cutoff = atoi(optarg);
        break;
      case 'h':
        usage(argv[0]);
        break;
      default:
        abort();
    }
  }

  if (optind == argc) {
    usage(argv[0]);
    exit(1);
  }

  lace_init(workers, dqsize);
  lace_startup(0, 0, 0);

  LACE_ME;

  int n = atoi(argv[optind]);
  double t1, t2;
  int r;

  if (cutoff == -1) {
    t1 = wctime();
    r = CALL(fiblace, n);
    t2 = wctime();
  } else {
    t1 = wctime();
    r = CALL(fiblacecutoff, n, cutoff);
    t2 = wctime();
  }

  if (cutoff == -1) {
    printf("Cilk Benchmark: Fib \n\nInput n: %i \nOutcome: %i \nTime: %f\n", n, r, t2-t1);
  } else {
    printf("Cilk Benchmark: Fib \n\nInput n: %i \nCutoff: %i \nOutcome: %i \nTime: %f\n", n, cutoff, r, t2-t1);
  }
  return 0;
}