/*
 * RT-scheduler.cc
 *
 *  Created on: Feb 1, 2018
 *      Author: Ping-Chun
 */

#include "RT-scheduler.h"
#include "RT-link-params.h"
#include "ns3/log.h"
#include "dca-txop.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RTScheduler");

NS_OBJECT_ENSURE_REGISTERED (RTScheduler);

TypeId
RTScheduler::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::RTScheduler")
			.SetParent<Object> ()
			.SetGroupName ("Wifi");
	return tid;
}

RTScheduler::RTScheduler(void):
		m_rtLinkVec(std::vector<RTLinkParams*>()),
		m_scheduled(false),
		m_scheduledLinkId(0)
{

}

RTScheduler::~RTScheduler()
{
	// Do nothing for now
}

void
RTScheduler::AddOneNewRTLink(RTLinkParams* p)
{
	if (p != 0){
		m_rtLinkVec.push_back(p);
	}
}

RTLinkParams*
RTScheduler::GetRTLinkAtPosition(uint32_t pos)
{
	NS_ASSERT (m_rtLinkVec.size() > pos);
	return 	m_rtLinkVec.at(pos);
}

RTLinkParams*
RTScheduler::UpdateSchedulingDecision(void)
{
	/** LDF:
	 */
	uint32_t m = FindLinkWithLargestDeliveryDebtTimesPn();
	if (m >= m_rtLinkVec.size()){
		return 0;
	}
	return GetRTLinkAtPosition(m);

}

uint32_t
RTScheduler::FindLinkWithLargestDeliveryDebtTimesPn()
{
	/* Implement scheduling policy
	 *
	 * LDF policy: schedule the one with a non-dummy packet
	 * and largest delivery debt. If no link has a valid packet, then
	 * return m = m_rtLinkVec.size()
	 *
	 * */
	NS_ASSERT(m_rtLinkVec.size() > 0);
	double temp =  0;
	uint32_t id = m_rtLinkVec.size();
	// Find the first link with a non-dummy packet
	for (uint32_t i = 0; i < m_rtLinkVec.size(); i++)
	{
		if (((m_rtLinkVec.at(i))->GetPacketCount()) > 0)
		{
			temp = (m_rtLinkVec.at(i)->GetPn())*(m_rtLinkVec.at(i))->GetDeliveryDebt();
			id = i;
			break;
		}
	}
	//  Find the one with a non-dummy packet and largest delivery debt
	for (uint32_t i = id + 1; i < m_rtLinkVec.size(); i++)
	{
		if (temp < ((m_rtLinkVec.at(i)->GetPn())*(m_rtLinkVec.at(i))->GetDeliveryDebt())
				&&  ((m_rtLinkVec.at(i))->GetPacketCount()) > 0)
		{
			temp = (m_rtLinkVec.at(i)->GetPn())*(m_rtLinkVec.at(i))->GetDeliveryDebt();
			id = i;
		}
	}
	return id;
}

void
RTScheduler::SetCurrentIntervalEnd(Time t)
{
	m_currentIntervalEnd = t;
	for (uint32_t i = 0; i < m_rtLinkVec.size(); i++)
	{
		(m_rtLinkVec.at(i)->GetDcaTxop())->SetCurrentIntervalEnd(t);
	}
}

void
RTScheduler::GeneratePacketCountForLinks()
{
	for (uint32_t i = 0; i < m_rtLinkVec.size(); i++)
	{
		m_rtLinkVec.at(i)->GeneratePacketCount();
	}
}

void
RTScheduler::AddQnDeliveryDebtForLinks()
{
	for (uint32_t i = 0; i < m_rtLinkVec.size(); i++)
	{
		m_rtLinkVec.at(i)->AddDeliveryDebt((m_rtLinkVec.at(i)->GetQn())*(m_rtLinkVec.at(i)->GetArrivalRate()));
	}
}

void
RTScheduler::StartSchedulingTransmissionsNow()
{
	//NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": in StartSchedulingTransmissionsNow() \n");
	/* Get the pointer of the link to be scheduled
	 * If pointer is 0, then this means there is no link with a valid packet
	 * */
	if (! IsScheduled()){
		RTLinkParams* p = UpdateSchedulingDecision();
		if (p != 0)
		{
			m_scheduledLinkId = p->GetLinkId();
			/* Note: decrement first, then enqueue the packet.
			 * Otherwise, the scheduling process will be an infinite loop
			 * */
			p->DecrementPacketCount(1);
			p->EnqueueOnePacket();
		} else {
			// No more action by scheduler in this interval
			// Do nothing for now
		}
	}
}

void
RTScheduler::ReceiveCallFromScheduledLink(RTLinkParams* p)
{
	/* This function is called by the scheduled link when this link
	 * delivers the packet or drop a expired packet
	 * */
	NS_ASSERT(p != 0);
	if (p->GetLinkId() == m_scheduledLinkId){
		ResetScheduled();
		StartSchedulingTransmissionsNow();
	}
}

uint32_t
RTScheduler::GetRTLinkCount()
{
	return (m_rtLinkVec.size());
}

}




