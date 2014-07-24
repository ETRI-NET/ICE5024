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

void daemon_log (Uint32T priority, const StringT format, ...)
{
	va_list args;
	char tmp[256];
	char ptr[256];
	va_start(args, format);
	strcpy (tmp, format);
	strcat (tmp, "\n");
	vsnprintf (ptr, 255, tmp, args);
	DEBUGPRINT (ptr);
	if (priority == LOG_DEBUG) {
		if (pLacp->debug)
			NNLOG(priority, ptr);
	}
	else {
		NNLOG(priority, ptr);
	}
	va_end(args);
}
