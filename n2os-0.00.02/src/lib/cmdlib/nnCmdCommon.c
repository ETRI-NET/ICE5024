/**
 * @file      : nnCmdService.c
 * @brief       :
 *
 * $Id: nnCmdMd5.c 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: hyryu $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "nnTypes.h"
#include "nnLog.h"
#include "taskManager.h"
#include "nnDefines.h"

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */

Uint32T
cmdDaemonize(Uint16T nochdir, Uint16T noclose)
{
    pid_t pid;
    Uint32T fd;
    pid = fork ();
    /** In case of fork is error. */
    if (pid < 0)
    {
        perror ("fork");
        return -1;
    }
    /** In case of this is parent process. */
    if (pid != 0)
    {
        exit (0);
    }
    /** Become session leader and get pid. */
    pid = setsid();
    if (pid < -1)
    {
        perror ("setsid");
        return -1;
    }

    /** Change directory to root. */
    if (! nochdir)
    {
        if(chdir("/") != 0)
        {
            perror("chdir\n");
            exit(0);
        }
    }
    /** File descriptor close. */
    if (! noclose)
    {
        fd = open ("/dev/null", O_RDWR, 0);
        if (fd != -1)
        {
            dup2 (fd, STDIN_FILENO);
            dup2 (fd, STDOUT_FILENO);
            dup2 (fd, STDERR_FILENO);
            if (fd > 2)
            {
                close (fd);
            }
        }
    }
    umask (0027);
    return 0;
}
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void
cmdCallbackShellPipeEv(Int8T *argv1[], Int8T *argv2[])
{
  Int32T des_p[2];
  if(pipe(des_p) == -1)
  {
    perror("Pipe failed");
    exit(1);
  }

  if(fork() == 0)      /**  first fork  */
  {
    close(1);          /**  closing stdout */
    dup(des_p[1]);     /**  replacing stdout with pipe write */
    close(des_p[0]);   /**  closing pipe read */
    close(des_p[1]);

    execvp(argv1[0], argv1);
    perror("execvp of ls failed");
    exit(1);
  }

  if(fork() == 0)      /**  creating 2nd child */
  {
    close(0);          /**  closing stdin        */
    dup(des_p[0]);     /**  replacing stdin with pipe read */
    close(des_p[1]);   /**  closing pipe write   */
    close(des_p[0]);

    execvp(argv2[0], argv2);
    perror("execvp of wc failed");
    exit(1);
  }

  close(des_p[0]);
  close(des_p[1]);
  wait(0);
  wait(0);
}

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void
cmdCallbackShellEv(Int8T *argv[])
{
  Int32T status;
  pid_t  pid;

  if ((pid = fork()) < 0)
  {     /** fork a child process           */
    exit(1);
  }
  else if (pid == 0)
  {          /** for the child process:         */
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    if (execvp(argv[0], argv) < 0)
    {     /** execute the command  */
      exit(1);
    }
  }
  else
  {                                   /** for the parent:      */
    while (wait(&status) != pid); /** wait for completion  */
  }
}

static void
cmdParseCommandLine(Int8T *line, Int8T **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' &&
                 *line != '\t' && *line != '\n')
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
}

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Int32T
cmdCallbackShellCmd(Int8T *argv[])
{
  Int32T status;
  pid_t  pid;

  if ((pid = fork()) < 0)
  {     /** fork a child process           */
    exit(1);
  }
  else if (pid == 0)
  {          /** for the child process:         */
    if (execvp(argv[0], argv) < 0)
    {     /** execute the command  */
      exit(1);
    }
  }
  else
  {                                   /** for the parent:      */
    while (wait(&status) != pid); /** wait for completion  */
  }
  return (pid + 1);
}

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Int32T
cmdCallbackShellSrv(Int8T *argv)
{
  Int8T  *argvs[64];
  cmdParseCommandLine(argv, argvs);
  return cmdCallbackShellCmd(argvs);
}

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void *
cmdSetSignal(Int32T signo, void (*func)(Int32T))
{
  struct sigaction sig;
  struct sigaction osig;
  sig.sa_handler = func;
  sigemptyset (&sig.sa_mask);
  sig.sa_flags = 0;
#ifdef SA_RESTART
  sig.sa_flags |= SA_RESTART;
#endif /** SA_RESTART */
  if (sigaction (signo, &sig, &osig) < 0) {
      return (SIG_ERR);
  } else {
      return (osig.sa_handler);
  }
}

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Uint32T 
cmdGetTerm(Int32T sock, Uint16T *row, Uint16T *col)
{
    struct winsize w;
    Int32T ret;
    ret = ioctl (sock, TIOCGWINSZ, &w);
    if (ret < 0)
    {
        return -1;
    }
    *row = w.ws_row;
    *col = w.ws_col;
    return 0;

}

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Uint32T 
cmdSetTerm(Int32T sock, Uint16T row, Uint16T col)
{
    struct winsize w;
    Int32T ret;
    w.ws_row = row;
    w.ws_col = col;
    ret = ioctl (sock, TIOCSWINSZ, &w);
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}

/**
 * Description:
 *
 * @param [in]
 * @param [out]
 *
 * @retval : code
 */
Uint32T 
cmdCompBitAdd(Uint8T value[], Uint32T nIndex)
{
  Uint32T x, y;
  x = nIndex / 8;
  y = nIndex % 8;
  Uint8T c = (1 << y);
  value[x] |= c;
  return 0;
}

/**
 * Description:
 *
 * @param [in]
 * @param [out]
 *
 * @retval : code
 */
Uint32T 
cmdCompBitCheck(Uint8T value[], Uint32T nIndex)
{
  Uint32T x, y;
  x = nIndex / 8;
  y = nIndex % 8;
  Uint8T c = (1 << y);
  if(value[x] & c)
  {
    return 1;
  }
  return 0;
}

