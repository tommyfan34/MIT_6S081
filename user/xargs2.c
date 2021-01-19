#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

void child(char *command, char *paramv[], int count) {
	char *bf, *p, *param;
	char *argv_temp[MAXARG];
	for (int i=0; i<count; i++) {
		strcpy(argv_temp[i], agrv[i]);
	}
	int argc_temp = count;
	p = param;
	int flag = 0;

	while (1) {
		if (read(0, bf, 1) == 0) {
			
		}
		// hits new line
		if (*bf == '\n') {
			*p = 0;
			if ((int pid = fork()) < 0) {
				fprintf(2, "ERROR: failed to fork\n");
			} else if (pid == 0) {
				child(command, paramv, count);
			} else {
				exec(command, argv_temp);
				wait((int *) 0);
				exit(0);
			}
		} else if (*bf == ' ') {
			// there is no arg in param
			if (flag == 0) {

			} else {
				++p;
				*p = 0;
				strcpy(argv_temp[argc_temp], param);
				flag = 0;
				memset(param, 0, p-param);
				p = param;
				argc_temp++;
			}
		}

		flag = 1;
		*p = *bf;
		++p;

	}
	// reaches end of input
	
	exit(0);
}

int main(int argc, char *argv[]) {
	char *command = argv[1];
	char *paramv[MAXARG];

	// copy the parameter of command
	for (int i=2; i<argc; i++) {
		strcpy(paramv[i-2], argv[i]);
	}
	
	if ((int pid = fork()) < 0) {
		fprintf(2, "ERROR: failed to fork\n");	
	} else if (pid == 0) {
		child(command, paramv, argc-2);
	} else {
		wait((int *) 0);
	}
}
