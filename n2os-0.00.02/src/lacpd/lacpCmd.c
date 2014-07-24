/********************************************************************************
 *                                  INCLUDE FILES 
 * ********************************************************************************/
#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
#include "nnUtility.h"
#include "nnList.h"

#include "nnBuffer.h"
#include "nnPrefix.h"

#include "lacpDef.h"

DECMD(cmdFuncLacpTimeout,
    CMD_NODE_INTERFACE,
    IPC_LACP,
    "lacp timeout (long|short)",
    "set link aggregation protocol",
    "set periodic timer",
	"Long timeout (30 secs)",
	"Short timeout (1 sec)")
{
  Int32T len;
  Int32T ifIndex;
  Int32T groupId;
  Int32T longTimeout;

  if (cargc != 3) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }

  if (strncmp (uargv1[1], "po" , 2)) {
noIf:
    cmdPrint (cmsh, "Wrong interface %s.\n", uargv1[1]);
    return (CMD_IPC_OK);
  }

  groupId = sscanf (uargv1[1], "po%i", &groupId);
  if (groupId < 0 || groupId > 4)
    goto noIf;

#if 0
  ifIndex = lacpGetPortIfIndex (uargv1[1]);
  if (ifIndex < 0) {
    cmdPrint (cmsh, "Unavailable interface %s.\n", uargv1[1]);
   	return (CMD_IPC_OK);	//ERROR
  }
#endif

  len = strlen(cargv[2]);
  if (!strncmp (cargv[2], "long", len))
     longTimeout = LACP_SYS_TIMEOUT_LONG;
  else if (!strncmp (cargv[2], "short", len))
     longTimeout = LACP_SYS_TIMEOUT_SHORT;
  else
   	return (CMD_IPC_OK);	//ERROR

  return lacpGroupTimeoutChange (cmsh, groupId, longTimeout);
}

DECMD(cmdFuncLacpNoTimeout,
    CMD_NODE_INTERFACE,
    IPC_LACP,
    "no lacp timeout",
    "Negate a command or set its defaults",
    "set link aggregation protocol",
    "set periodic timer")
{
  Int32T ifIndex;
  Int32T groupId;

  if (cargc != 3) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }

  if (strncmp (uargv1[1], "po" , 2)) {
noIf:
    cmdPrint (cmsh, "Wrong interface %s.\n", uargv1[1]);
    return (CMD_IPC_OK);	//ERROR
  }

  groupId = sscanf (uargv1[1], "po%i", &groupId);
  if (groupId < 0 || groupId > 4)
    goto noIf;

#if 0
  ifIndex = lacpGetPortIfIndex (uargv1[1]);
  if (ifIndex < 0) {
    cmdPrint (cmsh, "Unavailable interface %s.\n", uargv1[1]);
   	return (CMD_IPC_OK);	//ERROR
  }
#endif

  return lacpGroupTimeoutChange (cmsh, groupId, LACP_DEFAULT_SYS_TIMEOUT);
}

DECMD(cmdFuncLacpPortChannel,
    CMD_NODE_INTERFACE,
    IPC_LACP,
    "channel-group <1-4> (active|passive)",
    "set port trunking (link aggregation)",
    "Group ID",
	"Active actor",
	"passive actor")
{
  Int32T len;
  Int32T groupId;
  Int32T active= 0;

  if (cargc != 3) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }

  if (strncmp (uargv1[1], "eth" , 3)) {
    cmdPrint (cmsh, "Wrong interface %s.\n", uargv1[1]);
    return (CMD_IPC_OK);	//ERROR
  }

  len = strlen(cargv[2]);
  if (!strncmp (cargv[2], "active", len))
     active = 1;
  else if (!strncmp (cargv[2], "passive", len))
     active = 0;
  else
    return (CMD_IPC_OK);	//ERROR

  groupId = atoi(cargv[1]);

  return lacpPortAttach (cmsh, groupId, uargv1[1], active);
}

DECMD(cmdFuncLacpNoPortChannel,
    CMD_NODE_INTERFACE,
    IPC_LACP,
    "no channel-group",
    "Negate a command or set its defaults",
    "set port trunking (link aggregation)")
{
  if (cargc != 2) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }

  if (strncmp (uargv1[1], "eth" , 3)) {
    cmdPrint (cmsh, "Wrong interface %s.\n", uargv1[1]);
    return (CMD_IPC_OK);	//ERROR
  }

  return lacpPortDetach (cmsh, uargv1[1]);
}

DECMD(cmdFuncLacpShowCliPortChannel,
    CMD_NODE_EXEC,
    IPC_LACP | IPC_SHOW_MGR,
    "show port-channel cli",
    "show",
    "port trunking (link aggregation)",
    "CLI config")
{
  if (cargc != 3) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }
  lacpCliConfigDisplay (cmsh);

  return (CMD_IPC_OK);
}

DECMD(cmdFuncLacpShowPifPortChannel,
    CMD_NODE_EXEC,
    IPC_LACP | IPC_SHOW_MGR,
    "show port-channel pif",
    "show",
    "port trunking (link aggregation)",
    "Pif reported")
{
  if (cargc != 3) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }
  lacpPifPortDisplay (cmsh);

  return (CMD_IPC_OK);
}

DECMD(cmdFuncLacpShowPortChannelConfig,
    CMD_NODE_EXEC,
    IPC_LACP | IPC_SHOW_MGR,
    "show port-channel <1-4> config",
    "show",
    "port trunking (link aggregation)",
    "Group ID",
    "LACP config")
{
  if (cargc != 4) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }
  lacpRunConfigDisplay (cmsh, atoi(cargv[2]));

  return (CMD_IPC_OK);
}

DECMD(cmdFuncLacpShowPortChannelState,
    CMD_NODE_EXEC,
    IPC_LACP | IPC_SHOW_MGR,
    "show port-channel <1-4> state",
    "show",
    "port trunking (link aggregation)",
    "Group ID",
    "LACP state")
{
  if (cargc != 4) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }
  lacpRunStateDisplay (cmsh, atoi(cargv[2]));

  return (CMD_IPC_OK);
}

DECMD(cmdFuncNoLacpDebug,
    CMD_NODE_EXEC,
    IPC_LACP,
    "no debug lacp",
    "Negate a command or set its defaults",
    "debug",
    "link aggregation protocol")
{
  if (cargc != 3) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }
  pLacp->debug = 0;

  return (CMD_IPC_OK);
}

DECMD(cmdFuncLacpDebug,
    CMD_NODE_EXEC,
    IPC_LACP,
    "debug lacp",
    "debug",
    "link aggregation protocol")
{
  if (cargc != 2) {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);	//ERROR
  }

  pLacp->debug = 1;

  return (CMD_IPC_OK);
}
