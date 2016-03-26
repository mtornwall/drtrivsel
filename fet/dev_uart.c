#define _GNU_SOURCE

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
#include<errno.h>
#include"fet.h"


typedef struct{
  device dev;
  int ifd, ofd;
  pid_t xterm_pid;
  }uart;

device dev_uart;  // Defined at the bottom of this file

enum{BACKEND_PTY, BACKEND_FILE, BACKEND_NULL};

static pid_t xterm_killpid;

static void xterm_sighup(int foo){
  kill(xterm_killpid, SIGTERM);
  }

static void xterm_sigchld(int foo){
  exit(0);
  }

static void usage(){
  devinit_usage(
    "uart",
    "pty|file(<name>)|null [noterm]",
    "[in=pty|file(<name>)|null] [out=pty|file(<name>)|null] [noterm]", 0);
  }

static device *create(char *name, paddr start, char **argv){
  uart *uart;

  uart=malloc(sizeof(*uart));
  uart->ifd=uart->ofd=-1;
  uart->xterm_pid=0;
  uart->dev=dev_uart;
  uart->dev.argv_temp=argv;
  uart->dev.start=start;
  uart->dev.end=start+1;
  uart->dev.n_mapped=-1;     // So we can tell that this device struct is a
                             // mapped device and not the devtype struct
  return (device *)uart;
  }

static int init(device *_dev){
  uart *dev=(uart *)_dev;
  int itype=BACKEND_PTY, otype=BACKEND_PTY;
  char *infile, *outfile;
  int xterm=1;
  char **argv=_dev->argv_temp;
  int set_in=1, set_out=1;
  int ptm=-1, pts;

  while(*argv){
    if(!strcmp(*argv, "pty")){
      if(set_in)itype=BACKEND_PTY;
      if(set_out)otype=BACKEND_PTY;
      if(!(set_in || set_out)){usage(); return -1;}
      set_in=set_out=0;
      }
    else if(!strcmp(*argv, "file")){
      if(set_in)itype=BACKEND_FILE;
      if(set_out)otype=BACKEND_FILE;
      if(!(set_in || set_out)){usage(); return -1;}

      ++argv;
      if(! (argv[0] && !strcmp(argv[0], "(") &&
            argv[1] &&
            argv[2] && !strcmp(argv[2], ")")) ){usage(); return -1;}
      if(set_in)infile=argv[1];
      if(set_out)outfile=argv[1];

      argv+=2;

      set_in=set_out=0;
      }
    else if(!strcmp(*argv, "null")){
      if(set_in)itype=BACKEND_NULL;
      if(set_out)otype=BACKEND_NULL;
      if(!(set_in || set_out)){usage(); return -1;}
      set_in=set_out=0;
      }
    else if(set_in != set_out){usage(); return -1;}
    else if(!strcmp(*argv, "in")){
      ++argv;
      if(! (*argv && !strcmp(*argv, "="))){usage(); return -1;}
      set_in=1; set_out=0;
      }
    else if(!strcmp(*argv, "out")){
      ++argv;
      if(! (*argv && !strcmp(*argv, "="))){usage(); return -1;}
      set_in=0; set_out=1;
      }
    else if(!strcmp(*argv, "noterm")){
      xterm=0;
      }
    else{usage(); return -1;}
    ++argv;
    }

  if(set_in != set_out){usage(); return -1;}

  if(itype==BACKEND_PTY || otype==BACKEND_PTY){
    ptm=open("/dev/ptmx", O_RDWR|O_NOCTTY|O_CLOEXEC);
    if(ptm<0){printf("Couldn't create PTY.\n"); return -1;}
    grantpt(ptm);
    unlockpt(ptm);
    if(fork())wait(0);
    else execlp("stty", "stty", "raw", "-echo", "-F", ptsname(ptm), 0);
    }

  switch(itype){
    case BACKEND_PTY: dev->ifd=ptm; break;
    case BACKEND_FILE: 
      dev->ifd=open(infile, O_RDONLY);
      if(dev->ifd<0){
        printf("Could not open \"%s\": %s\n", infile, strerror(errno));
        goto fail;
        }
      break;
    case BACKEND_NULL: break;
    }

  switch(otype){
    case BACKEND_PTY: dev->ofd=ptm; break;
    case BACKEND_FILE:
      dev->ofd=open(outfile, O_WRONLY);
      if(dev->ofd<0){
        printf("Could not open \"%s\": %s\n", outfile, strerror(errno));
        goto fail;
        }
      break;
    case BACKEND_NULL: break;
    }

  if(xterm && ptm!=-1){
    pid_t xterm_pid;
    char *name=ptsname(ptm);
    char *S_arg;
    char c;
    int pfd[2];

    pipe(pfd);
    xterm_pid=fork();
    if(!xterm_pid){
      close(pfd[0]);
      struct sigaction sa;
      xterm_killpid=getpid();
      pid_t child_pid;

      sigemptyset(&sa.sa_mask);
      sa.sa_flags=0;
      sa.sa_handler=xterm_sighup;
      sigaction(SIGHUP, &sa, 0);
      sa.sa_handler=xterm_sigchld;
      sigaction(SIGCHLD, &sa, 0);

      close(ptm);

      setsid();          // So that we will get a SIGHUP when the ptm is closed
      pts=open(name, O_RDWR);
      if(pts<0){
        printf("Could not open \"%s\": %s\n", name, strerror(errno));
        exit(1);
        }
      close(pfd[1]);     // This lets the parent know we have opened the pts

      child_pid=fork();
      if(child_pid){              // This process now just waits for a SIGHUP;
        xterm_killpid=child_pid;  // the handler sends a SIGTERM to the xterm.
        while(1)pause();          // Needed because xterm ignores SIGHUP.
        }
      else{
        asprintf(&S_arg, "-S%s/%d", strrchr(name, '/')+1, pts);
        execlp("xterm", "xterm", S_arg, 0);  // TODO: add error handling
        exit(1);
        }
      }

    close(pfd[1]);       // The point of this pipe is to block on it until the
    read(pfd[0], &c, 1); // child closes the write end after it has opened the
    close(pfd[0]);       // pts, so that we can read and discard the window ID
                         // that xterm writes to the pts when it starts.
    while(1){            
      if(read(ptm, &c, 1)==-1){
        printf("Couldn't start xterm\n");
        goto fail;
        }
      if(c=='\n')break;
      }

    dev->xterm_pid=xterm_pid;
    }

  return 0;

fail:
  close(ptm);
  close(dev->ifd);
  close(dev->ofd);
  return -1;
  }

static void writeb(device *_dev, paddr a, ubyte d){
  uart *dev=(uart *)_dev;
  switch(a){
    case 0:  // Data out register
      write(dev->ofd, &d, 1);
      break;
    }
  }

static void writew(device *_dev, paddr a, uword d){
//  uart *dev=(uart *)_dev;
  }

static ubyte readb(device *_dev, paddr a){
  uart *dev=(uart *)_dev;
  char c;
  switch(a){
    case 0:  // Data in register
      read(dev->ifd, &c, 1);
      return c;
    }
  return 0;
  }

static uword readw(device *_dev, paddr a){
//  uart *dev=(uart *)_dev;
  return 0;
  }

static void console_command(device *_dev, char **firstline){
//  uart *dev=(uart *)_dev;
  printf("NO\n");
  }

static int lookup_reg(paddr *addr, device *_dev, char *name){
//  uart *dev=(uart *)_dev;
  if(!strcmp("data", name)){*addr=0; return 0;}
  return 1;
  }


device dev_uart={
  "uart",            // typename
  0, 0, 0, 0, 0, 0,  // devname, n_mapped, argv_temp, start, end, next
  create, init, writeb, writew, readb, readw, console_command, lookup_reg
  };
