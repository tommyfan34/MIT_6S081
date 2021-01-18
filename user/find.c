#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"

void find(char *path, char *target_file);

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(2, "ERROR: You need pass in only 2 arguments\n");
		exit(1);
	}
	char *target_path = argv[1];
	char *target_file = argv[2];
	find(target_path, target_file);
	exit(0);
}

void find(char *path, char *target_file) {
	int fd;
	struct stat st;
	struct dirent de;
	char buf[512], *p;

	if ((fd = open(path, 0)) < 0) {
		fprintf(2, "ERROR: cannot open %s\n", path);
		return;
	}
	
	if (fstat(fd, &st) < 0) {
		fprintf(2, "ERROR: cannot stat %s\n", path);
		close(fd);
		return;
	}


	while (read(fd, &de, sizeof(de)) == sizeof(de)) {
		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		if (de.inum == 0)
			continue;
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;

		if (stat(buf, &st) < 0) {
			fprintf(2, "ERROR: cannot stat %s\n", buf);
		}

		switch (st.type) {
		case T_FILE:
			if (strcmp(target_file, de.name) == 0) {
				printf("%s\n", buf);
			}
			break;
		case T_DIR:
			// get the full path name of the current directory selected
			if ((strcmp(de.name, ".") != 0) && (strcmp(de.name, "..") != 0)) {
				find(buf, target_file);
}
		}	
	}
	close(fd);
	return;
}
