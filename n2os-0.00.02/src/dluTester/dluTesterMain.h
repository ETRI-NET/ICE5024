#if !defined(_dluTesterMain_h)
#define _dluTesterMain_h

#include "nnTypes.h"

typedef struct dluTestT
{
  void * gCmshGlobal;      /** Component Cmd Variable **/

} dluTestT;

void dluTesterInitProcess(void);
void dluTesterTermProcess(void);
void dluTesterRestartProcess(void);
void dluTesterHoldProcess(void);
void dluTesterSingalProcess(Int32T sig);

#endif /* _dluTesterMain_h */

