/*
 ============================================================================
 Name        : gMaxLinked.cpp
 Author      : Rafael Gu
 Version     : 1.0.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#include "./config/Config.h"
#include "./transaction/CTransactionManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/syslog.h>

const c1_ DAEMON_NAME[] = "gMaxLinked";
const c1_ RUN_AS_USER[] = "root";
const c1_ LOCK_FILE[] = "/var/lock/subsys/gMaxLinked";
const c1_ PID_FILE[] = "/var/run/gMaxLinked.pid";

static none_ sigHandler(b4_ sigNo) {
	switch (sigNo) {
	case SIGALRM:
		exit(EXIT_FAILURE);
		break;
	case SIGUSR1:
		exit(EXIT_SUCCESS);
		break;
	case SIGCHLD:
		exit(EXIT_FAILURE);
		break;
	}
}

static none_ runAsDaemon() {
	pid_t pid, sid, parent;

	// already a daemon
	if (getppid() == 1) {
		return;
	}

	// create the lock file as the current user
	if (open(LOCK_FILE, O_RDWR | O_CREAT, 0640) < 0) {
		syslog(LOG_ERR, "unable to create lock file %s, code=%d (%s)",
				LOCK_FILE, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// drop user if there is one, and we were run as root
	if (getuid() == 0 || geteuid() == 0) {
		struct passwd *pw = getpwnam(RUN_AS_USER);

		if (pw) {
			syslog(LOG_NOTICE, "setting user to %s", RUN_AS_USER);
			setuid(pw->pw_uid);
		}
	}

	// trap signals that we expect to receive
	signal(SIGCHLD, sigHandler);
	signal(SIGUSR1, sigHandler);
	signal(SIGALRM, sigHandler);

	// fork off the parent process
	pid = fork();

	if (pid < 0) {
		syslog(LOG_ERR, "unable to fork daemon, code=%d (%s)", errno,
				strerror(errno));
		exit(EXIT_FAILURE);
	}

	// if we got a good PID, then we can exit the parent process
	if (pid > 0) {
		// wait for confirmation from the child via SIGTERM or SIGCHLD, or for
		// two seconds to elapse (SIGALRM).
		// pause() should not return
		alarm(2);
		pause();

		exit(EXIT_FAILURE);
	}

	// at this pob4_ we are executing as the child process
	parent = getppid();

	// cancel certain signals
	signal(SIGCHLD, SIG_DFL); // a child process dies
	signal(SIGTSTP, SIG_IGN); // various TTY signals
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, SIG_IGN); // ignore hangup signal
	signal(SIGTERM, SIG_DFL); // die on SIGTERM

	// change the file mode mask
	umask(0);

	// create a new SID for the child process
	sid = setsid();

	if (sid < 0) {
		syslog(LOG_ERR, "unable to create a new session, code %d (%s)", errno,
				strerror(errno));
		exit(EXIT_FAILURE);
	}

	// change the current working directory
	// this prevents the current directory from being locked; hence not being
	// able to remove it
	if ((chdir("/tmp")) < 0) {
		syslog(LOG_ERR, "unable to change directory to %s, code %d (%s)",
				"/tmp", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// redirect standard files to /dev/null
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);

	// tell the parent process that we are A-okay
	kill(parent, SIGUSR1);

	// create the pid file
	b4_ fd = open(PID_FILE, O_RDWR | O_CREAT, 0640);

	if (fd < 0) {
		syslog(LOG_ERR, "unable to create pid file %s, code=%d (%s)", PID_FILE,
		errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// get pid and write b4_o PID_FILE
	b1_ temp[16];

	pid = getpid();
	memset(temp, 0, 16);
	sprintf(temp, "%d", pid);
	write(fd, temp, strlen(temp) + 1);
	close(fd);
}

// we need to configure the system manually to make the feature work
// shell(as root): ulimit -c unlimited
static none_ enableDump() {
	// core dump
	struct rlimit res;

	res.rlim_cur = RLIM_INFINITY;
	res.rlim_max = RLIM_INFINITY;

	setrlimit(RLIMIT_CORE, &res);
}

static none_ run(b4_ argc, c1_ **argv) {
	// load configure file
	c1_ *conf = null_v;

	if (2 <= argc) {
		conf = argv[1];
	}

	Config::initialize(conf);
	log_info("%s will start.", DAEMON_NAME);

	if (Config::App::RUN_AS_DAEMON) {
		runAsDaemon();
	}

	log_info("%s is working ...", DAEMON_NAME);
	CTransactionManager::instance()->work();

	log_fatal("%s will end.", DAEMON_NAME);
	CWorker::stop();

	while (CWorker::getTotalNum()) {
		CBase::sleep(1, 0); // sleep for 1 second
	}

	log_info("%s is ended.", DAEMON_NAME);
}

b4_ main(b4_ argc, c1_ **argv) {
	if (!CBase::initialize()) {
		return 1;
	}

	// initialize the logging b4_erface
	openlog(DAEMON_NAME, LOG_USER | LOG_PID, LOG_LOCAL5);
	syslog(LOG_INFO, "starting...");

	// enable dump to catch the bugs and issues
	enableDump();

	// run loop
	run(argc, argv);

	// finish up
	syslog(LOG_NOTICE, "terminated");
	closelog();

	CBase::uninitialize();

	return 0;
}
