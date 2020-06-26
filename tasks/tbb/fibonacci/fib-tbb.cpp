#include <sys/time.h>
#include <iostream>
#include <tbb/tbb.h>
#include "tbb/global_control.h"

namespace tbb {
    long fibseq(long n) {
        if (n < 2) return n;
        return fibseq(n-1) + fibseq(n-2);
    }
    
    class FibTask: public task {
    public:
        const long n;
        long* const sum;
        FibTask( long n_, long* sum_ ) :
            n(n_), sum(sum_)
        {}
        task* execute() {      // Overrides virtual function task::execute
            if( n<2 ) {
                *sum = n;
            } else {
                long x, y;
                FibTask& a = *new( allocate_child() ) FibTask(n-1,&x);
                FibTask& b = *new( allocate_child() ) FibTask(n-2,&y);
                // Set ref_count to 'two children plus one for the wait".
                set_ref_count(3);
                // Start b running.
                spawn( b );
                // Start a running and wait for all children (a and b).
                spawn_and_wait_for_all(a);
                // Do the sum
                *sum = x+y;
            }
            return NULL;
        }
    };

    class FibTaskCutoff: public task {
    public:
        const long n;
        long* const sum;
        const long c;
        FibTaskCutoff( long n_, long* sum_, long c_ ) :
            n(n_), sum(sum_), c(c_)
        {}
        task* execute() {      // Overrides virtual function task::execute
            if( n<2 ) {
                *sum = n;
            } else if (n<c) {
                *sum = fibseq(n);
            } else {
                long x, y;
                FibTaskCutoff& a = *new( allocate_child() ) FibTaskCutoff(n-1,&x,c);
                FibTaskCutoff& b = *new( allocate_child() ) FibTaskCutoff(n-2,&y,c);
                // Set ref_count to 'two children plus one for the wait".
                set_ref_count(3);
                // Start b running.
                spawn( b );
                // Start a running and wait for all children (a and b).
                spawn_and_wait_for_all(a);
                // Do the sum
                *sum = x+y;
            }
            return NULL;
        }
    };

    long ParallelFib(long n);

    long ParallelFib( long n ) {
        long sum;
        FibTask& a = *new(task::allocate_root()) FibTask(n,&sum);
        task::spawn_root_and_wait(a);
        return sum;
    }

    long ParallelFibCutoff(long n, long c);

    long ParallelFibCutoff(long n, long c) {
        long sum;
        FibTaskCutoff& a = *new(task::allocate_root()) FibTaskCutoff(n,&sum,c);
        task::spawn_root_and_wait(a);
        return sum;
    }

}

double wctime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec + 1E-6 * tv.tv_usec);
}

void usage(void) {
  fprintf(stderr, "\nUsage: fib [-w <amount of workers>] [-c (cutoff)] <n>\n\n");
  fprintf(stderr, "Calculates the nth fibonacci number.\n");
}

int main(int argc, char **argv) {
    int workers = 1;
    __int64_t n = -1;
    __int64_t c = -1;

    int i = 1;
    while (i < argc) {
        if (argv[i][0] == '-' && argv[i][1] == 'w') {
            workers = atoi(argv[i+1]);
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

    tbb::global_control co(tbb::global_control::max_allowed_parallelism, workers);

    double t1, t2;

    __int64_t r;
    if (c == -1) {
        // Without Cutoff
        t1 = wctime();
        r = tbb::ParallelFib(n);
        t2 = wctime();
    } else {
        // With Cutoff
        printf("Running with cutoff...\n");
        t1 = wctime();
        r = tbb::ParallelFibCutoff(n, c);
        t2 = wctime();
    }

    if (c == -1) {
        printf("TBB Benchmark: Fib \n\nInput n: %ld \nOutcome: %ld \nTime: %f\n", n, r, t2-t1);
    } else {
        printf("TBB Benchmark: Fib \n\nInput n: %ld \nCutoff: %ld \nOutcome: %ld \nTime: %f\n", n, c, r, t2-t1);
    }
    return 0;
}