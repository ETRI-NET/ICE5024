#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "../nnStr.h"
#include "../nnPrefix.h"
#include "../nnRibDefines.h"

int main(void)
{
	Int8T strResult[][8] = {"FAILURE", "SUCCESS"};
	Int8T ret = 0;
	Int32T ret32 = 0;
	Int32T value = 0;

	Int8T addrToStr[20] = {0,};
	Int8T addr6ToStr[40] = {0,};

	StringT ipv4 = "192.168.1.1";
	StringT ipv4nprefix = "1.1.1.1/24";
	StringT ipv6 = "1111:2222:3333:4444:5555:6666:7777:8888";
	StringT ipv6nprefix = "f111:f222:f333:f444:555f:666f:777f:888F/100";
	StringT netmask = "255.255.255.0";

	StringT strNum = "1234567";

	struct in_addr testAddr1 = {0,};
	struct in_addr testAddr2 = {0,};
	struct in_addr testBroadAddr = {0,};
	StringT strIp = NULL;
	StringT strRet = NULL;

	PrefixT testPrefix = {0,};
	PrefixT testPrefix2 = {0,};

        Uint8T prefixLen = 0;
	Uint32T testUint = 0;
	Uint8T *pTestUint = NULL;

	PrefixT *pTestPrefix = NULL;
	Prefix4T testPrefix4 = {0,};
	Prefix4T *pTestPrefix4 = NULL;

	Prefix6T testPrefix6 = {0,};
	Prefix6T testPrefix7 = {0,};
	Prefix6T *pTestPrefix6 = NULL;

	struct in6_addr testAddr6 = {0,};
	struct in6_addr testAddr7 = {0,};

	memInit(1);
	nnLogInit(1);

        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	/* nnCnvStringtoAddr() */
	printf("\n== nnCnvStringtoAddr(IPV4)\n");
	memset(&testAddr1, 0x0, sizeof(struct in_addr));
	strIp = inet_ntoa(testAddr1);
	printf("Before struct in_addr IP : %s\n\n", strIp);

	ret = nnCnvStringtoAddr(&testAddr1, ipv4);

	strIp = inet_ntoa(testAddr1);
	printf("After Result : %s, struct in_addr IP : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");



	printf("\n== nnCnvStringtoAddr(IPV4_PREFIX)\n");
	memset(&testAddr1, 0x0, sizeof(struct in_addr));
	strIp = inet_ntoa(testAddr1);
	printf("Before struct in_addr IP : %s\n\n", strIp);

	ret = nnCnvStringtoAddr(&testAddr1, ipv4nprefix);

	strIp = inet_ntoa(testAddr1);
	printf("After Result : %s, struct in_addr IP : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");



	/* nnCnvAddrtoString() */
	printf("\n== nnCnvAddrtoString\n");
	memset(addrToStr, 0, sizeof(addrToStr));
	memset(&testAddr1, 0, sizeof(testAddr1));

	testAddr1.s_addr = 0x0101A8C0;
	strIp = inet_ntoa(testAddr1);

	printf("Before struct struct in_addr IP : %s -> String IP : %s\n\n", strIp, addrToStr);

	ret = nnCnvAddrtoString(addrToStr, &testAddr1);

	printf("After Result : %s, String IP : %s\n", strResult[ret + 1], addrToStr);
        printf("=================================================================================================================================\n");



	/* nnCnvStringtoPrefix4() */
	printf("\n== nnCnvStringtoPrefix4(IPV4)\n");
	memset(&testPrefix4, 0x0, sizeof(Prefix4T));
	printf("Before String IP : %s -> sockaddr_in IP : %s\n\n", ipv4, strIp);

	ret = nnCnvStringtoPrefix4(&testPrefix4, ipv4);

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("After Result : %s, String IP : %s -> Prefix4T.prefix : %s\n", strResult[ret + 1], ipv4, strIp);
        printf("-----------------------------------------------------------\n");




	printf("\n== nnCnvStringtoPrefix4(IPV4_PREFIX)\n");
	memset(&testAddr1, 0x0, sizeof(struct in_addr));
        memset(strIp, 0, sizeof(strIp));
	printf("Before String IP : %s -> Prefix4T.prefix IP : %s, Family : %d, prefixLen : %d\n\n", ipv4nprefix, strIp, testPrefix4.family, testPrefix.prefixLen);

	ret = nnCnvStringtoPrefix4(&testPrefix4, ipv4nprefix);

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("After Result : %s, String IP : %s -> Prefix4T.prefix IP : %s, Family : %d, prefixLen : %d\n", strResult[ret + 1], ipv4nprefix, strIp, testPrefix4.family, testPrefix4.prefixLen);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefix4toString() */
	printf("\n== nnCnvPrefix4toString()\n");
	memset(addrToStr, 0, sizeof(addrToStr));
	memset(&testPrefix4, 0, sizeof(testPrefix4));

	testPrefix4.family = AF_INET;
	testPrefix4.prefixLen = 24;
	testPrefix4.prefix.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Before Prefix4T.prefix IP : %s -> String IP : %s\n\n", strIp, addrToStr);

	ret = nnCnvPrefix4toString(addrToStr, &testPrefix4);

	printf("After Result : %s, Prefix4T.prefix IP : %s -> String IP : %s\n", strResult[ret + 1], strIp, addrToStr);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvStringtoPrefix() */
	printf("\n== nnCnvStringtoPrefix(IPV4)\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));
        strIp = inet_ntoa(testPrefix.u.prefix4);

	printf("String Address : %s\n", ipv4);
	printf("Before Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);

	ret = nnCnvStringtoPrefix(&testPrefix, ipv4);

        strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("After Result : %s, Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", strResult[ret + 1], testPrefix.family, testPrefix.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");




	printf("\n== nnCnvStringtoPrefix(IPV4_PREFIX)\n");
	memset(&testPrefix, 0, sizeof(PrefixT));
        strIp = inet_ntoa(testPrefix.u.prefix4);

	printf("String Address : %s\n", ipv4nprefix);
	printf("Before Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);

	ret = nnCnvStringtoPrefix(&testPrefix, ipv4nprefix);

        strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("After Result : %s, Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", strResult[ret + 1], testPrefix.family, testPrefix.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");



	/* nnCnvPrefixtoString() */
	printf("\n== nnCnvPrefixtoString()\n");
	memset(addrToStr, 0, sizeof(addrToStr));
	memset(&testPrefix, 0, sizeof(testPrefix));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

        strIp = inet_ntoa(testPrefix.u.prefix4);

	printf("Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix.family, testPrefix.prefixLen, strIp);
	printf("Before String Address : %s\n\n", addrToStr);

	ret = nnCnvPrefixtoString(addrToStr, &testPrefix);

	printf("After Result : %s, String Address : %s\n", strResult[ret + 1], addrToStr);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefixLentoNetmask() */
	printf("\n== nnCnvPrefixLentoNetmask()\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	strIp = inet_ntoa(testAddr1);
	prefixLen = 24;
        printf("PrefixLen : %d\n", prefixLen);
	printf("Before Netmask IP : %s\n\n", strIp);

	ret = nnCnvPrefixLentoNetmask(&testAddr1, &prefixLen);

	strIp = inet_ntoa(testAddr1);
	printf("After Result : %s, Netmask IP : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvNetmasktoPrefixLen() */
	printf("\n== nnCnvNetmasktoPrefixLen()\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	prefixLen = 0;

	testAddr1.s_addr = 0x00FFFFFF;
	strIp = inet_ntoa(testAddr1);

        printf("Netmask : %s\n", strIp);
	printf("Before PrefixLen : %d\n\n", prefixLen);

	ret = nnCnvNetmasktoPrefixLen(&prefixLen, &testAddr1);

	printf("After Result : %s, PrefixLen : %d\n", strResult[ret + 1], prefixLen);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvMasklentoIp() */
	printf("\n== nnCnvMasklentoIp()\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	strIp = inet_ntoa(testAddr1);
	prefixLen = 24;
        printf("PrefixLen : %d\n", prefixLen);
	printf("Before Netmask IP : %s\n\n", strIp);

	nnCnvMasklentoIp((Int32T)prefixLen, &testAddr1);

	strIp = inet_ntoa(testAddr1);
	printf("After Netmask IP : %s\n", strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvIpmasktoLen() */
	printf("\n== nnCnvIpmasktoLen()\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	prefixLen = 0;

	testAddr1.s_addr = 0x00FFFFFF;
	strIp = inet_ntoa(testAddr1);

        printf("Netmask : %s\n", strIp);
	printf("Before PrefixLen : %d\n\n", prefixLen);

	prefixLen = nnCnvIpmasktoLen(testAddr1);

	printf("After PrefixLen : %d\n", prefixLen);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvAddrtoUint() */
	printf("\n== nnCnvAddrtoUint()\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	testUint = 0;

	testAddr1.s_addr = 0x0101A8C0;
	strIp = inet_ntoa(testAddr1);

        printf("Address : %s\n", strIp);
	printf("Before Uint : %d\n\n", testUint);

	ret = nnCnvAddrtoUint(&testUint, &testAddr1);

	printf("After Result : %s, Uint : %u\n", strResult[ret + 1], testUint);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnGetBroadcastfromAddr() */
	printf("\n== nnGetBroadcastfromAddr()\n");
	memset(&testBroadAddr, 0, sizeof(testBroadAddr));

	prefixLen = 24;
	testPrefix4.family = AF_INET;
	testPrefix4.prefix.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix4.prefix);
        printf("Address : %s, PrefixLen : %d\n", strIp, prefixLen);

	strIp = inet_ntoa(testBroadAddr);
	printf("Before Broadcast Address : %s\n\n", strIp);

	ret = nnGetBroadcastfromAddr(&testBroadAddr, &testPrefix4.prefix, &prefixLen);

	strIp = inet_ntoa(testBroadAddr);
	printf("After Result : %s, Broadcast Address : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnGetBroadcastfromPrefix() */
	printf("\n== nnGetBroadcastfromPrefix()\n");

	memset(&testBroadAddr, 0x0, sizeof(struct in_addr));
	memset(&testPrefix, 0x0, sizeof(Prefix4T));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix.family, testPrefix.prefixLen, strIp);
	strIp = inet_ntoa(testBroadAddr);
	printf("Before Broadcast Address : %s\n\n", strIp);

	ret = nnGetBroadcastfromPrefix(&testBroadAddr, &testPrefix);

	strIp = inet_ntoa(testBroadAddr);
	printf("After Result : %s, Broadcast Address : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnGetBroadcastfromString() */
	printf("\n== nnGetBroadcastfromString(IPV4)\n");
	memset(&testBroadAddr, 0x0, sizeof(struct in_addr));

	prefixLen = 24;
	strIp = inet_ntoa(testBroadAddr);

	printf("String Address : %s, PrefixLen : %d\n", ipv4, prefixLen);
	printf("Before Broadcast Address : %s\n\n", strIp);

	ret = nnGetBroadcastfromString(&testBroadAddr, ipv4, &prefixLen);

	strIp = inet_ntoa(testBroadAddr);
	printf("After Result : %s, Broadcast Address : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");



	printf("\n== nnGetBroadcastfromString(IPV4_PREFIX)\n");
	memset(&testBroadAddr, 0x0, sizeof(struct in_addr));

	prefixLen = 24;
	strIp = inet_ntoa(testBroadAddr);

	printf("String Address : %s, PrefixLen : %d\n", ipv4nprefix, prefixLen);
	printf("Before Broadcast Address : %s\n\n", strIp);

	ret = nnGetBroadcastfromString(&testBroadAddr, ipv4nprefix, &prefixLen);

	strIp = inet_ntoa(testBroadAddr);
	printf("After Result : %s, Broadcast Address : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnGetBroadcastfromUint() */
	printf("\n== nnGetBroadcastfromUint()\n");
	memset(&testBroadAddr, 0x0, sizeof(struct in_addr));
	testUint = 0x0101A8C0; /* 192.168.1.1 */
	prefixLen = 24;

	strIp = inet_ntoa(testBroadAddr);
	printf("Uint : %u, prefixLen : %d\n", testUint, prefixLen);
	printf("Before Broadcast Address : %s\n\n", strIp);

	ret = nnGetBroadcastfromUint(&testBroadAddr, &testUint, &prefixLen);

	strIp = inet_ntoa(testBroadAddr);
	printf("After Result : %s, Broadcast Address : %s\n", strResult[ret + 1], strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefix4TtoPrefixT() */
	printf("\n== nnCnvPrefix4ToPrefixT()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));
	memset(&testPrefix4, 0x0, sizeof(testPrefix4));

	testPrefix4.family = AF_INET;
	testPrefix4.prefixLen = 24;
	testPrefix4.prefix.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Prefix4 -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix4.family, testPrefix4.prefixLen, strIp);

	strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("Before Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);

	ret = nnCnvPrefix4TtoPrefixT(&testPrefix, &testPrefix4);

	strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("After Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix.family, testPrefix.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefixTtoPrefix4T() */
	printf("\n== nnCnvPrefixToPrefix4T()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));
	memset(&testPrefix4, 0x0, sizeof(testPrefix4));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix.family, testPrefix.prefixLen, strIp);

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Before Prefix4 -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix4.family, testPrefix4.prefixLen, strIp);

	ret = nnCnvPrefixTtoPrefix4T(&testPrefix4, &testPrefix);

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("After Prefix4 -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix4.family, testPrefix4.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");




	/* nnCnvAfitoFamily() */
	printf("\n== nnCnvAfitoFamily()\n");

	ret = nnCnvAfitoFamily(AFI_IP);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvFamilytoAfi() */
	printf("\n== nnCnvFamilytoAfi()\n");

	ret = nnCnvFamilytoAfi(AF_INET);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixMatch()) */
	printf("\n== nnPrefixMatch()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));
	memset(&testPrefix2, 0x0, sizeof(testPrefix2));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	testPrefix2.family = AF_INET;
	testPrefix2.prefixLen = 24;
	testPrefix2.u.prefix4.s_addr = 0x0101A8C0;

	ret = nnPrefixMatch(&testPrefix, &testPrefix2);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixCopy()) */
	printf("\n== nnPrefixCopy()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));
	memset(&testPrefix2, 0x0, sizeof(testPrefix2));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	ret = nnPrefixCopy(&testPrefix2, &testPrefix);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixSame()) */
	printf("\n== nnPrefixSame()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));
	memset(&testPrefix2, 0x0, sizeof(testPrefix2));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	testPrefix2.family = AF_INET;
	testPrefix2.prefixLen = 24;
	testPrefix2.u.prefix4.s_addr = 0x0101A8C0;

	ret = nnPrefixSame(&testPrefix, &testPrefix2);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixCmp()) */
	printf("\n== nnPrefixCmp()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));
	memset(&testPrefix2, 0x0, sizeof(testPrefix2));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	testPrefix2.family = AF_INET;
	testPrefix2.prefixLen = 24;
	testPrefix2.u.prefix4.s_addr = 0x0101A8C0;

	ret = nnPrefixCmp(&testPrefix, &testPrefix2);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefixFamilytoString()) */
	printf("\n== nnCnvPrefixFamilytoString()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	strRet = nnCnvPrefixFamilytoString(&testPrefix);

	printf("Result : %s\n", strRet);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixIpv4New() & nnPrefixIpv4Free()) */
	printf("\n== nnPrefixIpv4New() & nnPrefixIpv4Free()\n");

	if (pTestPrefix4 != NULL)
	{
		free(pTestPrefix4);
	}
	pTestPrefix4 = NULL;

	pTestPrefix4 = nnPrefixIpv4New();

	if ((Int32T)pTestPrefix4 == -1)
	{
		printf("Make New Prefix4 Failure\n");
	}
	else
	{
		printf("Make New Prefix4 SUCCESS\n");
		nnPrefixIpv4Free(pTestPrefix4);
		pTestPrefix4 = NULL;
		printf("Prefix4 Free SUCCESS\n");
	}

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnApplyNetmasktoPrefix4() */
	printf("\n== nnApplyNetmasktoPrefix4()\n");
	memset(&testPrefix4, 0x0, sizeof(testPrefix4));

	prefixLen = 24;
	testPrefix4.family = AF_INET;
	testPrefix4.prefix.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Before Prefix4 -> Prefix4->family : %d, Prefix4->prefixLen : %d, Prefix4->in_addr : %s\n\n", testPrefix4.family, testPrefix4.prefixLen, strIp);
	printf("PrefixLen : %d\n", prefixLen);

	nnApplyNetmasktoPrefix4(&testPrefix4, &prefixLen);

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("After Prefix4 -> Prefix4->family : %d, Prefix4->prefixLen : %d, Prefix4->in_addr : %s\n\n", testPrefix4.family, testPrefix4.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixCheckIpv4Any() */
	printf("\n== nnPrefixCheckIpv4Any()\n");
	memset(&testPrefix4, 0x0, sizeof(testPrefix4));

	testPrefix4.family = AF_INET;
	testPrefix4.prefixLen = 0;
	testPrefix4.prefix.s_addr = 0x0;

	ret = nnPrefixCheckIpv4Any(&testPrefix4);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnApplyNetmasktoPrefix() */
	printf("\n== nnApplyNetmasktoPrefix()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));

	prefixLen = 24;
	testPrefix.family = AF_INET;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("Before Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);
	printf("PrefixLen : %d\n", prefixLen);

	nnApplyNetmasktoPrefix(&testPrefix, &prefixLen);

	strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("After Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnGetPrefixByteLen() */
	printf("\n== nnGetPrefixByteLen()\n");
	memset(&testPrefix, 0x0, sizeof(testPrefix));

	testPrefix.family = AF_INET;

	ret = nnGetPrefixByteLen(&testPrefix);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixNew() & nnPrefixFree()) */
	printf("\n== nnPrefixNew() & nnPrefixFree()\n");

	if (pTestPrefix != NULL)
	{
		free(pTestPrefix);
	}
	pTestPrefix = NULL;

	pTestPrefix = nnPrefixNew();

	if ((Int32T)pTestPrefix == -1)
	{
		printf("Make New Prefix Failure\n");
	}
	else
	{
		printf("Make New Prefix SUCCESS\n");
		nnPrefixFree(pTestPrefix);
		pTestPrefix = NULL;
		printf("Prefix Free SUCCESS\n");
	}

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCheckAllDigit() */
	printf("\n== nnCheckAllDigit()\n");

	ret = nnCheckAllDigit(strNum);

	printf("Result : %d\n", ret);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnApplyClassfulMaskIpv4() */
	printf("\n== nnApplyClassfulMaskIpv4()\n");
	memset(&testPrefix4, 0x0, sizeof(testPrefix4));

	testPrefix4.family = AF_INET;
	testPrefix4.prefix.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Before Prefix4 -> Prefix4->family : %d, Prefix4->prefixLen : %d, Prefix4->in_addr : %s\n\n", testPrefix4.family, testPrefix4.prefixLen, strIp);

	ret = nnApplyClassfulMaskIpv4(&testPrefix4);

	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Result : %s, After Prefix4 -> Prefix4->family : %d, Prefix4->prefixLen : %d, Prefix4->in_addr : %s\n\n", strResult[ret + 1], testPrefix4.family, testPrefix4.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnApplyPrefixLentoAddr() */
	printf("\n== nnApplyPrefixLentoAddr()\n");
	memset(&testAddr1, 0, sizeof(testAddr2));
	memset(&testAddr2, 0, sizeof(testAddr2));

	prefixLen = 24;
	testAddr2.s_addr = 0x0101A8C0;

	strIp = inet_ntoa(testAddr1);
	printf("Before testAddr1 : %s\n\n", strIp);
	strIp = inet_ntoa(testAddr2);
	printf("Apply Address : %s, PrefixLen : %d\n\n", strIp, prefixLen);

	ret = nnApplyPrefixLentoAddr(&testAddr1, &testAddr2, &prefixLen);

	strIp = inet_ntoa(testAddr1);
	printf("Result : %d\n", ret);
	printf("After Prefix4 -> Prefix4->family : %d, Prefix4->prefixLen : %d, Prefix4->in_addr : %s\n\n", testPrefix4.family, testPrefix4.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvNetmaskstrtoPrefixstr() */
	printf("\n== nnCnvNetmaskstrtoPrefixstr()\n");
	memset(&addrToStr, 0, sizeof(addrToStr));

	printf("Before addrToStr : %s\n\n", addrToStr);
	printf("Convert String -> Network : %s, Netmask : %s\n", ipv4, netmask);

	ret = nnCnvNetmaskstrtoPrefixstr(addrToStr, ipv4, netmask);

	printf("Result : %d\n", ret);
	printf("After addrToStr : %s\n", addrToStr);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnGetPrefixBit() */
	printf("\n== nnGetPrefixBit()\n");
	memset(&testAddr1, 0, sizeof(testAddr1));

	prefixLen = 24;
	testAddr1.s_addr = 0x0101A8C0;

	testUint = nnGetPrefixBit((Uint8T *)&testAddr1.s_addr, &prefixLen);

	printf("Result : %u\n", testUint);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV4_ADDR_CMP() */
	printf("\n== PREFIX_IPV4_ADDR_CMP(A, B)\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	memset(&testAddr2, 0, sizeof(testAddr2));

	testAddr1.s_addr = 0x0101A8C0;
	testAddr2.s_addr = 0x0101A8C0;

	printf("Result : %d\n", PREFIX_IPV4_ADDR_CMP(&testAddr1, &testAddr2));
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV4_ADDR_SAME() */
	printf("\n== PREFIX_IPV4_ADDR_SAME(A, B)\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	memset(&testAddr2, 0, sizeof(testAddr2));

	testAddr1.s_addr = 0x0101A8C0;
	testAddr2.s_addr = 0x0101A8C0;

	printf("Result : %d\n", PREFIX_IPV4_ADDR_SAME(&testAddr1, &testAddr2));
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV4_ADDR_COPY() */
	printf("\n== PREFIX_IPV4_ADDR_COPY(A, B)\n");
	memset(&testAddr1, 0, sizeof(testAddr1));
	memset(&testAddr2, 0, sizeof(testAddr2));

	testAddr2.s_addr = 0x0101A8C0;
	strIp = inet_ntoa(testAddr1);
	printf("Before : %s\n", strIp);

	if(PREFIX_IPV4_ADDR_COPY(&testAddr1, &testAddr2) == NULL)
	{
		printf("After Failure\n");
	}
	else
	{
		strIp = inet_ntoa(testAddr1);
		printf("After : %s\n", strIp);
	}
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV4_NET0() */
	printf("\n== PREFIX_IPV4_NET0(A)\n");
	memset(&testAddr1, 0, sizeof(testAddr1));

	testAddr1.s_addr = 0x0001A8C0;
	strIp = inet_ntoa(testAddr1);
	printf("Check IP : %s\n", strIp);

	ret = PREFIX_IPV4_NET0(testAddr1.s_addr);

	printf("Result : %d\n", ret);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV4_NET127() */
	printf("\n== PREFIX_IPV4_NET127(A)\n");
	memset(&testAddr1, 0, sizeof(testAddr1));

	testAddr1.s_addr = 0x7F01A8C0;
	strIp = inet_ntoa(testAddr1);
	printf("Check IP : %s\n", strIp);

	ret = PREFIX_IPV4_NET127(testAddr1.s_addr);

	printf("Result : %d\n", ret);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV4_LINKLOCAL() */
	printf("\n== PREFIX_IPV4_LINKLOCAL(A)\n");
	memset(&testAddr1, 0, sizeof(testAddr1));

	testAddr1.s_addr = 0xA9FEA8C0;
	strIp = inet_ntoa(testAddr1);
	printf("Check IP : %s\n", strIp);

	ret = PREFIX_IPV4_LINKLOCAL(testAddr1.s_addr);

	printf("Result : %d\n", ret);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_MASKBIT() */
	printf("\n== PREFIX_MASKBIT(OFFSET)\n");
	value = 8;

	ret32 = PREFIX_MASKBIT(value);

	printf("Result : %x\n", ret32);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_PSIZE() */
	printf("\n== PREFIX_SIZE(A)\n");
	value = 32;

	ret32 = PREFIX_PSIZE(value);

	printf("Result : %x\n", ret32);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_FAMILY() */
	printf("\n== PREFIX_FAMILY(P)\n");
	memset(&testPrefix, 0, sizeof(testPrefix));

	testPrefix.family = AF_INET;
	pTestPrefix = &testPrefix;

	ret = PREFIX_FAMILY(pTestPrefix);

	printf("Result : %d\n", ret);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_COPY_IPV4() */
	printf("\n== PREFIX_COPY_IPV4(D, S)\n");
	memset(&testPrefix, 0, sizeof(testPrefix));
	memset(&testPrefix2, 0, sizeof(testPrefix2));

	testPrefix.family = AF_INET;
	testPrefix.prefixLen = 24;
	testPrefix.u.prefix4.s_addr = 0x0101A8C0;

        strIp = inet_ntoa(testPrefix.u.prefix4);
	printf("Source Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix.family, testPrefix.prefixLen, strIp);

        strIp = inet_ntoa(testPrefix2.u.prefix4);
	printf("Dest Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix2.family, testPrefix2.prefixLen, strIp);

	PREFIX_COPY_IPV4(&testPrefix2, &testPrefix);

        strIp = inet_ntoa(testPrefix2.u.prefix4);
	printf("Dest Prefix2->family : %d, Prefix2->prefixLen : %d, Prefix2->in_addr : %s\n", testPrefix2.family, testPrefix2.prefixLen, strIp);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");

#if HAVE_IPV6
	/* nnCnvStringtoAddr6() */
	printf("\n== nnCnvStringtoAddr6(IPV6)\n");
	memset(&testAddr6, 0, sizeof(testAddr6));
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));
	inet_pton(AF_INET6, addr6ToStr, &testAddr6);
	printf("Before String IP : %s, struct in6_addr IP : %s\n\n", ipv6, addr6ToStr);

	ret = nnCnvStringtoAddr6(&testAddr6, ipv6);
	strIp = (StringT)inet_ntop(AF_INET6, &testAddr6, addr6ToStr, MAX_IPV6_LEN + 1);

	printf("After Result : %s, struct in6_addr IP : %s\n", strResult[ret + 1], addr6ToStr);

        printf("-----------------------------------------------------------\n");



	printf("\n== nnCnvStringtoAddr6(IPV6_PREFIX)\n");
	memset(&testAddr6, 0x0, sizeof(struct in6_addr));
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));
	inet_pton(AF_INET6, addr6ToStr, &testAddr6);
	printf("Before String IP : %s, struct in6_addr IP : %s\n\n", ipv6, addr6ToStr);

	ret = nnCnvStringtoAddr6(&testAddr6, ipv6nprefix);

	strIp = (StringT)inet_ntop(AF_INET6, &testAddr6, addr6ToStr, MAX_IPV6_LEN + 1);

	printf("After Result : %s, struct in6_addr IP : %s\n", strResult[ret + 1], addr6ToStr);

        printf("-----------------------------------------------------------\n");



	/* nnCnvAddr6toString() */
	printf("\n== nnCnvAddr6toString()\n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));
	memset(&testAddr6, 0, sizeof(testAddr6));
	inet_pton(AF_INET6, addr6ToStr, &testAddr6);
	printf("Before String IPV6 : %s\n\n", addr6ToStr);

	testAddr6.s6_addr[0] = 0x11;	testAddr6.s6_addr[1] = 0x11;	testAddr6.s6_addr[2] = 0x22;	testAddr6.s6_addr[3] = 0x22;
	testAddr6.s6_addr[4] = 0x33;	testAddr6.s6_addr[5] = 0x33;	testAddr6.s6_addr[6] = 0x44;	testAddr6.s6_addr[7] = 0x44;
	testAddr6.s6_addr[8] = 0x55;	testAddr6.s6_addr[9] = 0x55;	testAddr6.s6_addr[10] = 0x66;	testAddr6.s6_addr[11] = 0x66;
	testAddr6.s6_addr[12] = 0x77;	testAddr6.s6_addr[13] = 0x77;	testAddr6.s6_addr[14] = 0x88;	testAddr6.s6_addr[15] = 0x88;

	ret = nnCnvAddr6toString(addr6ToStr, &testAddr6);

	printf("After Result : %s, String IP : %s\n", strResult[ret + 1], addr6ToStr);
        printf("=================================================================================================================================\n");



	/* nnCnvStringtoPrefix6() */
	printf("\n== nnCnvStringtoPrefix6(IPV6)\n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr)); 
	memset(&testPrefix6, 0, sizeof(testPrefix6));
	strIp = NULL;
	printf("String IP : %s\n", ipv6);
	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Before Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in6_addr : %s\n\n", testPrefix6.family, testPrefix6.prefixLen, strIp);

	ret = nnCnvStringtoPrefix6(&testPrefix6, ipv6);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("After Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in6_addr : %s\n\n", testPrefix6.family, testPrefix6.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");



	printf("\n== nnCnvStringtoPrefix6(IPV6_PREFIX) \n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr)); 
	memset(&testPrefix6, 0, sizeof(testPrefix6));
	strIp = NULL;
	printf("String IP : %s\n", ipv6);
	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Before Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in6_addr : %s\n\n", testPrefix6.family, testPrefix6.prefixLen, strIp);

	ret = nnCnvStringtoPrefix6(&testPrefix6, ipv6nprefix);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("After Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in6_addr : %s\n\n", testPrefix6.family, testPrefix6.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefix6toString() */
	printf("\n== nnCnvPrefix6toString(IPV6)\n");
	memset(addr6ToStr, 0, sizeof(addr6ToStr));
	memset(&testPrefix6, 0, sizeof(testPrefix6));
	strIp = NULL;

	testPrefix6.prefixLen = 100;
	testPrefix6.prefix.s6_addr[0] = 0x11;	testPrefix6.prefix.s6_addr[1] = 0x11;	testPrefix6.prefix.s6_addr[2] = 0x22;
	testPrefix6.prefix.s6_addr[3] = 0x22;	testPrefix6.prefix.s6_addr[4] = 0x33;	testPrefix6.prefix.s6_addr[5] = 0x33;
	testPrefix6.prefix.s6_addr[6] = 0x44;	testPrefix6.prefix.s6_addr[7] = 0x44;	testPrefix6.prefix.s6_addr[8] = 0x55;
	testPrefix6.prefix.s6_addr[9] = 0x55;	testPrefix6.prefix.s6_addr[10] = 0x66;	testPrefix6.prefix.s6_addr[11] = 0x66;
	testPrefix6.prefix.s6_addr[12] = 0x77;	testPrefix6.prefix.s6_addr[13] = 0x77;	testPrefix6.prefix.s6_addr[14] = 0x88;
	testPrefix6.prefix.s6_addr[15] = 0x88;

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	memset(addr6ToStr, 0, sizeof(addr6ToStr));

	printf("Before Prefix6T.prefix IP : %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x -> String IP : %s\n\n", testPrefix6.prefix.s6_addr[0], testPrefix6.prefix.s6_addr[1], testPrefix6.prefix.s6_addr[2], testPrefix6.prefix.s6_addr[3], testPrefix6.prefix.s6_addr[4], testPrefix6.prefix.s6_addr[5], testPrefix6.prefix.s6_addr[6], testPrefix6.prefix.s6_addr[7], testPrefix6.prefix.s6_addr[8], testPrefix6.prefix.s6_addr[9], testPrefix6.prefix.s6_addr[10], testPrefix6.prefix.s6_addr[11], testPrefix6.prefix.s6_addr[12],testPrefix6.prefix.s6_addr[13], testPrefix6.prefix.s6_addr[14], testPrefix6.prefix.s6_addr[15], addr6ToStr);

	ret = nnCnvPrefix6toString(addr6ToStr, &testPrefix6);

	printf("After Result : %s, Prefix6T.prefix IP : %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x -> String IP : %s\n", strResult[ret + 1], testPrefix6.prefix.s6_addr[0], testPrefix6.prefix.s6_addr[1], testPrefix6.prefix.s6_addr[2], testPrefix6.prefix.s6_addr[3], testPrefix6.prefix.s6_addr[4], testPrefix6.prefix.s6_addr[5], testPrefix6.prefix.s6_addr[6], testPrefix6.prefix.s6_addr[7], testPrefix6.prefix.s6_addr[8], testPrefix6.prefix.s6_addr[9], testPrefix6.prefix.s6_addr[10], testPrefix6.prefix.s6_addr[11], testPrefix6.prefix.s6_addr[12],testPrefix6.prefix.s6_addr[13], testPrefix6.prefix.s6_addr[14], testPrefix6.prefix.s6_addr[15], addr6ToStr);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefix6TtoPrefixT() */
	printf("\n== nnCnvPrefix6ToPrefixT()\n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));
	memset(&testPrefix, 0, sizeof(testPrefix));
	memset(&testPrefix6, 0, sizeof(testPrefix6));

	testPrefix6.family = AF_INET6;
	testPrefix6.prefixLen = 100;
	testPrefix6.prefix.s6_addr[0] = 0x11;	testPrefix6.prefix.s6_addr[1] = 0x11;	testPrefix6.prefix.s6_addr[2] = 0x22;
	testPrefix6.prefix.s6_addr[3] = 0x22;	testPrefix6.prefix.s6_addr[4] = 0x33;	testPrefix6.prefix.s6_addr[5] = 0x33;
	testPrefix6.prefix.s6_addr[6] = 0x44;	testPrefix6.prefix.s6_addr[7] = 0x44;	testPrefix6.prefix.s6_addr[8] = 0x55;
	testPrefix6.prefix.s6_addr[9] = 0x55;	testPrefix6.prefix.s6_addr[10] = 0x66;	testPrefix6.prefix.s6_addr[11] = 0x66;
	testPrefix6.prefix.s6_addr[12] = 0x77;	testPrefix6.prefix.s6_addr[13] = 0x77;	testPrefix6.prefix.s6_addr[14] = 0x88;
	testPrefix6.prefix.s6_addr[15] = 0x88;

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in6_addr : %s\n\n", testPrefix6.family, testPrefix6.prefixLen, strIp);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix.u.prefix6, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Before Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);

	ret = nnCnvPrefix6TtoPrefixT(&testPrefix, &testPrefix6);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix.u.prefix6, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("After Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", testPrefix.family, testPrefix.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefixTtoPrefix6T() */
	printf("\n== nnCnvPrefixToPrefix6T()\n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));
	memset(&testPrefix, 0, sizeof(testPrefix));
	memset(&testPrefix6, 0, sizeof(testPrefix6));

	testPrefix.family = AF_INET6;
	testPrefix.prefixLen = 100;
	testPrefix.u.prefix6.s6_addr[0] = 0x11;	testPrefix.u.prefix6.s6_addr[1] = 0x11;	testPrefix.u.prefix6.s6_addr[2] = 0x22;
	testPrefix.u.prefix6.s6_addr[3] = 0x22;	testPrefix.u.prefix6.s6_addr[4] = 0x33;	testPrefix.u.prefix6.s6_addr[5] = 0x33;
	testPrefix.u.prefix6.s6_addr[6] = 0x44;	testPrefix.u.prefix6.s6_addr[7] = 0x44;	testPrefix.u.prefix6.s6_addr[8] = 0x55;
	testPrefix.u.prefix6.s6_addr[9] = 0x55;	testPrefix.u.prefix6.s6_addr[10] = 0x66;	testPrefix.u.prefix6.s6_addr[11] = 0x66;
	testPrefix.u.prefix6.s6_addr[12] = 0x77;	testPrefix.u.prefix6.s6_addr[13] = 0x77;	testPrefix.u.prefix6.s6_addr[14] = 0x88;
	testPrefix.u.prefix6.s6_addr[15] = 0x88;

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix.u.prefix6.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Prefix -> Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in6_addr : %s\n", testPrefix.family, testPrefix.prefixLen, strIp);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Before Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in_addr : %s\n\n", testPrefix6.family, testPrefix6.prefixLen, strIp);

	ret = nnCnvPrefixTtoPrefix6T(&testPrefix6, &testPrefix);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("After Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in_addr : %s\n", testPrefix6.family, testPrefix6.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnGetPrefix6Bit() */
	printf("\n== nnGetPrefix6Bit()\n");
	memset(&testAddr6, 0, sizeof(testAddr6));

	prefixLen = 100;
	testAddr6.s6_addr[0] = 0x11;	testAddr6.s6_addr[1] = 0x11;	testAddr6.s6_addr[2] = 0x22;	testAddr6.s6_addr[3] = 0x22;
	testAddr6.s6_addr[4] = 0x33;	testAddr6.s6_addr[5] = 0x33;	testAddr6.s6_addr[6] = 0x44;	testAddr6.s6_addr[7] = 0x44;
	testAddr6.s6_addr[8] = 0x55;	testAddr6.s6_addr[9] = 0x55;	testAddr6.s6_addr[10] = 0x66;	testAddr6.s6_addr[11] = 0x66;
	testAddr6.s6_addr[12] = 0x77;	testAddr6.s6_addr[13] = 0x77;	testAddr6.s6_addr[14] = 0x88;	testAddr6.s6_addr[15] = 0x88;

	testUint = nnGetPrefix6Bit(&testAddr6, &prefixLen);

	printf("Result : %u\n", testUint);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnPrefixIpv6New() & nnPrefixIpv6Free()) */
	printf("\n== nnPrefixIpv6New() & nnPrefixIpv6Free()\n");

	if (pTestPrefix6 != NULL)
	{
		free(pTestPrefix6);
	}
	pTestPrefix6 = NULL;

	pTestPrefix6 = nnPrefixIpv6New();

	if ((Int32T)pTestPrefix6 == -1)
	{
		printf("Make New Prefix6 Failure\n");
	}
	else
	{
		printf("Make New Prefix6 SUCCESS\n");
		nnPrefixIpv6Free(pTestPrefix6);
		pTestPrefix6 = NULL;
		printf("Prefix6 Free SUCCESS\n");
	}

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnApplyNetmasktoPrefix6() */
	printf("\n== nnApplyNetmasktoPrefix6()\n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));
	memset(&testPrefix6, 0, sizeof(testPrefix6));

	prefixLen = 100;
	testPrefix6.prefix.s6_addr[0] = 0x11;   testPrefix6.prefix.s6_addr[1] = 0x11;   testPrefix6.prefix.s6_addr[2] = 0x22;
	testPrefix6.prefix.s6_addr[3] = 0x22;   testPrefix6.prefix.s6_addr[4] = 0x33;   testPrefix6.prefix.s6_addr[5] = 0x33;
	testPrefix6.prefix.s6_addr[6] = 0x44;   testPrefix6.prefix.s6_addr[7] = 0x44;   testPrefix6.prefix.s6_addr[8] = 0x55;
	testPrefix6.prefix.s6_addr[9] = 0x55;   testPrefix6.prefix.s6_addr[10] = 0x66;  testPrefix6.prefix.s6_addr[11] = 0x66;
	testPrefix6.prefix.s6_addr[12] = 0x77;  testPrefix6.prefix.s6_addr[13] = 0x77;  testPrefix6.prefix.s6_addr[14] = 0x88;
	testPrefix6.prefix.s6_addr[15] = 0x88;

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Before Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in6_addr : %s\n", testPrefix6.family, testPrefix6.prefixLen, strIp);
	printf("PrefixLen : %d\n\n", prefixLen);

	nnApplyNetmasktoPrefix6(&testPrefix6, &prefixLen);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix6.prefix.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("After Prefix6 -> Prefix6->family : %d, Prefix6->prefixLen : %d, Prefix6->in6_addr : %s\n", testPrefix6.family, testPrefix6.prefixLen, strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvNetmask6toPrefixLen() */
	printf("\n== nnCnvNetmask6toPrefixLen()\n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));

	prefixLen = 0;
	testAddr6.s6_addr[0] = 0xFF;	testAddr6.s6_addr[1] = 0xFF;	testAddr6.s6_addr[2] = 0xFF;	testAddr6.s6_addr[3] = 0xFF;
	testAddr6.s6_addr[4] = 0xFF;	testAddr6.s6_addr[5] = 0xFF;	testAddr6.s6_addr[6] = 0xFF;	testAddr6.s6_addr[7] = 0xFF;
	testAddr6.s6_addr[8] = 0xFF;	testAddr6.s6_addr[9] = 0xFF;	testAddr6.s6_addr[10] = 0xFF;	testAddr6.s6_addr[11] = 0xFF;
	testAddr6.s6_addr[12] = 0xF0;	testAddr6.s6_addr[13] = 0x00;	testAddr6.s6_addr[14] = 0x00;	testAddr6.s6_addr[15] = 0x00;

	strIp = (StringT)inet_ntop(AF_INET6, &testAddr6.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
        printf("Netmask : %s\n", strIp);
	printf("Before PrefixLen : %d\n\n", prefixLen);

	nnCnvNetmask6toPrefixLen(&prefixLen, &testAddr6);

	printf("After Result : %s, PrefixLen : %d\n", strResult[ret + 1], prefixLen);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* nnCnvPrefixLentoNetmask6() */
	printf("\n== nnCnvPrefixLentoNetmask6()\n");
	memset(&testAddr6, 0, sizeof(testAddr6));

	prefixLen = 100;

	strIp = (StringT)inet_ntop(AF_INET6, &testAddr6.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("PrefixLen : %d\n\n", prefixLen);
        printf("Before Netmask : %s\n", strIp);

	ret = nnCnvPrefixLentoNetmask6(&testAddr6, &prefixLen);

	strIp = (StringT)inet_ntop(AF_INET6, &testAddr6.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
        printf("After Netmask : %s\n", strIp);
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV6_ADDR_CMP() */
	printf("\n== PREFIX_IPV6_ADDR_CMP(A, B)\n");
	memset(&testAddr6, 0, sizeof(testAddr6));
	memset(&testAddr7, 0, sizeof(testAddr7));

	testAddr6.s6_addr[0] = 0x11;	testAddr6.s6_addr[1] = 0x11;	testAddr6.s6_addr[2] = 0x22;	testAddr6.s6_addr[3] = 0x22;
	testAddr6.s6_addr[4] = 0x33;	testAddr6.s6_addr[5] = 0x33;	testAddr6.s6_addr[6] = 0x44;	testAddr6.s6_addr[7] = 0x44;
	testAddr6.s6_addr[8] = 0x55;	testAddr6.s6_addr[9] = 0x55;	testAddr6.s6_addr[10] = 0x66;	testAddr6.s6_addr[11] = 0x66;
	testAddr6.s6_addr[12] = 0x77;	testAddr6.s6_addr[13] = 0x77;	testAddr6.s6_addr[14] = 0x88;	testAddr6.s6_addr[15] = 0x88;

	testAddr7.s6_addr[0] = 0x11;	testAddr7.s6_addr[1] = 0x11;	testAddr7.s6_addr[2] = 0x22;	testAddr7.s6_addr[3] = 0x22;
	testAddr7.s6_addr[4] = 0x33;	testAddr7.s6_addr[5] = 0x33;	testAddr7.s6_addr[6] = 0x44;	testAddr7.s6_addr[7] = 0x44;
	testAddr7.s6_addr[8] = 0x55;	testAddr7.s6_addr[9] = 0x55;	testAddr7.s6_addr[10] = 0x66;	testAddr7.s6_addr[11] = 0x66;
	testAddr7.s6_addr[12] = 0x77;	testAddr7.s6_addr[13] = 0x77;	testAddr7.s6_addr[14] = 0x88;	testAddr7.s6_addr[15] = 0x88;

	printf("Result : %d\n", PREFIX_IPV6_ADDR_CMP(&testAddr6, &testAddr7));
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV6_ADDR_SAME() */
	printf("\n== PREFIX_IPV6_ADDR_SAME(A, B)\n");
	memset(&testAddr6, 0, sizeof(testAddr6));
	memset(&testAddr7, 0, sizeof(testAddr7));

	testAddr6.s6_addr[0] = 0x11;	testAddr6.s6_addr[1] = 0x11;	testAddr6.s6_addr[2] = 0x22;	testAddr6.s6_addr[3] = 0x22;
	testAddr6.s6_addr[4] = 0x33;	testAddr6.s6_addr[5] = 0x33;	testAddr6.s6_addr[6] = 0x44;	testAddr6.s6_addr[7] = 0x44;
	testAddr6.s6_addr[8] = 0x55;	testAddr6.s6_addr[9] = 0x55;	testAddr6.s6_addr[10] = 0x66;	testAddr6.s6_addr[11] = 0x66;
	testAddr6.s6_addr[12] = 0x77;	testAddr6.s6_addr[13] = 0x77;	testAddr6.s6_addr[14] = 0x88;	testAddr6.s6_addr[15] = 0x88;

	testAddr7.s6_addr[0] = 0x11;	testAddr7.s6_addr[1] = 0x11;	testAddr7.s6_addr[2] = 0x22;	testAddr7.s6_addr[3] = 0x22;
	testAddr7.s6_addr[4] = 0x33;	testAddr7.s6_addr[5] = 0x33;	testAddr7.s6_addr[6] = 0x44;	testAddr7.s6_addr[7] = 0x44;
	testAddr7.s6_addr[8] = 0x55;	testAddr7.s6_addr[9] = 0x55;	testAddr7.s6_addr[10] = 0x66;	testAddr7.s6_addr[11] = 0x66;
	testAddr7.s6_addr[12] = 0x77;	testAddr7.s6_addr[13] = 0x77;	testAddr7.s6_addr[14] = 0x88;	testAddr7.s6_addr[15] = 0x88;

	printf("Result : %d\n", PREFIX_IPV6_ADDR_SAME(&testAddr6, &testAddr7));
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_IPV6_ADDR_COPY() */
	printf("\n== PREFIX_IPV6_ADDR_COPY(A, B)\n");
	memset(&addr6ToStr, 0, sizeof(addr6ToStr));
	memset(&testAddr6, 0, sizeof(testAddr6));
	memset(&testAddr7, 0, sizeof(testAddr7));

	testAddr7.s6_addr[0] = 0x11;	testAddr7.s6_addr[1] = 0x11;	testAddr7.s6_addr[2] = 0x22;	testAddr7.s6_addr[3] = 0x22;
	testAddr7.s6_addr[4] = 0x33;	testAddr7.s6_addr[5] = 0x33;	testAddr7.s6_addr[6] = 0x44;	testAddr7.s6_addr[7] = 0x44;
	testAddr7.s6_addr[8] = 0x55;	testAddr7.s6_addr[9] = 0x55;	testAddr7.s6_addr[10] = 0x66;	testAddr7.s6_addr[11] = 0x66;
	testAddr7.s6_addr[12] = 0x77;	testAddr7.s6_addr[13] = 0x77;	testAddr7.s6_addr[14] = 0x88;	testAddr7.s6_addr[15] = 0x88;


	strIp = (StringT)inet_ntop(AF_INET6, &testAddr6, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Before : %s\n\n", strIp);

	if(PREFIX_IPV6_ADDR_COPY(&testAddr6, &testAddr7) == NULL)
	{
		printf("After Failure\n");
	}
	else
	{
		strIp = (StringT)inet_ntop(AF_INET6, &testAddr6, addr6ToStr, MAX_IPV6_LEN + 1);
		printf("After : %s\n", strIp);
	}
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");



	/* PREFIX_COPY_IPV6() */
	printf("\n== PREFIX_COPY_IPV6(D, S)\n");
	memset(&testPrefix, 0, sizeof(testPrefix));
	memset(&testPrefix2, 0, sizeof(testPrefix2));

	testPrefix2.family = AF_INET6;
	testPrefix2.prefixLen = 100;
	testPrefix2.u.prefix6.s6_addr[0] = 0x11;	testPrefix2.u.prefix6.s6_addr[1] = 0x11;	testPrefix2.u.prefix6.s6_addr[2] = 0x22;
	testPrefix2.u.prefix6.s6_addr[3] = 0x22;	testPrefix2.u.prefix6.s6_addr[4] = 0x33;	testPrefix2.u.prefix6.s6_addr[5] = 0x33;
	testPrefix2.u.prefix6.s6_addr[6] = 0x44;	testPrefix2.u.prefix6.s6_addr[7] = 0x44;	testPrefix2.u.prefix6.s6_addr[8] = 0x55;
	testPrefix2.u.prefix6.s6_addr[9] = 0x55;	testPrefix2.u.prefix6.s6_addr[10] = 0x66;	testPrefix2.u.prefix6.s6_addr[11] = 0x66;
	testPrefix2.u.prefix6.s6_addr[12] = 0x77;	testPrefix2.u.prefix6.s6_addr[13] = 0x77;	testPrefix2.u.prefix6.s6_addr[14] = 0x88;
	testPrefix2.u.prefix6.s6_addr[15] = 0x88;

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix2.u.prefix6.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Source Prefix2 -> Prefix2->family : %d, Prefix2->prefixLen : %d, Prefix2->in6_addr : %s\n\n", testPrefix2.family, testPrefix2.prefixLen, strIp);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix.u.prefix6.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Dest Prefix1 -> Prefix1->family : %d, Prefix1->prefixLen : %d, Prefix1->in6_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);
	PREFIX_COPY_IPV6(&testPrefix, &testPrefix2);

	strIp = (StringT)inet_ntop(AF_INET6, &testPrefix.u.prefix6.s6_addr, addr6ToStr, MAX_IPV6_LEN + 1);
	printf("Dest Prefix1 -> Prefix1->family : %d, Prefix1->prefixLen : %d, Prefix1->in6_addr : %s\n\n", testPrefix.family, testPrefix.prefixLen, strIp);

        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");

#endif /* HAVE_IPV6 */

	nnLogClose();
	memClose();

	return 0;
}
#if 0
	/* NewPrefix from Addr */
	printf("\n== nnNewPrefixfromAddr\n");

	prefixLen = 10;
	pTestPrefix = NULL;
	memset(&testPrefix4, 0x0, sizeof(Prefix4T));
	ret = nnCnvStringtoPrefix4(&testPrefix4, ipv4);
	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Address : %s\n", strIp);
	printf("Before Prefix : %p\n", pTestPrefix);

	pTestPrefix = nnNewPrefixfromAddr(&testPrefix4.prefix, &prefixLen);

	if (!((Int32T)pTestPrefix < 0))
	{
		strIp = inet_ntoa(pTestPrefix->u.prefix4);
		printf("After Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", pTestPrefix->family, pTestPrefix->prefixLen, strIp);

		free(pTestPrefix);
		pTestPrefix = NULL;
	}
	else
	{
		printf("nnNewPrefixfromAddr Fail : %d %p\n", (Int32T)pTestPrefix, pTestPrefix);
	}
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");

	/* NewPrefix from Prefix4 */
	printf("\n== nnNewPrefixfromPrefix4\n");

	prefixLen = 11;
	pTestPrefix = NULL;
	memset(&testPrefix4, 0x0, sizeof(Prefix4T));

	testPrefix4.family = AF_INET;
	testPrefix4.prefixLen = 24;
	ret = nnCnvStringtoPrefix4(&testPrefix4, ipv4);
	strIp = inet_ntoa(testPrefix4.prefix);
	printf("Prefix4.family : %d, Prefix4.prefixLen : %d, Prefix4.in_addr : %s\n", testPrefix4.family, testPrefix4.prefixLen, strIp);
	printf("Before Prefix : %p\n", pTestPrefix);

	pTestPrefix = nnNewPrefixfromPrefix4(&testPrefix4, &prefixLen);

	if (!((Int32T)pTestPrefix < 0))
	{
		strIp = inet_ntoa(pTestPrefix->u.prefix4);
		printf("After Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", pTestPrefix->family, pTestPrefix->prefixLen, strIp);

		free(pTestPrefix);
		pTestPrefix = NULL;
	}
	else
	{
		printf("nnNewPrefixfromPrefix4 Fail : %d %p\n", (Int32T)pTestPrefix, pTestPrefix);
	}
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");

/* NewPrefix from String */
	printf("\n== nnNewPrefixfromString(IPV4)\n");

	prefixLen = 12;
	pTestPrefix = NULL;
	printf("Address : %s\n", ipv4);
	printf("Before Prefix : %p\n", pTestPrefix);

	pTestPrefix = nnNewPrefixfromString(ipv4, &prefixLen);

	if (!((Int32T)pTestPrefix < 0))
	{
		strIp = inet_ntoa(pTestPrefix->u.prefix4);
		printf("After Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", pTestPrefix->family, pTestPrefix->prefixLen, strIp);

		free(pTestPrefix);
		pTestPrefix = NULL;
	}
	else
	{
		printf("nnNewPrefixfromString Fail : %d %p\n", (Int32T)pTestPrefix, pTestPrefix);
	}
        printf("-----------------------------------------------------------\n");

	printf("\n== nnNewPrefixfromString(IPV4_PREFIX)\n");

	prefixLen = 13;
	pTestPrefix = NULL;
	pTestPrefix = NULL;
	printf("Address : %s\n", ipv4nprefix);
	printf("Before Prefix : %p\n", pTestPrefix);

	pTestPrefix = nnNewPrefixfromString(ipv4nprefix, &prefixLen);
	if (!((Int32T)pTestPrefix < 0))
	{
		strIp = inet_ntoa(pTestPrefix->u.prefix4);
		printf("After Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", pTestPrefix->family, pTestPrefix->prefixLen, strIp);

		free(pTestPrefix);
		pTestPrefix = NULL;
	}
	else
	{
		printf("nnNewPrefixfromString Fail : %d %p\n", (Int32T)pTestPrefix, pTestPrefix);
	}
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");

/* NewPrefix4 from Uint */
	printf("\n== nnNewPrefix4fromUint\n");

	prefixLen = 14;
	pTestPrefix = NULL;
//	testUint = 3232235777U; /* 192.168.1.1 */
	testUint = 16843009U; /* 1.1.1.1 */
	printf("Uint : %u\n", testUint);
	printf("Before Prefix4 : %p\n", pTestPrefix4);

	pTestPrefix4 = nnNewPrefix4fromUint(&testUint, &prefixLen);

	if (!((Int32T)pTestPrefix4 < 0))
	{
		strIp = inet_ntoa(pTestPrefix4->prefix);
		printf("After Prefix->family : %d, Prefix->prefixLen : %d, Prefix->in_addr : %s\n", pTestPrefix4->family, pTestPrefix4->prefixLen, strIp);

		free(pTestPrefix4);
		pTestPrefix4 = NULL;
	}
	else
	{
		printf("nnNewPrefix4fromUint Fail : %d %p\n", (Int32T)pTestPrefix4, pTestPrefix4);
	}
        printf("-----------------------------------------------------------\n");
        printf("=================================================================================================================================\n");
#endif /* 0 */

