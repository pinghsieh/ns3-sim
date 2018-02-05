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
 * RT-link-params.h
 *
 *  Created on: Jan 22, 2018
 *      Author: Ping-Chun Hsieh
 */

#ifndef RT_LINK_PARAMS_H
#define RT_LINK_PARAMS_H

#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/output-stream-wrapper.h"
#include <random>
//#include "dca-txop.h"
//#include "ns3/assert.h"
//#include "ns3/packet.h"
//#include "ns3/log.h"
//#include "ns3/simulator.h"
//#include "ns3/uinteger.h"
//#include "ns3/pointer.h"

namespace ns3{

class DcaTxop;
class AdhocWifiMac;
class WifiNetDevice;
class OutputStreamWrapper;
class RTScheduler;

class RTLinkParams : public Object
{
public:
	 enum SwapStates
	 {
		  STATE_NONE,
		  STATE_LEAD,
		  STATE_TRAIL
	 };
	 enum SwapActions
	 {
		  ACT_NONE,
		  ACT_UP,
		  ACT_DOWN
	 };
	 enum SwapIntents
	 {
		  INT_NONE,
		  INT_UP,
		  INT_DOWN
	 };
	 enum ArrivalCode
	 {
		 ARR_BERN,
		 ARR_UNIF,
		 ARR_BERNUNIF,
		 ARR_CONST
	 };
	 enum AlgorithmCode
	 {
		 ALG_DBDP,
		 ALG_FCSMA,
		 ALG_LDF
	 };
	static TypeId GetTypeId (void);
	RTLinkParams();
    RTLinkParams(Ptr<WifiNetDevice>, Ptr<AdhocWifiMac>,
    		uint32_t ps, uint32_t pc, uint32_t lp, double qn, double R, double pn, uint32_t id, uint32_t bo, double ar,
			uint32_t cwm, uint32_t cwl, double rmax, uint32_t mpc, double mal);
    virtual ~RTLinkParams();
    void DoInitialize(Ptr<WifiNetDevice> nd, Ptr<AdhocWifiMac> wm,
    		uint32_t ps, uint32_t pc, uint32_t lp, double qn, double R, double pn,
			Ptr<OutputStreamWrapper> osw, uint32_t id, uint32_t bo, ArrivalCode ac, double ar, AlgorithmCode alc,
			uint32_t cwm, uint32_t cwl, double rmax, RTScheduler* sch, uint32_t mpc, double mal);
    //void DoDispose();

    Ptr<WifiNetDevice> GetNetDevice();
    Ptr<AdhocWifiMac> GetMacDest();
    uint32_t GetPacketSize() { return m_packetSize; }
    uint32_t GetPacketCount() { return m_packetCount; }
    uint32_t GetLinkPriority() { return m_linkPriority; }
    double GetQn() { return m_qn; }
    double GetPn() { return m_pn; }
    std::vector<uint32_t> GetSwapId() { return m_swapId;}
    double GetArrivalRate() { return m_arrivalRate;}
    uint32_t GetLinkId() { return m_linkId; }
    Ptr<DcaTxop> GetDcaTxop();
    Ptr<AdhocWifiMac> GetMacSource();
    bool GetIsUsingDummyPacket() { return m_isUsingDummyPacket;}
    void SetPacketSize(uint32_t ps) { m_packetSize = ps; }
    void SetPacketCount(uint32_t pc) { m_packetCount = pc; }
    void SetLinkPriority(uint32_t lp) { m_linkPriority = lp; }
    void SetSwapState(SwapStates s) { m_swapState = s; }
    void SetIsUsingDummyPacket() {m_isUsingDummyPacket = true;}
    void ResetIsUsingDummyPacket() {m_isUsingDummyPacket = false;}
    void ResetSwapState(void) { m_swapState = SwapStates::STATE_NONE; }
    void ResetSwapIntent(void) { m_swapIntent = SwapIntents::INT_NONE; }
    void ResetSwapAction(void) { m_swapAction = SwapActions::ACT_NONE; }
    bool IsStateLead(void) { return (m_swapState == SwapStates::STATE_LEAD);}
    bool IsStateTrail(void) { return (m_swapState == SwapStates::STATE_TRAIL);}
    bool IsIntentUp(void) { return (m_swapIntent == SwapIntents::INT_UP); }
    bool IsIntentDown(void) { return (m_swapIntent == SwapIntents::INT_DOWN); }
    bool IsActionUp(void) { return (m_swapAction == SwapActions::ACT_UP); }
    bool IsActionDown(void) { return (m_swapAction == SwapActions::ACT_DOWN); }
    void SetSwapActionUp(void) { m_swapAction = SwapActions::ACT_UP; }
    bool IsUsingFCSMA(void) { return m_algCode == AlgorithmCode::ALG_FCSMA; }
    bool IsUsingDBDP(void) { return m_algCode == AlgorithmCode::ALG_DBDP; }
    bool IsUsingScheduler(void) { return m_algCode == AlgorithmCode::ALG_LDF; }

    uint32_t CalculateRTBackoff(std::vector<uint32_t>);
    double CalculateAccessProbability(void);
    void UpdateLinkPriority(void);
    void ChangePriorityIfNeeded(void);
    void ResetAllSwapVariables(void);
    void PrintDeliveryDebtToFile(void);
    void EnqueueOnePacket(void);
    void EnqueueOneDummyPacket(void);
    void EnqueueMultiplePackets(uint32_t m);
    void EnqueueOnePacketGivenSize(uint32_t sz);
    void GeneratePacketCount(void);
    void EnqueueDummyPacketIfNeeded(void);
    uint32_t CalculateBackoffForFCSMA(void);
    uint32_t CalculateBackoff(std::vector<uint32_t>);
    uint32_t GetBackoffAfterTxorRx(void);
    uint32_t SetDcaBackoffAfterTxorRxIfNeeded(void);
    uint32_t ResetDcaBackoff(std::vector<uint32_t>);
    void AddDeliveryDebt(double);
    double GetDeliveryDebt(void);
    void DecrementPacketCount(uint32_t);
    void CallSchedulerIfNeeded();
    void UpdateDebt();
    uint32_t GetQueueLength();

private:
	Ptr<WifiNetDevice> m_netDev;
	Ptr<AdhocWifiMac> m_macDest;
	uint32_t m_packetSize;
	uint32_t m_packetCount;
    uint32_t m_linkPriority;
    double m_qn;  // target delivery ratio
    double m_R; // constant for calculating access probability
    double m_pn; // channel reliability
    SwapStates m_swapState;
    SwapActions m_swapAction; // to indicate whether this link shall move up or down in priority list in next interval
    SwapIntents m_swapIntent;  // to indicate whether this link intends to move up or down in priority list
    Ptr<OutputStreamWrapper> m_stream;
    uint32_t m_linkId;
    uint32_t m_backoff;
    ArrivalCode m_arrivalProcess;
    double m_arrivalRate;
    AlgorithmCode m_algCode;
    std::minstd_rand0 m_generator ;
    uint32_t m_sizeDummyPacket;
    std::vector<uint32_t> m_swapId;
    bool m_isUsingDummyPacket;
    /* For FCSMA*/
    uint32_t m_CWMin;
    uint32_t m_CWLevelCount;
    double m_Rmax;
    /* For LDF*/
    RTScheduler* m_scheduler;
    /* For arrival process*/
    uint32_t m_maxPacketCount;
    double m_alpha;

};

}

#endif

