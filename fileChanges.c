#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

void printOutEvent(int fd);

int main(){

	int inst;
	const char * fileName = "new.txt";
	FILE * fd, * newfd;
	int ret;

	//delete file
	if(remove(fileName) == -1)
		perror("remove");

	//create a file to watch
	fd = fopen(fileName, "w+");
	if(fd == NULL)
		perror("fopen");

	//create and inotify instance
	inst = inotify_init1(0);
	if(inst == -1)
		perror("inotify_init");
	//watch a file
	int watch;
	watch = inotify_add_watch(	inst,
					//fileName,
					"/home/peter/Documents/sysprgm/oreilly/chptr8/fileChange",
					//IN_ACCESS | IN_MODIFY | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE
					IN_ALL_EVENTS
					//IN_OPEN
				);
	if(watch == -1)
		perror("inotify add watch");

	//edit the file
	ret = fputs("sample text\n", fd);


	if(ret < 0)
		perror("fputs");
	fflush(fd);

	sleep(6);
	printOutEvent(inst);
	//close and re-open
	if(fclose(fd) == EOF)
		perror("fclose");

	newfd = fopen(fileName, "a");
	if(newfd == NULL)
		perror("fopen");
	fputs("more text\n", newfd);
	fclose(newfd);

	unsigned int len;
	ret = ioctl(watch, FIONREAD, &len);
	printf("%d bytes\n", len);

	printOutEvent(watch);

	printf("done\n");
	close(inst);

	exit(EXIT_SUCCESS);
}

void printOutEvent(int fd){
	unsigned int len;
	int ret;
	struct inotify_event evt;

	ret = ioctl(fd, FIONREAD, &len);
	printf("%d bytes of events to read\n", len);
	if(ret == -1)
		perror("ioctl");

	if(len !=0){
		char buff[len+1];
		ret = read(fd, buff, (size_t)len);
		//only print out the first event for now
		memcpy(&evt, buff, sizeof(evt));
		printf("read %d bytes", ret);
		printf("here 3\n");

		printf("mask = %d, cookie=%d, len=%d filename=%s\n", evt.mask, evt.cookie, evt.len, evt.name);
	}
}
