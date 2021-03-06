/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#pragma once

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


enum MpxEventCode {
	MpxEventBaseCode = 0,
	MpxStartEventCode = -1,
	MpxTimerEventCode = -2,
	MpxStopEventCode = -3,
	MpxTcp4ListenerEventCode = -4,
	MpxTcp4EndPointEventCode = -5,
	MpxTcp4ClientEventCode = -6,
	MpxTcp6ListenerEventCode = -7,
	MpxTcp6EndPointEventCode = -8,
	MpxTcp6ClientEventCode = -9,
	MpxLocalListenerEventCode = -10,
	MpxLocalEndPointEventCode = -11,
	MpxLocalClientEventCode = -12,
	MpxPosixMQEventCode = -13,
	MpxJobFinishedEventCode = -14,
	MpxLocalTaskQueryEventCode = -15,
	MpxPosixMQTaskQueryEventCode = -16,
	MpxTcp4TaskQueryEventCode = -17,
	MpxTcp6TaskQueryEventCode = -18,
	MpxUdp4TaskQueryEventCode = -19,
	MpxUdp6TaskQueryEventCode = -20,
	MpxExternalTaskEventCode = -21,
	MpxTaskQueryEventCode = -22,
	MpxTaskResponseEventCode = -23,
	MpxProxyTaskEventCode = -24,
	MpxProxyTaskRelocationEventCode = -25,
};
typedef enum MpxEventCode MpxEventCode;

typedef u_long taskid_t;

struct MpxEventBaseStruct {
	u_int m_code;
	taskid_t m_src;
	taskid_t m_dst;
};
typedef struct MpxEventBaseStruct MpxEventBaseStruct;

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_MpxEventCode (XDR *, MpxEventCode*);
extern  bool_t xdr_taskid_t (XDR *, taskid_t*);
extern  bool_t xdr_MpxEventBaseStruct (XDR *, MpxEventBaseStruct*);

#else /* K&R C */
extern bool_t xdr_MpxEventCode ();
extern bool_t xdr_taskid_t ();
extern bool_t xdr_MpxEventBaseStruct ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif
