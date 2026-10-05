#ifndef DAEMON_H
#define DAEMON_H

/* Runs sensord until SIGTERM/SIGINT. Returns the exit code. */
int daemon_run(const char *config_path);

#endif
