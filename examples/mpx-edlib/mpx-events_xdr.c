/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "mpx-events.h"

int encodeCnt = 0;
int decodeCnt = 0;
int freeCnt = 0;
int otherCnt = 0;

bool_t
xdr_MpxConsumerEventCode (XDR *xdrs, MpxConsumerEventCode *objp)
{
	register int32_t *buf;

	 if (!xdr_enum (xdrs, (enum_t *) objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_MpxConsumerEventAStruct (XDR *xdrs, MpxConsumerEventAStruct *objp)
{
	register int32_t *buf;

	 if (!xdr_MpxEventBaseStruct (xdrs, &objp->base))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->m_string, ~0))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_MpxConsumerEventBStruct (XDR *xdrs, MpxConsumerEventBStruct *objp)
{
	register int32_t *buf;

	 if (!xdr_MpxEventBaseStruct (xdrs, &objp->base))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->alpha))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->beta))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->gama))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_MpxConsumerEventStruct (XDR *xdrs, MpxConsumerEventStruct *objp)
{
	register int32_t *buf;

	switch (xdrs->x_op)
	{
	case XDR_ENCODE:
		++encodeCnt;
		break;
	case XDR_DECODE:
		++decodeCnt;
		break;
	case XDR_FREE:
		++freeCnt;
		break;
	default:
		++otherCnt;
		break;
	}
	 if (!xdr_MpxConsumerEventCode (xdrs, &objp->m_code))
		 return FALSE;
	switch (objp->m_code) {
	case MpxConsumerEventACode:
		 if (!xdr_MpxConsumerEventAStruct (xdrs, &objp->MpxConsumerEventStruct_u.m_ConsumerEventA))
			 return FALSE;
		break;
	case MpxConsumerEventBCode:
		 if (!xdr_MpxConsumerEventBStruct (xdrs, &objp->MpxConsumerEventStruct_u.m_ConsumerEventB))
			 return FALSE;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
