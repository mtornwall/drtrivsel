#define _GNU_SOURCE

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include"fet.h"


static void usage(){
  printf("Usage:  load <filename> [at] <address>\n");
  }

int cmd_load(int again, char **argv){
  paddr address;
  char *endptr, **as=argv+2;
  struct stat ss;
  int fd;
  unsigned char *buf;
  uword *memblock;
  long words;
  int ret=1;

  if(!argv[1] || !argv[2] || (!strcmp(argv[2], "at") && ++as && !argv[3]))
    USAGE()

  address=strtoul(*as, &endptr, 0);
  if(*endptr)USAGE()
  
  if((fd=open(argv[1], O_RDONLY))<0 || fstat(fd, &ss)<0){
    printf("\"%s\": %s\n", argv[1], strerror(errno));
    if(fd>0)close(fd);
    return 1;
    }

  words=(ss.st_size+1)/2;

  if(!(buf=malloc(words*2))){
    printf("Out of memory\n");
    goto fail;
    }

  memset(buf, 42, words*2);

  if(read(fd, buf, ss.st_size)<0){
    printf("\"%s\": %s\n", argv[1], strerror(errno));
    goto fail;
    }

  // ### Perhaps add "as <filename>" to this command
  if(runcmd("map mem(%ld) at %ld", words*2, (long)address))goto fail;

  memblock=mem_get_block(addr_to_dev(address));

  for(long i=0; i<words; i++)
    memblock[i]=(((uword)buf[i*2])<<8)|buf[i*2+1];

  ret=0;

fail:
  free(buf);
  close(fd);

  return ret;
  }
