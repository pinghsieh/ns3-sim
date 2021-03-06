/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
 *
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "dca-txop.h"
#include "dcf-manager.h"
#include "mac-low.h"
#include "wifi-mac-queue.h"
#include "mac-tx-middle.h"
#include "wifi-mac-trailer.h"
#include "wifi-mac.h"
#include "random-stream.h"
#include<random>
#include "RT-link-params.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT if (m_low != 0) { std::clog << "[mac=" << m_low->GetAddress () << "] "; }

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcaTxop");

class DcaTxop::Dcf : public DcfState
{
public:
  Dcf (DcaTxop * txop)
    : m_txop (txop)
  {
  }
  virtual bool IsEdca (void) const
  {
    return false;
  }
private:
  virtual void DoNotifyAccessGranted (void)
  {
    m_txop->NotifyAccessGranted ();
  }
  virtual void DoNotifyInternalCollision (void)
  {
    m_txop->NotifyInternalCollision ();
  }
  virtual void DoNotifyCollision (void)
  {
    m_txop->NotifyCollision ();
  }
  virtual void DoNotifyChannelSwitching (void)
  {
    m_txop->NotifyChannelSwitching ();
  }
  virtual void DoNotifySleep (void)
  {
    m_txop->NotifySleep ();
  }
  virtual void DoNotifyWakeUp (void)
  {
    m_txop->NotifyWakeUp ();
  }

  DcaTxop *m_txop;
};


/**
 * Listener for MacLow events. Forwards to DcaTxop.
 */
class DcaTxop::TransmissionListener : public MacLowTransmissionListener
{
public:
  /**
   * Create a TransmissionListener for the given DcaTxop.
   *
   * \param txop
   */
  TransmissionListener (DcaTxop * txop)
    : MacLowTransmissionListener (),
      m_txop (txop)
  {
  }

  virtual ~TransmissionListener ()
  {
  }

  virtual void GotCts (double snr, WifiMode txMode)
  {
    m_txop->GotCts (snr, txMode);
  }
  virtual void MissedCts (void)
  {
    m_txop->MissedCts ();
  }
  virtual void GotAck (double snr, WifiMode txMode)
  {
    m_txop->GotAck (snr, txMode);
  }
  virtual void MissedAck (void)
  {
    m_txop->MissedAck ();
  }
  virtual void StartNextFragment (void)
  {
    m_txop->StartNextFragment ();
  }
  virtual void StartNext (void)
  {
  }
  virtual void Cancel (void)
  {
    m_txop->Cancel ();
  }
  virtual void EndTxNoAck (void)
  {
    m_txop->EndTxNoAck ();
  }

private:
  DcaTxop *m_txop;
};

NS_OBJECT_ENSURE_REGISTERED (DcaTxop);

TypeId
DcaTxop::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcaTxop")
    .SetParent<ns3::Dcf> ()
    .SetGroupName ("Wifi")
    .AddConstructor<DcaTxop> ()
    .AddAttribute ("Queue", "The WifiMacQueue object",
                   PointerValue (),
                   MakePointerAccessor (&DcaTxop::GetQueue),
                   MakePointerChecker<WifiMacQueue> ())
  ;
  return tid;
}

DcaTxop::DcaTxop ()
  : m_manager (0),
    m_currentPacket (0),
	RT_decentralized (false),
	delivery_debt (0.0),
	channel_pn(0.0),
	m_diffDeliveryDebt(0.0)
{
  NS_LOG_FUNCTION (this);
  m_transmissionListener = new DcaTxop::TransmissionListener (this);
  m_dcf = new DcaTxop::Dcf (this);
  m_queue = CreateObject<WifiMacQueue> ();
  m_rng = new RealRandomStream ();
  m_currentIntervalEnd = Seconds(0);
  m_rtLinkParams = 0;
}

DcaTxop::~DcaTxop ()
{
  NS_LOG_FUNCTION (this);
}

void
DcaTxop::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_queue = 0;
  m_low = 0;
  m_stationManager = 0;
  delete m_transmissionListener;
  delete m_dcf;
  delete m_rng;
  m_transmissionListener = 0;
  m_dcf = 0;
  m_rng = 0;
  m_txMiddle = 0;
}

void
DcaTxop::SetManager (DcfManager *manager)
{
  NS_LOG_FUNCTION (this << manager);
  m_manager = manager;
  m_manager->Add (m_dcf);
}

void DcaTxop::SetTxMiddle (MacTxMiddle *txMiddle)
{
  m_txMiddle = txMiddle;
}

void
DcaTxop::SetLow (Ptr<MacLow> low)
{
  NS_LOG_FUNCTION (this << low);
  m_low = low;
}

void
DcaTxop::SetWifiRemoteStationManager (Ptr<WifiRemoteStationManager> remoteManager)
{
  NS_LOG_FUNCTION (this << remoteManager);
  m_stationManager = remoteManager;
}

void
DcaTxop::SetTxOkCallback (TxOk callback)
{
  NS_LOG_FUNCTION (this << &callback);
  m_txOkCallback = callback;
}

void
DcaTxop::SetTxFailedCallback (TxFailed callback)
{
  NS_LOG_FUNCTION (this << &callback);
  m_txFailedCallback = callback;
}

Ptr<WifiMacQueue >
DcaTxop::GetQueue () const
{
  NS_LOG_FUNCTION (this);
  return m_queue;
}

DcfManager*
DcaTxop::GetDcfManager() const
{
  NS_LOG_FUNCTION (this);
  return m_manager;
}


void
DcaTxop::SetMinCw (uint32_t minCw)
{
  NS_LOG_FUNCTION (this << minCw);
  m_dcf->SetCwMin (minCw);
}

void
DcaTxop::SetMaxCw (uint32_t maxCw)
{
  NS_LOG_FUNCTION (this << maxCw);
  m_dcf->SetCwMax (maxCw);
}

void
DcaTxop::SetAifsn (uint32_t aifsn)
{
  NS_LOG_FUNCTION (this << aifsn);
  m_dcf->SetAifsn (aifsn);
}

void
DcaTxop::SetTxopLimit (Time txopLimit)
{
  NS_LOG_FUNCTION (this << txopLimit);
  m_dcf->SetTxopLimit (txopLimit);
}

uint32_t
DcaTxop::GetMinCw (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetCwMin ();
}

uint32_t
DcaTxop::GetMaxCw (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetCwMax ();
}

uint32_t
DcaTxop::GetAifsn (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetAifsn ();
}

Time
DcaTxop::GetTxopLimit (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetTxopLimit ();
}

void
DcaTxop::Queue (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  NS_LOG_FUNCTION (this << packet << &hdr);
  WifiMacTrailer fcs;
  m_stationManager->PrepareForQueue (hdr.GetAddr1 (), &hdr, packet);
  m_queue->Enqueue (packet, hdr);
  StartAccessIfNeeded ();
}

int64_t
DcaTxop::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_rng->AssignStreams (stream);
  return 1;
}

void
DcaTxop::RestartAccessIfNeeded (void)
{
  NS_LOG_FUNCTION (this);
  // Keep clearing expired packets till find one valid packet in queue
  ClearExpiredPackets();
  //ClearExpiredPacketsAndDequeueValidOne();
  if ((m_currentPacket != 0
       || !m_queue->IsEmpty ())
      && !m_dcf->IsAccessRequested ())
    {
      m_manager->RequestAccess (m_dcf);
    }
  else{
	  /* It's possible that all packets are cleared by ClearExpiredPackets() */
	  if (m_currentPacket == 0 && m_queue->IsEmpty ()){
			/* No more packet available
			 * */
		    m_dcf->ResetAccessRequested();
			m_rtLinkParams->CallSchedulerIfNeeded();
	  }
  }
}

void
DcaTxop::StartAccessIfNeeded (void)
{
  NS_LOG_FUNCTION (this);
  // Keep clearing expired packets till find one valid packet in queue
  //ClearExpiredPacketsInDcaQueue();
  ClearExpiredPackets();
  if (m_currentPacket == 0
      && !m_queue->IsEmpty ()
      && !m_dcf->IsAccessRequested ())
    {
      m_manager->RequestAccess (m_dcf);
    }
  else{
	  /* It's possible that all packets are cleared by ClearExpiredPackets() */
	  if (m_currentPacket == 0 && m_queue->IsEmpty ()){
			/* No more packet available
			 * */
		    m_dcf->ResetAccessRequested();
			m_rtLinkParams->CallSchedulerIfNeeded();
	  }
  }
}

Ptr<MacLow>
DcaTxop::Low (void)
{
  NS_LOG_FUNCTION (this);
  return m_low;
}

void
DcaTxop::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  m_dcf->ResetCw ();
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
  ns3::Dcf::DoInitialize ();
}

bool
DcaTxop::NeedRtsRetransmission (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->NeedRtsRetransmission (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                                  m_currentPacket);
}

bool
DcaTxop::NeedDataRetransmission (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->NeedDataRetransmission (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                                   m_currentPacket);
}

bool
DcaTxop::NeedFragmentation (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->NeedFragmentation (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                              m_currentPacket);
}

void
DcaTxop::NextFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_fragmentNumber++;
}

uint32_t
DcaTxop::GetFragmentSize (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->GetFragmentSize (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                            m_currentPacket, m_fragmentNumber);
}

bool
DcaTxop::IsLastFragment (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->IsLastFragment (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                           m_currentPacket, m_fragmentNumber);
}

uint32_t
DcaTxop::GetNextFragmentSize (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->GetFragmentSize (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                            m_currentPacket, m_fragmentNumber + 1);
}

uint32_t
DcaTxop::GetFragmentOffset (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->GetFragmentOffset (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                              m_currentPacket, m_fragmentNumber);
}

Ptr<Packet>
DcaTxop::GetFragmentPacket (WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION (this << hdr);
  *hdr = m_currentHdr;
  hdr->SetFragmentNumber (m_fragmentNumber);
  uint32_t startOffset = GetFragmentOffset ();
  Ptr<Packet> fragment;
  if (IsLastFragment ())
    {
      hdr->SetNoMoreFragments ();
    }
  else
    {
      hdr->SetMoreFragments ();
    }
  fragment = m_currentPacket->CreateFragment (startOffset,
                                              GetFragmentSize ());
  return fragment;
}

bool
DcaTxop::NeedsAccess (void) const
{
  NS_LOG_FUNCTION (this);
  return !m_queue->IsEmpty () || m_currentPacket != 0;
}
void
DcaTxop::NotifyAccessGranted (void)
{
  NS_LOG_FUNCTION (this);
  ClearExpiredPackets();
  if (m_currentPacket == 0)
    {
      if (m_queue->IsEmpty ())
        {
          NS_LOG_DEBUG ("queue empty");
          return;
        }
      m_currentPacket = m_queue->Dequeue (&m_currentHdr);
      NS_ASSERT (m_currentPacket != 0);
      uint16_t sequence = m_txMiddle->GetNextSequenceNumberfor (&m_currentHdr);
      m_currentHdr.SetSequenceNumber (sequence);
      m_stationManager->UpdateFragmentationThreshold ();
      m_currentHdr.SetFragmentNumber (0);
      m_currentHdr.SetNoMoreFragments ();
      m_currentHdr.SetNoRetry ();
      m_fragmentNumber = 0;
      //m_currentHdrDup = m_currentHdr;
      NS_LOG_DEBUG ("dequeued size=" << m_currentPacket->GetSize () <<
                    ", to=" << m_currentHdr.GetAddr1 () <<
                    ", seq=" << m_currentHdr.GetSequenceControl ());
    }
  MacLowTransmissionParameters params;
  params.DisableOverrideDurationId ();


  if (m_currentHdr.GetAddr1 ().IsGroup ())
    {
      params.DisableRts ();
      params.DisableAck ();
      params.DisableNextData ();
      //if (!RT_decentralized ||
    		//  (RT_decentralized && IsPacketValidAfterTxAndAck(m_currentPacket, &m_currentHdr, params))){
  	  if (m_rtLinkParams != 0){
  		  m_rtLinkParams->SetAlreadyTransmit();
  	  }
      Low ()->StartTransmission (m_currentPacket,
                        &m_currentHdr,
                        params,
                        m_transmissionListener);
    	  NS_LOG_DEBUG ("tx broadcast");
    	  /*
    	   * Ping-Chun
    	   */
      	  //if (RT_decentralized && m_rtLinkParams != 0){
          //    if (m_rtLinkParams->IsIntentUp())
        	//	  {
        	//		  m_rtLinkParams->SetSwapActionUp();
        	//		  m_rtLinkParams->ResetSwapIntent();
        	//	  }
          //}
      //}
    }
  else
    {
      params.EnableAck ();

      if (NeedFragmentation ())
        {
          WifiMacHeader hdr;
          Ptr<Packet> fragment = GetFragmentPacket (&hdr);
          if (IsLastFragment ())
            {
              NS_LOG_DEBUG ("fragmenting last fragment size=" << fragment->GetSize ());
              params.DisableNextData ();
            }
          else
            {
              NS_LOG_DEBUG ("fragmenting size=" << fragment->GetSize ());
              params.EnableNextData (GetNextFragmentSize ());
            }
          //if (!RT_decentralized ||
        	//	  (RT_decentralized && IsPacketValidAfterTxAndAck(m_currentPacket, &m_currentHdr, params))){
          	  if (m_rtLinkParams != 0){
          		  m_rtLinkParams->SetAlreadyTransmit();
          	  }
        	 Low ()->StartTransmission (m_currentPacket,
                              &m_currentHdr,
                              params,
                              m_transmissionListener);
        	  /*
        	   * Ping-Chun
        	   */
          	  //if (RT_decentralized && m_rtLinkParams != 0){
              //    if (m_rtLinkParams->IsIntentUp())
            //		  {
            	//		  m_rtLinkParams->SetSwapActionUp();
            	//		  m_rtLinkParams->ResetSwapIntent();
            	//	  }
              //}
          //}
        }
      else
        {
              params.DisableNextData ();
    	  	  //if (!RT_decentralized ||
    	        // 		  (RT_decentralized && IsPacketValidAfterTxAndAck(m_currentPacket, &m_currentHdr, params))){
         	  /*
         	   * Ping-Chun
         	   */
               if (RT_decentralized && m_rtLinkParams != 0){
                   if (m_rtLinkParams->IsIntentUp())
             	    {
            	        m_rtLinkParams->SetSwapActionUp();
             		    m_rtLinkParams->ResetSwapIntent();
         	        }
                   if (m_rtLinkParams->GetIsUsingDummyPacket())
                   {
                	   //params.DisableAck ();
                   }
               }
           	   if (m_rtLinkParams != 0){
           		   m_rtLinkParams->SetAlreadyTransmit();
           	   }
    	        Low ()->StartTransmission (m_currentPacket,
    	                               &m_currentHdr,
    	                               params,
    	                               m_transmissionListener);

    	       //}

        }
    }
}

void
DcaTxop::NotifyInternalCollision (void)
{
  NS_LOG_FUNCTION (this);
  NotifyCollision ();
}

void
DcaTxop::NotifyCollision (void)
{
  NS_LOG_FUNCTION (this);
  /* Ping-Chun: for RT decentralized algorithm
   *   If is RT decentralized, then backoff=0  for the successive transmissions in the same interval
   * */
  if (RT_decentralized == false){
      m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
  } else {
	  //m_dcf->StartBackoffNow (uint32_t(0));
	  if (m_rtLinkParams != 0){
		  m_dcf->StartBackoffNow (m_rtLinkParams->GetBackoffAfterTxorRx());
	  }
  }
  RestartAccessIfNeeded ();
}

void
DcaTxop::NotifyChannelSwitching (void)
{
  NS_LOG_FUNCTION (this);
  m_queue->Flush ();
  m_currentPacket = 0;
}

void
DcaTxop::NotifySleep (void)
{
  NS_LOG_FUNCTION (this);
  if (m_currentPacket != 0)
    {
      m_queue->PushFront (m_currentPacket, m_currentHdr);
      m_currentPacket = 0;
    }
}

void
DcaTxop::NotifyWakeUp (void)
{
  NS_LOG_FUNCTION (this);
  RestartAccessIfNeeded ();
}

void
DcaTxop::GotCts (double snr, WifiMode txMode)
{
  NS_LOG_FUNCTION (this << snr << txMode);
  NS_LOG_DEBUG ("got cts");
}

void
DcaTxop::MissedCts (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("missed cts");
  if (!NeedRtsRetransmission ())
    {
      NS_LOG_DEBUG ("Cts Fail");
      m_stationManager->ReportFinalRtsFailed (m_currentHdr.GetAddr1 (), &m_currentHdr);
      if (!m_txFailedCallback.IsNull ())
        {
          m_txFailedCallback (m_currentHdr);
        }
      //to reset the dcf.
      m_currentPacket = 0;
      m_dcf->ResetCw ();
    }
  else
    {
      m_dcf->UpdateFailedCw ();
    }
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
  RestartAccessIfNeeded ();
}

void
DcaTxop::GotAck (double snr, WifiMode txMode)
{
  NS_LOG_FUNCTION (this << snr << txMode);
  if (!NeedFragmentation ()
      || IsLastFragment ())
    {
      NS_LOG_DEBUG ("got ack. tx done.");
      if (!m_txOkCallback.IsNull ())
        {
          m_txOkCallback (m_currentHdr);
        }

      /* Ping-Chun: for RT decentralized algorithm
       * If not using RT decentralized policy, then since we are not fragmenting or we are done fragmenting
       * so we can get rid of that packet now.
       * Else, we apply pseudo-unreliable transmissions here
       */
      std::random_device rd;
      std::default_random_engine generator (rd());
      std::uniform_real_distribution<double> distribution(0.0, 1.0);
      double rand_number = distribution(generator);

      bool RT_success = (rand_number < channel_pn)? true: false;

      if (RT_decentralized == false){
          m_currentPacket = 0;
          m_dcf->ResetCw ();
      } else {
    	  if (m_rtLinkParams != 0){
    		  /* Dummy packet is always at the back of the queue if any*/
    		  if (RT_success ||
    				  ((m_rtLinkParams->GetIsUsingDummyPacket()) && ((m_rtLinkParams->GetQueueLength()) == 0))){
    			  m_currentPacket = 0;
    			  m_dcf->ResetCw ();
    		  }  else {
    			  NS_LOG_DEBUG ("Retransmit");
    			  MacLowTransmissionParameters params;
    			  params.EnableAck ();
    			  if (IsPacketValidAfterTxAndAck(m_currentPacket, &m_currentHdr, params)){
                  //NeedDataRetransmission();
            	  //m_queue->PushFront(m_currentPacket, m_currentHdr);
            	  //Queue(m_currentPacket, m_currentHdr);
            	  //RTLinkParams* rtparam = GetDcfManager()->GetRTLinkParams();
            	  //rtparam->EnqueueOnePacket();
            	  // TODO...
    			  }
    			  //m_currentPacket = 0;
    			  m_dcf->ResetCw ();
    			  //m_currentHdr.SetRetry ();
    		  }
    	  }
      }

      /* Ping-Chun: for RT decentralized algorithm
       *   If is RT decentralized, then backoff=0  for the successive transmissions in the same interval
       * */
      if (RT_decentralized == false){
          m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
      } else {
    	  //m_dcf->StartBackoffNow (uint32_t(0));
    	  if (m_rtLinkParams != 0){
    		  m_dcf->StartBackoffNow(m_rtLinkParams->GetBackoffAfterTxorRx());
    		  if (RT_success  &&
    				  !(((m_rtLinkParams->GetIsUsingDummyPacket())) && ((m_rtLinkParams->GetQueueLength()) == 0))){
    			  UpdateDeliveryDebt (double(-1.0));
    			  m_rtLinkParams->IncrementTotalDeliveredPackets();
    			  if (! (m_rtLinkParams->IsUsingScheduler())){
    				  m_rtLinkParams->DecrementPacketCount(1);
    			  }
    			  m_rtLinkParams->CallSchedulerIfNeeded();
    		  } else {
    			  // do nothing so for...
    		  }
    	  }
      }

      RestartAccessIfNeeded ();
    }
  else
    {
      NS_LOG_DEBUG ("got ack. tx not done, size=" << m_currentPacket->GetSize ());
    }
}

void
DcaTxop::MissedAck (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("missed ack");
  if (!NeedDataRetransmission ())
    {
      NS_LOG_DEBUG ("Ack Fail");
      m_stationManager->ReportFinalDataFailed (m_currentHdr.GetAddr1 (), &m_currentHdr);
      if (!m_txFailedCallback.IsNull ())
        {
          m_txFailedCallback (m_currentHdr);
        }
      //to reset the dcf.
      m_currentPacket = 0;
      m_dcf->ResetCw ();
    }
  else
    {
      NS_LOG_DEBUG ("Retransmit");
      m_currentHdr.SetRetry ();
      m_dcf->UpdateFailedCw ();
    }
  /* Ping-Chun: for RT decentralized algorithm
   *   If is RT decentralized, then backoff=0  for the successive transmissions in the same interval
   * */
  if (RT_decentralized == false){
      m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
  } else {
	  //m_dcf->StartBackoffNow (uint32_t(0));
	  if (m_rtLinkParams != 0){
		  m_dcf->StartBackoffNow (m_rtLinkParams->GetBackoffAfterTxorRx());
	  }
  }
  RestartAccessIfNeeded ();
}

void
DcaTxop::StartNextFragment (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("start next packet fragment");
  /* this callback is used only for fragments. */
  NextFragment ();
  WifiMacHeader hdr;
  Ptr<Packet> fragment = GetFragmentPacket (&hdr);
  MacLowTransmissionParameters params;
  params.EnableAck ();
  params.DisableRts ();
  params.DisableOverrideDurationId ();
  if (IsLastFragment ())
    {
      params.DisableNextData ();
    }
  else
    {
      params.EnableNextData (GetNextFragmentSize ());
    }
  Low ()->StartTransmission (fragment, &hdr, params, m_transmissionListener);
}

void
DcaTxop::Cancel (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("transmission cancelled");
  /**
   * This happens in only one case: in an AP, you have two DcaTxop:
   *   - one is used exclusively for beacons and has a high priority.
   *   - the other is used for everything else and has a normal
   *     priority.
   *
   * If the normal queue tries to send a unicast data frame, but
   * if the tx fails (ack timeout), it starts a backoff. If the beacon
   * queue gets a tx oportunity during this backoff, it will trigger
   * a call to this Cancel function.
   *
   * Since we are already doing a backoff, we will get access to
   * the medium when we can, we have nothing to do here. We just
   * ignore the cancel event and wait until we are given again a
   * tx oportunity.
   *
   * Note that this is really non-trivial because each of these
   * frames is assigned a sequence number from the same sequence
   * counter (because this is a non-802.11e device) so, the scheme
   * described here fails to ensure in-order delivery of frames
   * at the receiving side. This, however, does not matter in
   * this case because we assume that the receiving side does not
   * update its <seq,ad> tupple for packets whose destination
   * address is a broadcast address.
   */
}

void
DcaTxop::EndTxNoAck (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("a transmission that did not require an ACK just finished");
  m_currentPacket = 0;
  m_dcf->ResetCw ();
  /* Ping-Chun: for RT decentralized algorithm
   *   If is RT decentralized, then backoff=0  for the successive transmissions in the same interval
   * */
  if (RT_decentralized == false){
      m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
  } else {
	  //m_dcf->StartBackoffNow (uint32_t(0));
	  if (m_rtLinkParams != 0){
		  m_dcf->StartBackoffNow (m_rtLinkParams->GetBackoffAfterTxorRx());
		  //UpdateDeliveryDebt (double(-1.0));
	  }
  }
  StartAccessIfNeeded ();
}

/* Ping-Chun: for RT-decentralized algorithm
 *
 */
void
DcaTxop::SetRTdecentralized(bool b)
{
	NS_LOG_FUNCTION (this);
	RT_decentralized = b;
}

void
DcaTxop::UpdateDeliveryDebt(double d)
{
	NS_LOG_FUNCTION (this);
	//delivery_debt += d;
	m_diffDeliveryDebt += d;
}

void
DcaTxop::ApplyDiffDeliveryDebt(void)
{
	NS_LOG_FUNCTION (this);
	delivery_debt += m_diffDeliveryDebt;
	m_diffDeliveryDebt = 0;
}

double
DcaTxop::GetDeliveryDebt()
{
	NS_LOG_FUNCTION (this);
	return delivery_debt;
}

void
DcaTxop::SetDeterministicBackoff (uint32_t backoff)
{
    NS_LOG_FUNCTION (this);
    m_dcf->ResetCw ();
    m_dcf->StartBackoffNow (backoff);
}

void
DcaTxop::SetChannelPn(double d)
{
	NS_LOG_FUNCTION (this);
	channel_pn = d;
}

void
DcaTxop::ClearExpiredPacketsAndDequeueValidOne()
{
	NS_LOG_FUNCTION (this);
	// Ping-Chun:
	// clear the expired packets, including the current packet and those in queue
	// starting from head of queue

	while (! m_queue->IsEmpty () || (m_currentPacket != 0)){
		if (m_currentPacket != 0){
    	// check if current packet will get expired after TX and ACK
    	// if still valid, then return;
    	// if not, keep checking the packets in queue
			MacLowTransmissionParameters params;
			params.EnableAck ();
			if (IsPacketValidAfterTxAndAck(m_currentPacket, &m_currentHdr, params)){
			    return;
			} else {
				// remove the current packet
			    m_currentPacket = 0;
			}
		} else  {
            	// get head of queue as the current packet
			    Ptr<const Packet> tempPacket = m_queue->Peek (&m_currentHdr);
        	    NS_ASSERT (tempPacket != 0);
        	    MacLowTransmissionParameters params;
        	    params.EnableAck ();
				if (IsPacketValidAfterTxAndAck(tempPacket, &m_currentHdr, params)){
					m_currentPacket = m_queue->Dequeue(&m_currentHdr);
                    uint16_t sequence = m_txMiddle->GetNextSequenceNumberfor (&m_currentHdr);
                    m_currentHdr.SetSequenceNumber (sequence);
                    m_stationManager->UpdateFragmentationThreshold ();
                    m_currentHdr.SetFragmentNumber (0);
                    m_currentHdr.SetNoMoreFragments ();
                    m_currentHdr.SetNoRetry ();
                    m_fragmentNumber = 0;
                    //m_currentHdrDup = m_currentHdr;
                    NS_LOG_DEBUG ("dequeued size=" << m_currentPacket->GetSize () <<
                              ", to=" << m_currentHdr.GetAddr1 () <<
                              ", seq=" << m_currentHdr.GetSequenceControl ());
                    return;
				} else {
					m_queue->Dequeue(&m_currentHdr);
					m_currentPacket = 0;
				}
    	}
    }
	/* No more packet available
	 * */
	m_rtLinkParams->CallSchedulerIfNeeded();

	/*
	 * Ping-Chun: cancel access request if there is no packet available
	 */
	//m_dcf->ResetAccessRequested();
}

void
DcaTxop::ClearExpiredPackets()
{
	NS_LOG_FUNCTION (this);
	// Ping-Chun:
	// clear the expired packets in queue, starting from head of queue
	// but does not check current packet

	while (! m_queue->IsEmpty () || (m_currentPacket != 0)){
			if (m_currentPacket != 0){
				/* Handle dummy packet*/
				if (m_rtLinkParams != 0 && m_queue->IsEmpty ()){
					if ((m_rtLinkParams->NoNeedToTransmitDummy())){
						 m_currentPacket = 0; // remove the current packet
					}
				}
	    	// check if current packet will get expired after TX and ACK
	    	// if still valid, then return;
	    	// if not, keep checking the packets in queue
				MacLowTransmissionParameters params;
				params.EnableAck ();
				if (IsPacketValidAfterTxAndAck(m_currentPacket, &m_currentHdr, params)){
				    return;
				} else {
					// remove the current packet
				    m_currentPacket = 0;
				}
			} else  {
					/* Handle dummy packet */
					if (m_rtLinkParams != 0 && (m_queue->GetSize () == 1)){
						if ((m_rtLinkParams->NoNeedToTransmitDummy())){
							m_queue->Dequeue(&m_currentHdr); // drop the packet
						}
					}
					if (! m_queue->IsEmpty ()){
						// get head of queue as the current packet
						Ptr<const Packet> tempPacket = m_queue->Peek (&m_currentHdr);
						NS_ASSERT (tempPacket != 0);
						MacLowTransmissionParameters params;
						params.EnableAck ();
						if (IsPacketValidAfterTxAndAck(tempPacket, &m_currentHdr, params)){
							return;
						} else {
							m_queue->Dequeue(&m_currentHdr);
						}
					}
	    	}
	 }
	/* No more packet available
	 * */
	//m_rtLinkParams->CallSchedulerIfNeeded();
}

bool
DcaTxop::IsPacketValidAfterTxAndAck(Ptr<const Packet> packet,
		const WifiMacHeader* hdr,
        const MacLowTransmissionParameters& params)
{
	NS_LOG_FUNCTION (this);
	// Ping-Chun:
	// for now, just assume that deadline = end of current interval
	// we may append deadline to each packet later
	Time tx_duration = m_low->CalculateOverallTxTime(packet, hdr, params);
	Time difs = m_manager->GetDifs();
	Time effective_tx_duration = tx_duration + difs ;
	if ((effective_tx_duration + Simulator::Now()) > m_currentIntervalEnd){
		return false;
	}
    return true;
}

void
DcaTxop::SetCurrentIntervalEnd(Time end_time)
{
	NS_LOG_FUNCTION (this);
	m_currentIntervalEnd = end_time;
}

void
DcaTxop::SetRTLinkParams(RTLinkParams* p)
{
	NS_LOG_FUNCTION (this);
    m_rtLinkParams = p;
    SetRTLinkParamsInDcfManager(p);
}

void
DcaTxop::SetRTLinkParamsInDcfManager(RTLinkParams* p)
{
	NS_LOG_FUNCTION (this);
    m_manager->SetRTLinkParams(p);
}

Ptr<WifiRemoteStationManager>
DcaTxop::GetStationManager() const
{
	NS_LOG_FUNCTION (this);
	return m_stationManager;
}

} //namespace ns3
