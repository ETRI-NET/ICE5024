#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "../nnStr.h"
#include <netinet/in.h>

int main(void)
{
	Int8T strResult[][8] = {"FALSE", "TRUE"};
	Int8T remainStr[40] = "123	456 789fffejkfjkljf";
	Int8T tokenStr[40];
	Int32T ret;

	StringT strNum1 = "1000";
	Int32T intNum1 = 0;
	Int8T strNum2[10] = {0,};

	Int8T charWord = 'A';

	Int8T addrToStr[20] = {0,};

	StringT ipv4 = "192.168.1.1";
	StringT ipv4nprefix = "192.168.1.1/24";
	StringT ipv6 = "1111:2222:3333:4444:5555:6666:7777:8888";
	StringT ipv6nprefix = "999:AAAA:BBBB:CCCC:DDDD:EEEE:FFFF:8888/128";

	StringT mac = "01:23:45:67:89:AB";

	Int32T prefix = 0;

	Int8T pre2string[20] = {0,};

	struct sockaddr_in addr1 = {0,};
	char *strIp = NULL;

	time_t numTime = 0;
	struct tm *structTime = NULL;
	Int8T strTime[22] = {0,};

	Int8T strTest[20] = "192.168.11.1/24";
	Int8T strTest2[20] = {0,};
	Int8T testTime[10] = "11:22:33";

	Int8T strTestDup1[20] = "Hi";
	Int8T strTestDup2[20] = "Hello";

	struct timespec nowTime;

	printf("Str:%s\n\n", remainStr);

	memInit(1);
	nnLogInit(1);

	/* StrGetAndToken */
	printf("\n== nnStrGetAndToken\n");
	printf("strTest : %s\n", strTest);
	printf("strTest2 : %s\n", strTest2);

	nnStrGetAndToken(strTest, strTest2, "/");

	printf("strTest : %s\n", strTest);
	printf("strTest2 : %s\n", strTest2);

	strcpy(strTest, "192.168.11.1/24");
	memset(strTest2, 0, sizeof(strTest2));

	/* StrGetOrToken */
	printf("\n== nnStrGetOrToken\n");
	printf("strTest : %s\n", strTest);
	printf("strTest2 : %s\n", strTest2);

	nnStrGetOrToken(strTest, strTest2, "/");

	printf("strTest : %s\n", strTest);
	printf("strTest2 : %s\n", strTest2);

	/* Sprintf */
	printf("\n== nnSprintf\n");
	memset(strTest2, 0, sizeof(strTest2));
	printf("strTest2 : %s\n", strTest2);
	ret = nnSprintf(strTest2, "%s", "Test nnSprintf");
	printf("Result : %d, strTest2 : %s\n", ret, strTest2);

	/* StringT to IntT */
	printf("\n== nnStrDupString\n");
	printf("Result : %d\n", nnStrDupString(strTestDup1, strTestDup2));

	/* Type Check */
	printf("\n== nnStrCheckIPv4Type(IPV4)\n");
	printf("Result : %s, str : %s\n", strResult[nnStrCheckIPv4Type(ipv4)], ipv4);
	printf("\n== nnStrCheckIPv4PrefixType(IPV4_PREFIX)\n");
	printf("Result : %s, str : %s\n", strResult[nnStrCheckIPv4PrefixType(ipv4nprefix)], ipv4nprefix);

	printf("\n== nnStrCheckIPv6Type(IPV6)\n");
	printf("Result : %s, str : %s\n", strResult[nnStrCheckIPv6Type(ipv6)], ipv6);
	printf("\n== nnStrCheckIPv6PrefixType(IPV6_PREFIX)\n");
	printf("Result : %s, str : %s\n", strResult[nnStrCheckIPv6PrefixType(ipv6nprefix)], ipv6nprefix);

	printf("\n== nnStrCheckMacAddressType(Mac Address)\n");
	printf("Result : %s, str : %s\n", strResult[nnStrCheckMacAddressType(mac)], mac);

	printf("\n== nnStrCheckTime(Time)\n");
	printf("Result : %s, str : %s\n", strResult[nnStrCheckTime(testTime)], testTime);

	/* Convert Function */
	printf("================ Convert Function ================\n");

	/* StringT to IntT */
	printf("\n== nnCnvStringtoInt\n");
	intNum1 = nnCnvStringtoInt(strNum1);
	printf("strNum1 : %s -> intNum1 : %d\n", strNum1, intNum1);

	/* IntT to StringT */
	printf("\n== nnCnvInttoString\n");
	ret = nnCnvInttoString(strNum2, intNum1);
	printf("Result : %d, intNum1 : %d -> strNum2 : %s\n", ret, intNum1, strNum2);

	/* Char to Int */
	printf("\n== nnCnvChartoInt\n");
	intNum1 = nnCnvChartoInt(charWord);
	printf("charWord : %c -> intNum1 : %d\n", charWord, intNum1);

	/* Int to Char */
	intNum1 = 97;
	memset(&charWord, 0, sizeof(char));
	printf("\n== nnCnvInttoChar\n");
	ret = nnCnvInttoChar(&charWord, intNum1);
	printf("Result : %d, intNum1 : %d -> charWord : %c\n", ret, intNum1, charWord);
#if 0
	/* TimeT to StringT */
	printf("\n== nnCnvTimetoString\n");
        numTime = time(NULL);

	printf("NowTime : %ld\n", numTime);
	structTime = nnCnvTimetoString(&numTime);

	if ((Int32T)structTime == FAILURE)
	{
		printf("Time to String is Failed\n");
	}
	else
	{
	        sprintf(strTime, "%d%02d%02d %02d:%02d:%2.2d",
                structTime->tm_year + 1900, structTime->tm_mon + 1, structTime->tm_mday,
                structTime->tm_hour, structTime->tm_min, structTime->tm_sec);

		printf("time_t : %ld -> struct tm : %s\n", numTime, strTime);
	}

	/* StringT to TimeT */
	printf("\n== nnCnvStringtoTime\n");

	numTime = 0;
	ret = nnCnvStringtoTime(&numTime, structTime);

	printf("Result : %d, time_t : %ld\n", ret, numTime);
#endif

	/* nnSprintfApp */
	printf("\n== nnSprintfApp\n");
	memset(strTest2, 0, sizeof(strTest2));
	printf("strTest2 : %s\n", strTest2);
	ret = nnSprintfApp(strTest2, 20, "%s", "deny");
	printf("Result : %d, strTest2 : %s\n", ret, strTest2);
       	ret = nnSprintfApp(strTest2, 20, " any");
	printf("Result : %d, strTest2 : %s\n", ret, strTest2);
        


	nnLogClose();

	memClose();

	return 0;
}
