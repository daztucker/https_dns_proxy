#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "logging.h"
#include "options.h"

void options_init(struct Options *opt) {
  opt->listen_addr = "127.0.0.1";
  opt->listen_port = 5053;
  opt->logfile = "/dev/stdout";
  opt->logfd = -1;
  opt->loglevel = LOG_ERROR;
  opt->daemonize = 0;
  opt->user = "nobody";
  opt->group = "nobody";
  opt->uid = -1;
  opt->gid = -1;
  opt->bootstrap_dns = "8.8.8.8,8.8.4.4";
};

int options_parse_args(struct Options *opt, int argc, char **argv) {
  int ix, c;
  while((c = getopt(argc, argv, "a:p:du:g:b:l:v")) != -1) {
    switch(c) {
      case 'a':  // listen_addr
        opt->listen_addr = optarg;
        break;
      case 'p':  // listen_port
        opt->listen_port = atoi(optarg);
        break;
      case 'd':  // daemonize
        opt->daemonize = 1;
        break;
      case 'u':  // user
        opt->user = optarg;
        break;
      case 'g':  // group
        opt->group = optarg;
        break;
      case 'b':  // bootstrap dns servers
        opt->bootstrap_dns = optarg;
        break;
      case 'l':  // logfile
        opt->logfile = optarg;
        break;
      case 'v': // verbose
        opt->loglevel--;
        break;
      case '?':
        printf("Unknown option '-%c'", c);
        return -1;
      default:
        printf("Unknown state!");
        exit(EXIT_FAILURE);
    }
  }
  if (opt->daemonize) {
    struct passwd *p;
    if (!(p = getpwnam(opt->user)) || !p->pw_uid) {
      printf("Username (%s) invalid.\n", opt->user);
      return -1;
    }
    opt->uid = p->pw_uid;
    struct group *g;
    if (!(g = getgrnam(opt->group)) || !g->gr_gid) {
      printf("Group (%s) invalid.\n", opt->group);
      return -1;
    }
    opt->gid = g->gr_gid;
  }
  if ((opt->logfd = open(opt->logfile, O_CREAT | O_WRONLY | O_APPEND,
                         S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)) <= 0) {
    printf("Logfile '%s' is not writable.\n", opt->logfile);
  }
  return 0;
}

void options_show_usage(int argc, char **argv) {
  struct Options defaults;
  options_init(&defaults);
  printf("Usage: %s [-a <listen_addr>] [-p <listen_port>]\n", argv[0]);
  printf("        [-d] [-u <user>] [-g <group>] [-b <dns_servers>]\n");
  printf("        [-l <logfile>]\n\n");
  printf("  -a listen_addr    Local address to bind to. (%s)\n",
         defaults.listen_addr);
  printf("  -p listen_port    Local port to bind to (%d).\n", 
         defaults.listen_port);
  printf("  -d                Daemonize\n");
  printf("  -u user           User to drop to launched as root (%s).\n",
         defaults.user);
  printf("  -g group          Group to drop to launched as root (%s).\n",
         defaults.group);
  printf("  -b dns_servers    Comma separated IPv4 address of DNS servers\n");
  printf("                    to resolve dns.google.com (%s)\n",
         defaults.bootstrap_dns);
  printf("  -l logfile        Path to file to log to. (%s)\n",
         defaults.logfile);
  printf("  -v                Increase logging verbosity (INFO)\n");
  options_cleanup(&defaults);
}

void options_cleanup(struct Options *opt) {
  if (opt->logfd > 0) close(opt->logfd);
}
