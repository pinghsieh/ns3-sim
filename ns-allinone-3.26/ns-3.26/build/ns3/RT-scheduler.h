/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * RT-scheduler.h
 *
 *  Created on: Feb 1, 2018
 *      Author: Ping-Chun Hsieh
 */

#ifndef RT_SCHEDULER_H
#define RT_SCHEDULER_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include <vector>

namespace ns3 {

class RTLinkParams;
class RTScheduler: public Object
{
	public:
		static TypeId GetTypeId (void);
		RTScheduler();
		virtual ~RTScheduler();

		void AddOneNewRTLink(RTLinkParams*);
		RTLinkParams* GetRTLinkAtPosition(uint32_t pos);
		RTLinkParams* UpdateSchedulingDecision(void);
		uint32_t FindLinkWithLargestDeliveryDebtTimesPn(void);
		void SetCurrentIntervalEnd(Time);
		void GeneratePacketCountForLinks();
		void AddQnDeliveryDebtForLinks();
		void StartSchedulingTransmissionsNow();
		bool IsNoMoreValidPacket();
		void ReceiveCallFromScheduledLink(RTLinkParams*);
		bool IsScheduled() { return m_scheduled;}
		void ResetScheduled() {m_scheduled = false;}
		uint32_t GetRTLinkCount();
		void UpdateDebt();

	private:
		std::vector<RTLinkParams*> m_rtLinkVec;
		Time m_currentIntervalEnd;
		bool m_scheduled;
		uint32_t m_scheduledLinkId;
		//bool m_noMoreValidPacket;
};

}

#endif



