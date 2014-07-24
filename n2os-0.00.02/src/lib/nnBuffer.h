#ifndef _NN_BUFFER_H_
#define _NN_BUFFER_H_

#include <string.h>
#include "nnDefines.h"
#include "nnPrefix.h"

#define NN_MAX_BUFFER_SIZE 4096


///////////////////////////////////////////////////////////////////////////
// Routing API Structure
///////////////////////////////////////////////////////////////////////////
struct nnBuffer {
  Uint32T        index;
  Uint32T        length;
  char           data[NN_MAX_BUFFER_SIZE];
};
typedef struct nnBuffer nnBufferT;

///////////////////////////////////////////////////////////////////////////
// Routing API Structure
///////////////////////////////////////////////////////////////////////////
void     nnBufferReset (nnBufferT * buffer);
void     nnBufferResetIndex (nnBufferT * buffer);
void     nnBufferAssign (nnBufferT * buffer, char * msg, Uint32T msgLen);
Uint32T  nnBufferGetLength (nnBufferT * buffer);
Uint32T  nnBufferGetIndex (nnBufferT * buffer);
void     nnBufferSetIndex (nnBufferT * buffer, Uint32T);
void     nnBufferNCpy (nnBufferT * buffer, nnBufferT * dest, Uint32T length);
char *   nnBufferGetData  (nnBufferT * buffer);
void     nnBufferPrint  (nnBufferT * buffer);
Int8T    nnBufferGetInt8T (nnBufferT * buffer);
Int16T   nnBufferGetInt16T(nnBufferT * buffer);
Int32T   nnBufferGetInt32T(nnBufferT * buffer);
Int64T   nnBufferGetInt64T(nnBufferT * buffer);
Prefix4T nnBufferGetPrefix4T(nnBufferT * buffer);
Prefix6T nnBufferGetPrefix6T(nnBufferT * buffer);
PrefixT  nnBufferGetPrefixT(nnBufferT * buffer);
struct in_addr nnBufferGetInaddr(nnBufferT * buffer);
struct in6_addr nnBufferGetInaddr6(nnBufferT * buffer);
void nnBufferGetString(nnBufferT * buffer, char * dest, Int32T length);

void nnBufferSetInt8T (nnBufferT * buffer, Int8T val);
void nnBufferSetInt16T(nnBufferT * buffer, Int16T val);
void nnBufferSetInt32T(nnBufferT * buffer, Int32T val);
void nnBufferSetInt64T(nnBufferT * buffer, Int64T val);
void nnBufferSetPrefix4T(nnBufferT * buffer, Prefix4T * val);
void nnBufferSetPrefix6T(nnBufferT * buffer, Prefix6T * val);
void nnBufferSetPrefixT(nnBufferT * buffer, PrefixT * val);
void nnBufferSetInaddr(nnBufferT * buffer, struct in_addr val);
void nnBufferSetInaddr6(nnBufferT * buffer, struct in6_addr val);;
void nnBufferSetString(nnBufferT * buffer, char * src, Int32T length);
void nnBufferInsertInt8T(nnBufferT * buffer, Uint32T position, Uint8T val);
void nnBufferInsertInt16T(nnBufferT * buffer, Uint32T position, Uint16T val);
void nnBufferInsertInt32T(nnBufferT * buffer, Uint32T position, Uint32T val);

#endif
