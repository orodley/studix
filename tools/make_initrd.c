// Create a simple initial ramdisk for the kernel

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "initrd.h"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

bool write_file(int out_fd, int in_fd, char *name, size_t start, size_t length)
{
	File_header header;
	strcpy(header.name, name);
	header.start  = start;
	header.length = length;

	write(out_fd, &header, sizeof(File_header));

	// After we've written the actual file data, we need to return to the
	// position of the next header
	off_t return_to = lseek(out_fd, 0, SEEK_CUR);

	lseek(out_fd, start, SEEK_SET);

	char *buf = malloc(length);
	read(in_fd, buf, length);
	write(out_fd, buf, length);
	free(buf);

	lseek(out_fd, return_to, SEEK_SET);

	return true;
}

int write_initrd(char *source_dir, char *output_file)
{
	DIR *rd_dir = opendir(source_dir);
	if (rd_dir == NULL) {
		perror("Couldn't open directory");
		return 1;
	}

	int output_fd = creat(output_file, O_WRONLY | FILE_MODE);
	if (output_fd == -1) {
		perror("Coudn't open output file");
		return 1;
	}

	// Skip past the first byte, which stores the number of files
	lseek(output_fd, 1, SEEK_SET);

	chdir(source_dir);

	struct stat    info;
	struct dirent *dir_entry;
	size_t end       = 1 + (sizeof(File_header) * MAX_FILES);
	size_t num_files = 0;

	while ((dir_entry = readdir(rd_dir)) != NULL) {
		char *name = dir_entry->d_name;

		int fd = open(name, O_RDONLY);
		if (fd == -1) {
			perror("Couldn't open file");
			return 1;
		}

		int err = fstat(fd, &info);
		if (err == -1) {
			perror("Couldn't stat file");
			close(fd);
			return 1;
		}

		if (!S_ISREG(info.st_mode)) {
			close(fd);
			continue;
		}

		printf("Adding file '%s' to ramdisk\n", name);
		write_file(output_fd, fd, name, end, info.st_size);

		close(fd);
		end += info.st_size;
		num_files++;
	}

	free(rd_dir);
	free(dir_entry);

	lseek(output_fd, 0, SEEK_SET);
	write(output_fd, &num_files, 1);
	close(output_fd);

	printf("Done! Created ramdisk image '%s', with %d file(s), and size %uB\n",
			output_file, num_files, end);
	return 0;
}

const char USAGE_FMT[] = "Usage: %s DIRECTORY RAMDISK\n";

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, USAGE_FMT, argv[0]);
		return 1;
	}

	return write_initrd(argv[1], argv[2]);
}
