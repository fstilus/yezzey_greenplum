/*-------------------------------------------------------------------------
 *
 * postmaster.h
 *	  Exports from postmaster/postmaster.c.
 *
 * Portions Copyright (c) 1996-2014, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/postmaster/postmaster.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef _POSTMASTER_H
#define _POSTMASTER_H

/* GUC options */
extern bool EnableSSL;
extern int	ReservedBackends;
extern PGDLLIMPORT int PostPortNumber;
extern int	Unix_socket_permissions;
extern char *Unix_socket_group;
extern char *Unix_socket_directories;
extern char *ListenAddresses;
extern char *interconnect_address;
extern bool ClientAuthInProgress;
extern int	PreAuthDelay;
extern int	AuthenticationTimeout;
extern bool Log_connections;
extern bool log_hostname;
extern bool enable_bonjour;
extern char *bonjour_name;
extern bool restart_after_crash;

#ifdef WIN32
extern HANDLE PostmasterHandle;
#else
extern int	postmaster_alive_fds[2];

/*
 * Constants that represent which of postmaster_alive_fds is held by
 * postmaster, and which is used in children to check for postmaster death.
 */
#define POSTMASTER_FD_WATCH		0		/* used in children to check for
										 * postmaster death */
#define POSTMASTER_FD_OWN		1		/* kept open by postmaster only */
#endif

#define POSTMASTER_IN_STARTUP_MSG "the database system is starting up"
#define POSTMASTER_IN_RESET_MSG "the database system is resetting"
#define POSTMASTER_IN_RECOVERY_MSG "the database system is in recovery mode"
#define POSTMASTER_IN_RECOVERY_DETAIL_MSG "last replayed record at"
/* gpstate must be updated if this message changes */
#define POSTMASTER_MIRROR_VERSION_DETAIL_MSG "- VERSION:"

extern const char *progname;
extern PGDLLIMPORT const char *progname;

extern void PostmasterMain(int argc, char *argv[]) __attribute__((noreturn));
extern void ClosePostmasterPorts(bool am_syslogger);

extern int	MaxLivePostmasterChildren(void);

extern int	GetNumShmemAttachedBgworkers(void);
extern bool PostmasterMarkPIDForWorkerNotify(int);

extern void ResetMirrorReadyFlag(void);

#ifdef EXEC_BACKEND
extern pid_t postmaster_forkexec(int argc, char *argv[]);
extern void SubPostmasterMain(int argc, char *argv[]) __attribute__((noreturn));

extern Size ShmemBackendArraySize(void);
extern void ShmemBackendArrayAllocation(void);
#endif

extern void load_auxiliary_libraries(void);
extern bool amAuxiliaryBgWorker(void);
extern bool IsUnderMasterDispatchMode(void);

#ifdef ENABLE_IC_PROXY
# define IC_PROXY_NUM_BGWORKER 1
#else  /* ENABLE_IC_PROXY */
# define IC_PROXY_NUM_BGWORKER 0
#endif  /* ENABLE_IC_PROXY */

/*
 * Note: MAX_BACKENDS is limited to 2^23-1 because inval.c stores the
 * backend ID as a 3-byte signed integer.  Even if that limitation were
 * removed, we still could not exceed INT_MAX/4 because some places compute
 * 4*MaxBackends without any overflow check.  This is rechecked in the relevant
 * GUC check hooks and in RegisterBackgroundWorker().
 */
#define MAX_BACKENDS	0x7fffff
#define MaxPMAuxProc	(6 + IC_PROXY_NUM_BGWORKER)

#endif   /* _POSTMASTER_H */
