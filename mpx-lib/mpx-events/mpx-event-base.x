/*
// mpx-event-base.x
// Created on: Mar 12, 2018
//     Author: miran
*/

typedef u_long taskid_t;

struct	MpxEventBaseStruct
{
	u_int m_code;
	taskid_t m_src;
	taskid_t m_dst;
};
