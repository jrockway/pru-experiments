#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


       
int main(int argc, char **argv) {
  int fd;
  char buf[10];

  int retval;
  fd_set rfds;
  struct timeval timeout;

  int n, off;

  int total, rise;
  double total_time, rise_time;
  
  FD_ZERO(&rfds);
  fd = open("/dev/rpmsg_pru30", O_RDONLY | O_NONBLOCK);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  FD_SET(fd, &rfds);

  off = 0;
  while(1) {
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    retval = select(fd+1, &rfds, NULL, NULL, &timeout);
    if (retval < 0) {
      perror("select");
      return 1;
    }

    n = read(fd, &buf[off], sizeof(buf)-off);
    if (n < 0) {
      perror("read");
      return 1;
    }
    off += n;
    if (off >= 10) {
      if (buf[0] == '\0' && buf[1] == '\0') {
        total = *(int *)&buf[2];
        rise = *(int *)&buf[6];

        printf("total=%uns rise=%uns\n", total*5, rise*5);
        buf[0] = 'X';
        buf[1] = 'X';
        n = 0;
        off = 0;
      } else {
        fprintf(stderr, "corrupt header\n");
        return 1;
      }
    }
  }

  return 0;
}
