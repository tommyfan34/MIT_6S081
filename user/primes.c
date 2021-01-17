#include "kernel/types.h"
#include "user/user.h"
#define PRIME_NUM 35
#define READEND 0
#define WRITEEND 1

int main(int argc, char *argv[]) {
	int p[2];
	pipe(p);
	if (fork() == 0) {
		child(p);	
	} else {
		close(p[READEND]);
		// feed the int array
		for (int i=0; i<PRIME_NUM-1; i++) {
			write(p[WRITEEND], i+2, sizeof(int));
		}
		close(p[WRITEEND]);
		wait();
	}
	exit(0);
}

void child(int* pl) {
	int pr[2];
	int n;

	close(pl[WRITEEND]);
	// tries to read the first number
	int read_result = read(pl[READEND], &n, sizeof(int));
	if (read_result == 0)
		exit(0);
	pipe(pr);

	if (fork() == 0) {
		child(pr);
	} else {
		close(pl[WRITEEND]);
		close(pr[READEND]);
		printf("prime %d\n", n);
		while (read(pl[READEND], &n, sizeof(int)) != 0) {
			if (n%prime != 0) {
				write(pr[WRITEEND], n, sizeof(int));
			}
		}
		close(pr[WRITEEND]);
		wait();
		exit(0);
	}
}
