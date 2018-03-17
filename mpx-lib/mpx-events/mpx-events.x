/*
//    Event Driven Task Multiplexing Library
//    Copyright (C) 2018 Miran Vodnik
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//    contact: miran.vodnik@siol.net
*/

enum MpxEventCode
{
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
	MpxTaskResponseEventCode = -23
};

typedef u_long taskid_t;

struct	MpxEventBaseStruct
{
	u_int m_code;
	taskid_t m_src;
	taskid_t m_dst;
};
