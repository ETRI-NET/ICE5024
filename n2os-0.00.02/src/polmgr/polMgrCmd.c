#include <stdio.h>
#include <string.h>
#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdInstall.h"

extern int polOamSwitch;	//global variable used for turning on/off the cmdOamPrint() function tester.

DECMD(cmdPolMgrTestDupFunc,
  CMD_NODE_EXEC,
  IPC_POL_MGR,
  "show abc def",
  "Polmgr Show Info",
  "Polmgr abc Help",
  "Polmgr def Help")
{
  cmdPrint(cmsh,"[%s]\n", __func__);
  return CMD_IPC_OK;
}

//DENODEC format Sample.
//The sample's DENODEC is mapped with vty-node. As a rusult, when user try to enter vty-node, this code is executed.
DENODEC(cmdPolFuncCheckVty,
  CMD_NODE_VTY, IPC_POL_MGR)
{
  Int32T i;
  printf("uargc1[%d]\n", uargc1);
  for(i = 0; i < uargc1; i++)
  {
    fprintf(stderr, "uargv1[%d] = [%s]\n", i, uargv1[i]);
  }
  fprintf(stderr, "uargc2[%d]\n", uargc2);
  for(i = 0; i < uargc2; i++)
  {
    fprintf(stderr, "uargv2[%d] = [%s]\n", i, uargv2[i]);
  }
  fprintf(stderr, "uargc3[%d]\n", uargc3);
  for(i = 0; i < uargc3; i++)
  {
    fprintf(stderr, "uargv3[%d] = [%s]\n", i, uargv3[i]);
  }
  fprintf(stderr, "uargc4[%d]\n", uargc4);
  for(i = 0; i < uargc4; i++)
  {
    fprintf(stderr, "uargv4[%d] = [%s]\n", i, uargv4[i]);
  }
  fprintf(stderr, "uargc5[%d]\n", uargc5);
  for(i = 0; i < uargc5; i++)
  {
    fprintf(stderr, "uargv5[%d] = [%s]\n", i, uargv5[i]);
  }
  fprintf(stderr, "cargc[%d]\n", cargc);
  for(i = 0; i < cargc; i++)
  {
    fprintf(stderr, "cargv[%d] = [%s]\n", i, cargv[i]);
  }

  if (polOamSwitch == 0)
  {
  /**since DENODE can only received one line. cmdPrint() cannot be used.
   * Please, use 'cmsh->outputStr = desired_string' to print the desired_string in the CMSH that inputed Node-Command.
   * In case one need to print multiple line, please use '\n'
   */
    cmsh->outputStr = strdup("Error Change Node\n[This is a result of sample DENODEC in polMgrCmd.c file]\n");

    return CMD_IPC_ERROR;
  }
  else
  {
    cmsh->outputStr = strdup("node change successful.");
    return CMD_IPC_OK;
  }
}

//Tester code to switch cmdOamPrint() function tester
DECMD(cmdPolFuncRipOamSwitch,
    CMD_NODE_CONFIG,
    IPC_POL_MGR,
    "test oam (on|off)",
    "Tester Command for Set-Commands",
    "oam related test commands",
    "turn on the oamPrintf() function tester",
    "turn off the oamPrintf() function tester (Default)")
{
  if(!strcmp(cargv[2], "on"))
  {
    polOamSwitch = 1;
    cmdPrint(cmsh, "tester for oamPrintf() function is turned on");
  }
  else
  {
    polOamSwitch = 0;
    cmdPrint(cmsh, "tester for oamPrintf() function is turned off");
  }
  return CMD_IPC_OK;
}

//Show-Command Example.
/**
 * "|IPC_SHOW_MGR" was added to make this command a Show-Command
 * cargc  : number of input in command line
 *  cargv  : inputs in the command line
 * uargc  : number of input in parants node
 *  uargv  : inputs in the parants node
 */
DECMD(cmdPollShowTest,
  CMD_NODE_VIEW,
  IPC_POL_MGR|IPC_SHOW_MGR,
  "show pol",
  "Show information",
  "POLL Manager")
{
  Int32T repeat;
  for(repeat = 0; repeat < 100000; repeat++)
  {
  cmdPrint(cmsh,"this has repeated %d times.\n", repeat+1);
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;

  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < uargc3; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv3[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv3[i]);
  }
  for(i = 0; i < uargc4; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv4[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv4[i]);
  }
  for(i = 0; i < uargc5; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv5[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv5[i]);
  }

  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
  cmdPrint(cmsh,"End");
  }
  return CMD_IPC_OK;
}

//below 3 DECMDs are set-command Example
/**
 * cargc  : number of input in command line
 *  cargv  : inputs in the command line
 * uargc  : number of input in parants node
 *  uargv  : inputs in the parants node
 */
DECMD(cmdPolFuncRipNoRouterRip1,
    CMD_NODE_CONFIG,
    IPC_POL_MGR,
    "test router rip WOLD",
    "Tester Command for Set-Commands",
    "Testing 2nd word(router) in Set-Command",
    "Testing 3rd word(rip) in Set-Command",
    "Testing 4th word(any-string) in Set-Command")
{
  cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  printf("Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < uargc3; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv3[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv3[i]);
  }
  for(i = 0; i < uargc4; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv4[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv4[i]);
  }
  for(i = 0; i < uargc5; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv5[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv5[i]);
  }

  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
    printf("Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
  cmdPrint(cmsh,"End");
  printf("End\n");
  return CMD_IPC_OK;
}
DECMD(cmdPolFuncRipNoRouterRip2,
    CMD_NODE_CONFIG,
    IPC_POL_MGR,
    "test running WOLD",
    "Tester Command for Set-Commands",
    "Testing 2nd word(running) in Set-Command",
    "Testing 3rd word(any-string) in Set-Command")
{
  cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  printf("Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < uargc3; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv3[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv3[i]);
  }
  for(i = 0; i < uargc4; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv4[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv4[i]);
  }
  for(i = 0; i < uargc5; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv5[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv5[i]);
  }

  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
    printf("Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
  cmdPrint(cmsh,"End");
  printf("End\n");
  return CMD_IPC_OK;
}
DECMD(cmdPolFuncRipNoRouterRip3,
    CMD_NODE_CONFIG_TEST,
    IPC_POL_MGR,
    "test node WOLD",
    "Tester Command for Set-Commands",
    "Testing Command within interface node",
    "Testing 3rd word(any-string) in Set-Command")
{
  Int32T i;
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc1[%d]\n", __FILE__, __func__, __LINE__, uargc1);
  printf("Enter [%s][%s][%d] Parent Node's uargc1[%d]\n", __FILE__, __func__, __LINE__, uargc1);

  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc2[%d]\n", __FILE__, __func__, __LINE__, uargc2);
  printf("Enter [%s][%s][%d] Parent Node's uargc2[%d]\n", __FILE__, __func__, __LINE__, uargc2);
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc3[%d]\n", __FILE__, __func__, __LINE__, uargc3);
  printf("Enter [%s][%s][%d] Parent Node's uargc3[%d]\n", __FILE__, __func__, __LINE__, uargc3);
  for(i = 0; i < uargc3; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv3[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv3[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv3[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv3[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc4[%d]\n", __FILE__, __func__, __LINE__, uargc4);
  printf("Enter [%s][%s][%d] Parent Node's uargc4[%d]\n", __FILE__, __func__, __LINE__, uargc4);
  for(i = 0; i < uargc4; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv4[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv4[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv4[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv4[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc5[%d]\n", __FILE__, __func__, __LINE__, uargc5);
  printf("Enter [%s][%s][%d] Parent Node's uargc5[%d]\n", __FILE__, __func__, __LINE__, uargc5);
  for(i = 0; i < uargc5; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv5[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv5[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv5[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv5[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  printf("Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
    printf("Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
  cmdPrint(cmsh,"End");
  printf("End\n");
  return CMD_IPC_OK;
}

//Test code for multi node
DECMD(cmdPolFuncRipNoRouterRipmul,
    CMD_NODE_CONFIG_TEST_MUL,
    IPC_POL_MGR,
    "test mul  WOLD",
    "Tester Command for Set-Commands",
    "Testing Command within interface node",
    "Testing 3rd word(any-string) in Set-Command")
{
  Int32T i;
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc1[%d]\n", __FILE__, __func__, __LINE__, uargc1);
  printf("Enter [%s][%s][%d] Parent Node's uargc1[%d]\n", __FILE__, __func__, __LINE__, uargc1);

  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc2[%d]\n", __FILE__, __func__, __LINE__, uargc2);
  printf("Enter [%s][%s][%d] Parent Node's uargc2[%d]\n", __FILE__, __func__, __LINE__, uargc2);
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc3[%d]\n", __FILE__, __func__, __LINE__, uargc3);
  printf("Enter [%s][%s][%d] Parent Node's uargc3[%d]\n", __FILE__, __func__, __LINE__, uargc3);
  for(i = 0; i < uargc3; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv3[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv3[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv3[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv3[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc4[%d]\n", __FILE__, __func__, __LINE__, uargc4);
  printf("Enter [%s][%s][%d] Parent Node's uargc4[%d]\n", __FILE__, __func__, __LINE__, uargc4);
  for(i = 0; i < uargc4; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv4[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv4[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv4[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv4[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node's uargc5[%d]\n", __FILE__, __func__, __LINE__, uargc5);
  printf("Enter [%s][%s][%d] Parent Node's uargc5[%d]\n", __FILE__, __func__, __LINE__, uargc5);
  for(i = 0; i < uargc5; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv5[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv5[i]);
    printf("Enter [%s][%s][%d] Parent Node uargv5[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv5[i]);
  }
  cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  printf("Enter [%s][%s][%d] Command's cargc[%d]\n", __FILE__, __func__, __LINE__, cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
    printf("Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
  cmdPrint(cmsh,"End");
  printf("End\n");
  return CMD_IPC_OK;
}


