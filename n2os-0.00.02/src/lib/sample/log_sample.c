#include "../nnLog.h"
#include "../nnMemmgr.h"

#define TEST_PROCESS 0
#define REMOTE_IP "192.168.80.148"
#define LOOP 10000

int main()
{
	MemT *pMem = NULL;
	Int32T ret = 0;
	Uint32T size = 10000;
	Int32T count = 0;

	memInit(0);

	ret = nnLogInit(TEST_PROCESS);

	if (ret != SUCCESS)
	{
		printf("Log Alloc Failed\n");
		return -1;
	}

	printf("======== Set Log File ========\n");
	printf("Set Log FileName -> logSample\n");
	printf("Set Log FileSize -> %d\n", size);

/* Max Log FileName size : 20 */
	ret = nnLogSetFile(NULL, 1);	// Default

/* Set Priority */
	nnLogSetPriority(LOG_DEBUG);

	printf("Result = %d\n", ret);
	printf("======== Set Log FileName ========\n");
printf("\n\n\n");

/* Set Remote IP */
	printf("======== Set Log Remote IP ========\n");

	nnLogSetRemoteAddr(REMOTE_IP, LOG_DEFAULT_REMOTE_PORT);
	printf("Remote IP : %s:%u\n", REMOTE_IP, LOG_DEFAULT_REMOTE_PORT);

	printf("======== Set Log Remote IP ========\n");
printf("\n\n\n");

/* Log Output STDOUT and File */
	printf("======== Logging Start ========\n");
	NNLOGDEBUG(LOG_ERR, "%s\n", "1 == Test Log_Sample");
	NNLOGDEBUG(LOG_INFO, "%s\n", "2 == Print -> STDOUT and FILE");
	printf("======== Logging End ========\n");
printf("\n\n\n");

/* Set Output syslog */
	nnLogSetFlag(LOG_SYSTEM);
	printf("======== Set Log Flag ========\n");
	printf("Set Log Flag -> Syslog\n");
	printf("======== Set Log Flag ========\n");
printf("\n\n\n");

/* Log Output STDOUT, syslog and File */
	printf("======== Logging Start ========\n");
	NNLOGDEBUG(LOG_WARNING, "%s\n", "3 == Test Log_Sample");
	NNLOGDEBUG(LOG_NOTICE, "%s\n", "4 == Print -> STDOUT, syslog and FILE");
	printf("======== Logging End ========\n");
printf("\n\n\n");

/* Unset Output STDOUT */
	nnLogUnsetFlag(LOG_STDOUT);
	printf("======== UnSet Log Flag ========\n");
	printf("UnSet Log Flag -> STDOUT\n");
	printf("======== UnSet Log Flag ========\n");
printf("\n\n\n");

/* Unset Output STDOUT */
	nnLogUnsetFlag(LOG_SYSTEM);
	printf("======== UnSet Log Flag ========\n");
	printf("UnSet Log Flag -> syslog\n");
	printf("======== UnSet Log Flag ========\n");
printf("\n\n\n");

/* Log Output File Only */
	printf("======== Logging Start ========\n");
	NNLOGDEBUG(LOG_DEBUG, "%s\n", "5 == Test Log_Sample");
	NNLOGDEBUG(LOG_DEBUG, "%s\n", "6 == Print -> FILE Only");
	printf("======== Logging End ========\n");

	printf("======== Test Logging Start ========\n");
	printf("Looping : %d\n", LOOP);
	nnLogSetFlag(LOG_FILE);
	for(count = 0; count < LOOP; ++count)
	{
		NNLOG(LOG_DEBUG, "%d %s\n", count, " == !!Test Component Log_Sample");
		NNLOG(LOG_DEBUG, "%d %s\n", count, " == !!Print -> FILE Only");

		/* Log Output File Only */
		NNLOGDEBUG(LOG_DEBUG, "%d %s\n", count, " == !!Test Library Log_Sample");
		NNLOGDEBUG(LOG_DEBUG, "%d %s\n", count, " == !!Print -> FILE Only");
	}
	printf("======== Test Logging End ========\n");

	nnLogClose();

	memClose();

	return 0;
}
