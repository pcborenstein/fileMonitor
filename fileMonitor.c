/***************
  fileMonitor.c
  by PCB

  This program takes a relative or absolute path
  and monitors it for changes
  The program ends if no changes occur for 10 seconds
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <unistd.h>

#define TIME_OUT_MS 10000

void processEvents(int fd);
void printEvents(uint32_t mask);
int watchFile(const char * pathname);
void waitForUpdates(int infd);

int main(int argc, char * argv[]){

	if(argc != 2){
		fprintf(stderr, "useage: %s [FILENAME or DIRECTORY]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int infd;
	infd = watchFile(argv[1]);
	waitForUpdates(infd);

	close(infd);
	printf("\ndone\n");
	exit(EXIT_SUCCESS);
}

/********************
  @fn waitForUpdates

  The program gets blocked here until an inotify event occurs
  It will retun if nothing happens for 10 seconds
 */

void waitForUpdates(int infd){
	int epfd;
	int ret;
	epfd = epoll_create1(0);
	struct epoll_event ev;
	ev.data.fd = infd;
	ev.events = EPOLLIN;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &ev);
	if(ret == -1)
		perror("epoll_ctl");
	struct epoll_event * retEv;
	retEv = malloc(sizeof(struct epoll_event));
	if(retEv == NULL)
		perror("malloc");
	while(1){
		ret = epoll_wait(epfd, retEv, 1, TIME_OUT_MS);
		if(ret == -1)
			perror("epoll wait");
		else if(ret == 0)
			break;
		for(int i = 0; i < ret; i++)
			processEvents(retEv[i].data.fd);
	}
}

/********************
  @fn watchFile

  initializes and intify instance and adds the filename given
  retruns the instance
 */
int watchFile(const char * pathname){
	int infd;
	infd = inotify_init1(0);
	if(infd == -1)
		perror("inotify_init");
	int watch;
	watch = inotify_add_watch(	infd,
					pathname,
					IN_ALL_EVENTS
				);
	if(watch == -1){
		perror("inotify add watch");
		exit(EXIT_FAILURE);
	}
	return infd;
}


/********************
  @fn processEvents

  read from the given inotify file descriptor
  and print out its events
 */
void processEvents(int fd){
	unsigned int len;
	int i = 0;
	int ret;
	struct inotify_event * evt;

	ret = ioctl(fd, FIONREAD, &len);
	if(ret == -1)
		perror("ioctl");

	char buff[len+1];
	ret = read(fd, buff, (size_t)len);

	while(len > i){
		evt = (struct inotify_event *)&buff[i];

		printEvents(evt->mask);
		printf("\t");
		if(evt->len != 0)
			printf("filename = %s ", evt->name);
		printf("mask = %d, cookie = %d", evt->mask, evt->cookie);
		printf("\n");

		i += (sizeof(struct inotify_event) + evt->len);
	}
}

/********************
  @fn printEvents

  print the event in human terms
 */
void printEvents(uint32_t mask){
		if(mask & IN_ACCESS)
			printf("A file was accessed\n");
		if(mask & IN_MODIFY)
			printf("A file was written to\n");
		if(mask & IN_ATTRIB)
			printf("A file's metadata was changed\n");
		if(mask & IN_CLOSE_WRITE)
			printf("A file openned for writting was closed\n");
		if(mask & IN_CLOSE_NOWRITE)
			printf("A file not openned for writting was closed\n");
		if(mask & IN_OPEN)
			printf("A file was opened\n");
		if(mask & IN_MOVED_FROM)
			printf("A file has left the directory\n");
		if(mask & IN_MOVED_TO)
			printf("A file has entered the directory\n");
		if(mask & IN_CREATE)
			printf("A file was created\n");
		if(mask & IN_DELETE)
			printf("A file was deleted\n");
		if(mask & IN_DELETE_SELF)
			printf("A watch was deleted\n");
		if(mask & IN_MOVE_SELF)
			printf("A watch was moved\n");
		if(mask & IN_IGNORED)
			printf("A file or watch was deleted\n");
		if(mask & IN_UNMOUNT)
			printf("Unmounted\n");
}

