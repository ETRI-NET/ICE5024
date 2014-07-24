#include <unistd.h>
#include <linux/if.h>
#include <sys/ioctl.h>

#if 1
#define DEBUGPRINT(...)	fprintf(stderr, ##__VA_ARGS__)
#else
#define DEBUGPRINT(...) do {} while(0)
#endif

#define LACP_GROUP_UNDEFINED (-1)

#define	LACP_DEFAULT_SYS_PRIORITY	65535
#define	LACP_SYS_TIMEOUT_LONG	1
#define	LACP_SYS_TIMEOUT_SHORT	0
#define	LACP_DEFAULT_SYS_TIMEOUT	LACP_SYS_TIMEOUT_LONG
#define	LACP_DEFAULT_PORT_PRIORITY	255

typedef struct LacpGroupContext {
	Int32T groupId;
	Int32T longTimeout;
	Int32T sysPriority;
	void	*context;
} LacpGroupContextT;

typedef struct LacpFiniContext {
	long	context;
} LacpGroupFiniT;

typedef struct LacpPort {
	Int8T ifName[32];
	Int32T groupId;
	Int32T active;
	Int32T priority;
	Int32T ifIndex;
} LacpPortT;

typedef struct Lacp {
	void *pCmshGlobal;
	ListT *pLacpPortList;
	ListT *pLacpGroupContextList;
	ListT *pLacpGroupFiniList;
#if 1
	ListT *pLacpPifPortList;
#endif
	Int32T sigIntEventFd;
	void *sigIntEventTask;
	Int32T sigTermEventFd;
	void *sigTermEventTask;
	Int32T sigQuitEventFd;
	void *sigQuitEventTask;
	Int32T quit;
	Int32T debug;
} LacpT;

Int32T lacpRunGroupAdd (Int32T groupId);
Int32T lacpRunGroupDel (Int32T groupId);
Int32T lacpRunPortAdd (Int32T groupId, Int8T *ifName);
Int32T lacpRunPortDel (Int32T groupId, Int8T *ifName);
Int32T lacpRunPortChange (Int32T groupId, Int8T *ifName);
void lacpRunStateDisplay (struct cmsh *cmsh, Int32T groupId);
void lacpRunConfigDisplay (struct cmsh *cmsh, Int32T groupId);
Int32T lacpGetRunGroupHwAddr(Int32T groupId, Int8T *mac);

void configWriteLacp (struct cmsh *cmsh);
void lacpGroupInit (void);
void lacpGroupFini (void);
Int32T lacpPortAttach (struct cmsh *, Int32T, Int8T *, Int32T);
Int32T lacpPortDetach (struct cmsh *, Int8T *);
Int32T getLacpConfig (Int32T groupId, LacpPortT **pPortGet);
void lacpCliConfigDisplay (struct cmsh *cmsh);
Int32T lacpGroupTimeoutChange (struct cmsh *, Int32T, Int32T);
Int32T lacpGetPortIfIndex(Int8T *ifName);

void lacpToPifReportAggregatorAdd (Int32T groupId);
void lacpToPifReportAggregatorDelete (Int32T groupId);
void lacpToPifReportPortAdd (Int32T groupId);
void lacpToPifReportPortDelete (Int32T groupId);

/* lacpProc.c */
void lacpDevFiniStart (void *context, struct timeval *tv, void *callback);
void lacpDevFiniEnd (void *context);

/* lacpTeam.c */
void *lacpDevAggregatorAdd (Int8T *configFile, Int32T debug);
void lacpDevAggregatorDelete (void *context);
void lacpDevPortAdd (void *context, Int8T *ifName);
void lacpDevPortDelete (void *context, Int8T *ifName);
void lacpDevPortModify (void *context, Int8T *ifName);
void lacpDevModify (void *context);
void lacpDevEventCallback (Int32T fd, void *arg);
Int8T *lacpDevConfigDump(void *context);
Int8T *lacpDevStateDump(void *context);
void lacpDevCallbackChange(void *context);
extern LacpT * pLacp;

void lacpTermProcess (void);
void lacpRestartProcess (void);

void lacpPifPortDisplay (struct cmsh *cmsh);
