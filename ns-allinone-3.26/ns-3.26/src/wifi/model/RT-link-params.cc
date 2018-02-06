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
 * RT-link-params.cc
 *
 *  Created on: Jan 22, 2018
 *      Author: Ping-Chun Hsieh
 */

#include "RT-link-params.h"
#include "adhoc-wifi-mac.h"
#include "wifi-net-device.h"
#include "dca-txop.h"
#include "wifi-mac-queue.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "RT-scheduler.h"
#include <algorithm>
#include <math.h>
#include <iomanip>
#include <limits>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("RTLinkParams");

NS_OBJECT_ENSURE_REGISTERED (RTLinkParams);

TypeId
RTLinkParams::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::RTLinkParams")
			.SetParent<Object> ()
			.SetGroupName ("Wifi");
	return tid;
}

RTLinkParams::RTLinkParams()
 : m_packetSize(0),
   m_packetCount(0),
   m_linkPriority(uint32_t(0)),
   m_qn(double(0)),
   m_R(double(0)),
   m_pn(double(0)),
   m_linkId(0),
   m_backoff(0),
   m_arrivalRate(0),
   m_sizeDummyPacket(8),
   m_swapId(std::vector<uint32_t>()),
   m_isUsingDummyPacket(false),
   m_alreadyTransmit(false),
   m_CWMin(1),
   m_CWLevelCount(1),
   m_Rmax(1),
   m_maxPacketCount(1),
   m_alpha(0)
{
    std::random_device rd;
    m_generator = std::minstd_rand0(rd());
}

RTLinkParams::RTLinkParams(Ptr<WifiNetDevice> nd, Ptr<AdhocWifiMac> wm,
		uint32_t ps, uint32_t pc, uint32_t lp, double qn, double R, double pn, uint32_t id,
		uint32_t bo, double ar, uint32_t cwm, uint32_t cwl, double rmax, uint32_t mpc, double mal)
{
	m_netDev = nd;
    m_macDest = wm;
	m_packetSize = ps;
	m_packetCount = pc;
    m_linkPriority = lp;
    m_qn = qn;
    m_R = R;
    m_pn = pn;
    m_swapState = SwapStates::STATE_NONE;
    m_swapAction = SwapActions::ACT_NONE;
    m_swapIntent = SwapIntents::INT_NONE;
    m_linkId = id;
    m_backoff = bo;
    m_arrivalProcess = ArrivalCode::ARR_CONST;
    m_arrivalRate = ar;
    m_swapId = std::vector<uint32_t>();
    m_isUsingDummyPacket = false;
    m_alreadyTransmit = false;
    m_algCode = AlgorithmCode::ALG_DBDP;
    std::random_device rd;
    m_generator = std::minstd_rand0(rd());
    m_CWMin = cwm;
    m_CWLevelCount = cwl;
    m_Rmax = rmax;
    m_maxPacketCount = mpc;
    m_alpha = mal;
}

RTLinkParams::~RTLinkParams()
{
   // do nothing for now
}

void
RTLinkParams::DoInitialize(Ptr<WifiNetDevice> nd, Ptr<AdhocWifiMac> wm,
		uint32_t ps, uint32_t pc, uint32_t lp, double qn, double R, double pn,
		Ptr<OutputStreamWrapper> osw, uint32_t id, uint32_t bo, ArrivalCode ac,
		double ar, AlgorithmCode alc, uint32_t cwm, uint32_t cwl, double rmax,
		RTScheduler* sch, uint32_t mpc, double mal)
{
	m_netDev = nd;
    m_macDest = wm;
	m_packetSize = ps;
	m_packetCount = pc;
    m_linkPriority = lp;
    m_qn = qn;
    m_R = R;
    m_pn = pn;
    m_swapState = SwapStates::STATE_NONE;
    m_swapAction = SwapActions::ACT_NONE;
    m_swapIntent = SwapIntents::INT_NONE;
    m_stream = osw;
    m_linkId = id;
    m_backoff = bo;
    m_arrivalProcess = ac;
    m_arrivalRate = ar;
    m_algCode = alc;
    m_CWMin = cwm;
    m_CWLevelCount = cwl;
    m_Rmax = rmax;
    if (IsUsingScheduler()){
    	m_scheduler = sch;
    }
    m_maxPacketCount = mpc;
    m_alpha = mal;
}

Ptr<WifiNetDevice>
RTLinkParams::GetNetDevice()
{
	NS_LOG_FUNCTION (this);
	return m_netDev;
}

Ptr<AdhocWifiMac>
RTLinkParams::GetMacDest()
{
	NS_LOG_FUNCTION (this);
	return m_macDest;
}

Ptr<DcaTxop>
RTLinkParams::GetDcaTxop()
{
	NS_LOG_FUNCTION (this);
	return m_netDev->GetMac()->GetObject<AdhocWifiMac>()->GetDcaTxop();
}

Ptr<AdhocWifiMac>
RTLinkParams::GetMacSource()
{
	NS_LOG_FUNCTION (this);
	return m_netDev->GetMac()->GetObject<AdhocWifiMac>();
}

double
RTLinkParams::CalculateAccessProbability(void)
{
	// TODO
	NS_LOG_FUNCTION (this);
    double debt = GetDcaTxop()->GetDeliveryDebt();
    double numerator = exp(m_pn*log10(std::max(1.0, 100.0*(debt+1.0))));
    double denominator =  m_R + exp(m_pn*log10(std::max(1.0, 100.0*(debt+1.0))));
    return (numerator/denominator);
}

void
RTLinkParams::UpdateLinkPriority(void)
{
	// TODO
	NS_LOG_FUNCTION (this);
	if (IsActionUp()) { m_linkPriority--; }
	if (IsActionDown()) { m_linkPriority++; }
	return;
}

void
RTLinkParams::ChangePriorityIfNeeded(void)
{
	NS_LOG_FUNCTION (this);
   /*
    * Ping-Chun: this function is called when channel is sensed busy at the time
    * when the remaining backoff slot equals 1 (see DcfManager::Updatebackoff)
    */
	// TODO
    if (IsStateLead() && IsIntentDown()){
    	m_swapAction = SwapActions::ACT_DOWN;  // move down
    }
    if (IsStateTrail() && IsIntentUp()){
    	m_swapIntent = SwapIntents::INT_NONE; // does not move
    }
	return;
}

void
RTLinkParams::ResetAllSwapVariables(void)
{
	NS_LOG_FUNCTION (this);
	ResetSwapState();
	ResetSwapIntent();
	ResetSwapAction();
}

void
RTLinkParams::PrintDeliveryDebtToFile(void)
{
	NS_LOG_FUNCTION (this);
    *(m_stream->GetStream()) << std::setw(10) << m_linkId << std::setw(12) << Simulator::Now().GetSeconds() << std::setw(20) << std::setprecision(5) << GetDcaTxop() -> GetDeliveryDebt()
    		<< std::setw(10) << m_linkPriority << std::setw(15) << m_backoff << std::setw(16) << ((GetDcaTxop())->GetQueue())->GetSize()
			<< std::setw(15) << GetPacketCount() << "\n";
}

void
RTLinkParams::EnqueueOnePacket(void)
{
	NS_LOG_FUNCTION (this);
	GetMacSource()->Enqueue(Create<Packet> (GetPacketSize()), GetMacDest()->GetAddress());
	//GetDcaTxop()->UpdateDeliveryDebt (GetQn());
}
void
RTLinkParams::EnqueueMultiplePackets(uint32_t m)
{
	NS_LOG_FUNCTION (this);
	for (uint32_t i = 0; i < m; i++){
		GetMacSource()->Enqueue(Create<Packet> (GetPacketSize()), GetMacDest()->GetAddress());
		//GetDcaTxop()->UpdateDeliveryDebt (GetQn());
	}
}

void
RTLinkParams::EnqueueOnePacketGivenSize(uint32_t sz)
{
	NS_LOG_FUNCTION (this);
	GetMacSource()->Enqueue(Create<Packet> (sz), GetMacDest()->GetAddress());
	//GetDcaTxop()->UpdateDeliveryDebt (GetQn());
}

void
RTLinkParams::EnqueueOneDummyPacket(void)
{
	NS_LOG_FUNCTION (this);
	/*
	 * Ping-Chun: dummy packet does not count in delivery or deficit
	 */
	SetIsUsingDummyPacket();
	GetMacSource()->Enqueue(Create<Packet> (m_sizeDummyPacket), GetMacDest()->GetAddress());

}

void
RTLinkParams::GeneratePacketCount(void)
{
	NS_LOG_FUNCTION (this);
    //std::random_device rd;
    //std::default_random_engine generator (rd());
	switch (m_arrivalProcess)
	{
		case ARR_BERN: {
			   //Bernoulli
		       std::uniform_real_distribution<double> distribution_unif(0.0, 1.0);
		       double rand_number = distribution_unif(m_generator);
			   m_packetCount = (rand_number < m_arrivalRate)? 1:0;
			   break;}
		case ARR_UNIF: {
			    // Uniform between 0 to maxCount
				uint32_t maxCount = (uint32_t) ceil(2*m_arrivalRate);
				std::uniform_int_distribution<uint32_t> distribution_int(0, std::max(uint32_t(0),maxCount));
				m_packetCount = distribution_int(m_generator);
				break;}
		case ARR_BERNUNIF: {
				/* with probability alpha, uniformly distributed within {1,...,m_maxPacketCount}
				 * with probability (1-alpha), 0 packet
				 */
			    std::uniform_int_distribution<uint32_t> distribution_int(1, std::max(uint32_t(1),m_maxPacketCount));
			    uint32_t pc = distribution_int(m_generator);
		        std::uniform_real_distribution<double> distribution_unif(0.0, 1.0);
		        double rand_number = distribution_unif(m_generator);
		        m_packetCount = (rand_number < m_alpha)? pc:0;
				break;}
		case ARR_CONST: {
			// Constant arrivals
				m_packetCount = std::max((uint32_t)(0),(uint32_t)ceil(m_arrivalRate));
				break;}
		default: {
			// Default is constant
				m_packetCount = std::max((uint32_t) (0),(uint32_t)ceil(m_arrivalRate));
				break;}
	}
}

void
RTLinkParams::EnqueueDummyPacketIfNeeded(void)
{
	NS_LOG_FUNCTION (this);
	if (IsUsingDBDP() && (IsStateLead() || IsStateTrail())){
	//if (IsUsingDBDP() && m_packetCount == 0 && (IsStateLead() || IsStateTrail())){
		EnqueueOneDummyPacket();
	}
}

uint32_t
RTLinkParams::CalculateRTBackoff(std::vector<uint32_t> swapId)
{
	NS_LOG_FUNCTION (this);
	/*
	 * Ping-Chun: this function does two things
	 * 1. calculate backoff number for the current interval
	 * 2. determine swap states
	 */
    //std::random_device rd;
    //std::default_random_engine generator (rd());
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    double rand_number = distribution(m_generator);

    for (uint32_t i = 0; i < swapId.size(); i++)
    {
        if (swapId.at(i) == m_linkPriority || (swapId.at(i) + 1) == m_linkPriority) {
        	if (swapId.at(i) == m_linkPriority) {
        		SetSwapState(SwapStates::STATE_LEAD);
            	if (rand_number < CalculateAccessProbability()){
            		m_swapIntent = SwapIntents::INT_NONE;
                    m_backoff = m_linkPriority - 1 + 2*i;
                    break;
            	} else {
            		m_swapIntent = SwapIntents::INT_DOWN;
            		m_backoff = m_linkPriority + 1 + 2*i;
            		break;
            	}
        	}
        	else {
        		SetSwapState(SwapStates::STATE_TRAIL);
            	if (rand_number < CalculateAccessProbability()){
            		m_swapIntent = SwapIntents::INT_UP;
            		m_backoff = m_linkPriority - 1 + 2*i;
            		break;
            	} else {
            		m_swapIntent = SwapIntents::INT_NONE;
            		m_backoff = m_linkPriority + 1 + 2*i;
            		break;
            	}
        	}
        }
        if (i == 0){
        	if (swapId.at(i) > m_linkPriority){
        		ResetAllSwapVariables();
        		m_backoff =  (m_linkPriority - 1);
        		break;
        	}
        }
        if (i < (swapId.size() - 1)){
        	if (((swapId.at(i) + 1) < m_linkPriority) && (swapId.at(i+1) > m_linkPriority)){
        		ResetAllSwapVariables();
        		m_backoff =  (m_linkPriority + 1 + 2*i);
        		break;
        	}
        }
        if (i == (swapId.size() - 1)){
        	if ((swapId.at(i) + 1) < m_linkPriority){
        		ResetAllSwapVariables();
        		m_backoff =  (m_linkPriority + 1 + 2*i);
        		break;
        	}
        }
    }
    /*
    if (swapId == m_linkPriority || (swapId + 1) == m_linkPriority) {
    	if (swapId == m_linkPriority) {
    		SetSwapState(SwapStates::STATE_LEAD);
        	if (rand_number < CalculateAccessProbability()){
        		m_swapIntent = SwapIntents::INT_NONE;
                m_backoff = m_linkPriority - 1;
        	} else {
        		m_swapIntent = SwapIntents::INT_DOWN;
        		m_backoff = m_linkPriority + 1;
        	}
    	}
    	else {
    		SetSwapState(SwapStates::STATE_TRAIL);
        	if (rand_number < CalculateAccessProbability()){
        		m_swapIntent = SwapIntents::INT_UP;
        		m_backoff = m_linkPriority - 1;
        	} else {
        		m_swapIntent = SwapIntents::INT_NONE;
        		m_backoff = m_linkPriority + 1;
        	}
    	}

    } else if ((swapId + 1) < m_linkPriority){
    	ResetAllSwapVariables();
    	m_backoff =  (m_linkPriority + 1);
    } else {
    	ResetAllSwapVariables();
    	m_backoff =  (m_linkPriority - 1);
    }
    */
    return m_backoff;
}

uint32_t
RTLinkParams::CalculateBackoffForFCSMA(void)
{
	NS_LOG_FUNCTION (this);
	double Rmin = m_Rmax/(pow(2, m_CWLevelCount - 1));
	double rate = std::exp(m_pn*(GetDcaTxop()->GetDeliveryDebt()));
	if (rate > Rmin){
		uint32_t level = uint32_t(std::max(0.0, ceil(log2(m_Rmax/rate)))) + 1;
		//std::random_device rd;
		//std::default_random_engine generator (rd());
		std::uniform_int_distribution<uint32_t> distribution(0, (m_CWMin*pow(2,(level - 1))) - 1);
		m_backoff = distribution(m_generator);
	} else {
		// if rate <= Rmin, let backoff number be extremely large
		m_backoff = m_CWMin*pow(2,m_CWLevelCount);
	}
	return m_backoff;
}

uint32_t
RTLinkParams::CalculateBackoff(std::vector<uint32_t> t)
{
	NS_LOG_FUNCTION (this);
    switch (m_algCode)
    {
    	case AlgorithmCode::ALG_DBDP:
    	{
    		m_swapId = t;
    		CalculateRTBackoff(t);
    		break;
    	}
    	case AlgorithmCode::ALG_FCSMA:
    	{
    		CalculateBackoffForFCSMA();
    		break;
    	}
    	case AlgorithmCode::ALG_LDF:
		{
    		m_backoff = 0;
    		break;
		}
    	default:
    	{
    		CalculateRTBackoff(t);
    		break;
    	}
    }
    return m_backoff;
}

uint32_t
RTLinkParams::GetBackoffAfterTxorRx(void)
{
	NS_LOG_FUNCTION (this);
    switch (m_algCode)
    {
    	case AlgorithmCode::ALG_DBDP:
    	{
    		m_backoff = 0;
    		break;
    	}
    	case AlgorithmCode::ALG_FCSMA:
    	{
    		CalculateBackoffForFCSMA();
    		break;
    	}
    	case AlgorithmCode::ALG_LDF:
		{
    		m_backoff = 0;
    		break;
		}
    	default:
    	{
    		m_backoff = 0;
    		break;
    	}
    }
    return m_backoff;
}

uint32_t
RTLinkParams::SetDcaBackoffAfterTxorRxIfNeeded(void)
{
	NS_LOG_FUNCTION (this);
	uint32_t backoff = 0;
	if (IsUsingFCSMA()){
		backoff = GetBackoffAfterTxorRx();
	    GetDcaTxop()->SetDeterministicBackoff(backoff);
	}
	return backoff;
}

uint32_t
RTLinkParams::ResetDcaBackoff(std::vector<uint32_t> t)
{
	NS_LOG_FUNCTION (this);
	uint32_t backoff = CalculateBackoff(t);
	GetDcaTxop()->SetDeterministicBackoff(backoff);
	/* Just for debugging
	if (backoff == 0){
		GetQn();
	}*/
	return backoff;
}

void
RTLinkParams::AddDeliveryDebt(double d)
{
	GetDcaTxop()->UpdateDeliveryDebt(d);
}

double
RTLinkParams::GetDeliveryDebt(void)
{
	return GetDcaTxop()->GetDeliveryDebt();
}

void
RTLinkParams::DecrementPacketCount(uint32_t n)
{
	m_packetCount = m_packetCount - std::min(n, m_packetCount);
}

void
RTLinkParams::CallSchedulerIfNeeded()
{
	if (IsUsingScheduler()){
		m_scheduler->ReceiveCallFromScheduledLink(this);
	}
}

void
RTLinkParams::UpdateDebt()
{
	GetDcaTxop()->ApplyDiffDeliveryDebt();
}

uint32_t
RTLinkParams::GetQueueLength()
{
	 return ((GetDcaTxop())->GetQueue())->GetSize();
}

bool
RTLinkParams::NoNeedToTransmitDummy()
{
	return (IsAlreadyTransmit() && IsUsingDummyPacket());
}

}


