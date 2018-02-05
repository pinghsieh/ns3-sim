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
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/output-stream-wrapper.h"
#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <ctime>
#include <cstring>
#include <sstream>

//#define DEBUG 1
// Default Network Topology
// 
// AP
// *      *
// n0     n1
//


using namespace ns3;

uint32_t MacTXCount;


void
MacTX (Ptr<const Packet> p)
{
	//NS_LOG_INFO("Packet Received");
	MacTXCount++;
	std::cout << Simulator::Now().GetSeconds() << "\t" << "MacTXCount = " << MacTXCount << "\n";
}

void
FlushMacQueue  (RTLinkParams* param)
{
	Ptr<DcaTxop> dca = param->GetDcaTxop();
	Ptr<WifiMacQueue> m_queue = dca->GetQueue();
	/* Flush the expired packets */
	m_queue->Flush();

	//NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Queue size = " << m_queue->GetSize());
	//NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Is empty? " << m_queue->IsEmpty());
}

void
SetDeterministicBackoffNow (Ptr<WifiNetDevice> netdev, uint32_t backoff)
{
	Ptr<AdhocWifiMac> mac_source = netdev->GetMac()->GetObject<AdhocWifiMac>();
	Ptr<DcaTxop> dca = mac_source->GetDcaTxop();
	dca->SetDeterministicBackoff(backoff);
}

void
StartNewIntervalForScheduler (RTScheduler* sch, double rel_time)
{
	Time t = Simulator::Now();
	sch->SetCurrentIntervalEnd(Simulator::Now() + Seconds(rel_time));

	/* Update delivery debt */
	sch->UpdateDebt();

	/* Generate packet count */
	sch->GeneratePacketCountForLinks();

	/* Increase delivery debt */
	sch->AddQnDeliveryDebtForLinks();

	/* Logging */
	for (uint32_t i = 0; i < (sch->GetRTLinkCount()); i++){
		sch->GetRTLinkAtPosition(i)->PrintDeliveryDebtToFile();
	}

	//NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": in scheduler\n");

	/* Scheduling starts */
	sch->StartSchedulingTransmissionsNow();
}
void
StartNewIntervalForDistributedLink (RTLinkParams* param, double rel_time, uint32_t nRT, uint32_t rand_number)
{
	/* (i) Cancel on-going transmissions or (ii) should we check timing before sending?
	 * Currently, choose option (ii)
	 * */

	if (param->IsUsingFCSMA() || param->IsUsingDBDP())
	{
		/* Set end of current interval */
		Time t = Simulator::Now();
		(param->GetDcaTxop())->SetCurrentIntervalEnd(Simulator::Now() + Seconds(rel_time));

		/* Update priority */
		param->UpdateLinkPriority();

		/* Update delivery debt*/
		param->UpdateDebt();

		/* Reset parameters for swapping */
    	param->ResetAllSwapVariables();

    	/* Reset dummy packet*/
    	param->ResetIsUsingDummyPacket();

		/* Reassign backoff timer */
    	param->ResetDcaBackoff(rand_number);
    	//param->ResetDcaBackoff(9); //Only for DEBUG

		/* Generate packet count */
		param->GeneratePacketCount();

		/* Increase delivery debt */
		param->AddDeliveryDebt((param->GetQn())*(param->GetArrivalRate()));

		/* Get packet arrivals */
		param->EnqueueMultiplePackets(param->GetPacketCount());
		param->EnqueueDummyPacketIfNeeded();

		/* Logging */
	    param->PrintDeliveryDebtToFile();
	}
	//NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Queue size = " << m_queue->GetSize());
	//NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Delivery Debt =  " << dca->GetDeliveryDebt());
}

void
PrintHeadersToFile(Ptr<OutputStreamWrapper> stream)
{
	*(stream->GetStream()) << std::setw(10) << "Link ID" << std::setw(12) << "Timestamp" << std::setw(20)
			<< "Delivery Debt" << std::setw(10) << "Priority" << std::setw(15) << "Backoff" << std::setw(16)
			<< "Queue Length" << std::setw(15) << "Packet Count" << "\n";
}

void
PrintDashLines(Ptr<OutputStreamWrapper> stream)
{
	*(stream->GetStream()) << "-----------------------------------------------------------------------------------------------------" << "\n";
}

bool
CheckIfFileExists(const std::string& filename)
{
	std::ifstream infile(filename);
	return infile.good();
}

void
ConfigRTdecentralized (RTLinkParams* param, uint32_t maxRetry)
{
	Ptr<DcaTxop> dca = param->GetDcaTxop();
	dca->SetRTdecentralized(true);
	dca->SetChannelPn(param->GetPn());
	dca->SetRTLinkParams(param);
	dca->GetStationManager()->SetMaxSlrc(maxRetry);
}

void
ReceivePacket (Ptr<Socket> socket)
{
	if (socket->Recv()) {
		NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Received one packet! \n");
	}
}

void
DeleteCustomObjects(std::vector<RTLinkParams*> paramVec, RTScheduler* sch)
{
    for (uint32_t i =0 ; i < paramVec.size(); i++){
    	delete paramVec[i];
    }
    delete sch;
}
/*
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, uint32_t packetCount)
{
	if (packetCount > 0){
		for (uint32_t i = 0; i < packetCount; i++){
			socket->Send(Create<Packet> (pktSize));
		}
		//Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize, packetCount-1, pktInterval);
	}
}
*/

/*
static void CloseSocket (Ptr<Socket> socket)
{
	socket->Close();
}
*/

NS_LOG_COMPONENT_DEFINE ("RT-decentralized");


int
main (int argc, char *argv[])
{

	/* Measure system time*/
	clock_t tStart = clock();

	/* Network-wide parameters */
    bool verbose = true;
    bool tracing = true;

    /* Declaration */
    uint32_t nRT = 1;
    double packet_interval = 1;
    double startT = 0;
    uint32_t nIntervals = 100;
    double stopT = 1;
    double offset = 0;
    uint32_t packetSize = 10;
    uint32_t packetCount = 1;
    uint32_t maxRetry = 1024;
    double p = 1;
    double p1 = 1;
    double p2 = 1;
    std::vector<double> channel_pn;
    double q = 1;
    std::vector<double> qn;
    double R = 1;
    double alpha = 0.75;
    std::vector<double> alphan;
    uint32_t maxPacketCount = 1;
    double lambda = 1;
    double lambda1 = 1;
    double lambda2 = 1;
    std::vector<double> arrivalRate;
    RTLinkParams::AlgorithmCode algcode = RTLinkParams::AlgorithmCode::ALG_LDF;
    RTLinkParams::ArrivalCode arrcode = RTLinkParams::ArrivalCode::ARR_BERNUNIF;
    uint32_t CWMin = 32;
    uint32_t CWLevelCount = 6;
    double Rmax = exp(5);
    std::string debtlogpath;
    std::string policy;
    /* Test case */
    uint32_t testId = 5;

    /* Handle input arguments
	 * argv[1]: number of intervals
	 * argv[2]: case id
	 * argv[3]: variable
	 * */
    CommandLine cmd;
    cmd.AddValue("nIntervals", "Number of intervals in simulation", nIntervals);
    cmd.AddValue("testId", "Index of test case", testId);
    cmd.AddValue("alpha", "a control variable of arrival rates of Bern-Unif distribution", alpha);
    cmd.AddValue("q", "a control variable of delivery ratio", q);
    cmd.AddValue("lambda", "a control variable of arrival rates of Bernoulli distribution", lambda);
    cmd.AddValue("policy", "policy to be used for links", policy);
    cmd.Parse(argc, argv);
    //NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << "policy is " << policy << "\n");

    if (policy.compare("DBDP") == 0){
    	algcode = RTLinkParams::AlgorithmCode::ALG_DBDP;
    	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ":Using DBDP! \n");
    }  else if (policy.compare("FCSMA") == 0){
    	algcode = RTLinkParams::AlgorithmCode::ALG_FCSMA;
    	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ":Using FCSMA! \n");
    }  else if (policy.compare("LDF") == 0){
    	algcode = RTLinkParams::AlgorithmCode::ALG_LDF;
    	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ":Using LDF! \n");
    }  else{
    	algcode = RTLinkParams::AlgorithmCode::ALG_LDF;
    	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ":Using default (LDF)! \n");
    }
/*
	if (argc >= 2){
		nIntervals = std::stoi(std::string(argv[1]));
	}
	if (argc >= 3){
		uint32_t tempId = std::stoi(std::string(argv[2]));
		if (tempId <= 6 && tempId >= 1){
			testId = tempId;
		}
	}
	if (argc >= 4){
		switch(testId) {
			case 1:{
				alpha = std::stod(std::string(argv[3]));
				break;
			}
			case 2:{
				q = std::stod(std::string(argv[3]));
				break;
			}
			case 3:{
				alpha = std::stod(std::string(argv[3]));
				break;
			}
			case 4:{
				q = std::stod(std::string(argv[3]));
				break;
			}
			case 5:{
				lambda = std::stod(std::string(argv[3]));
				break;
			}
			case 6:{
				q = std::stod(std::string(argv[3]));
				break;
			}
		}
	}
	*/

     switch(testId){
     	 case 1: {
     		 /*  Testcase 1: 20 links, symmetric, Bern-Unif arrivals between 1~6, pn=0.7
     		  *  fix qn = 0.9, change alpha
     		  * */
     		 nRT = 21; // AP is 00:00:00:00:00:01
     		 packet_interval = 0.02; // 20ms
     		 startT = 2.5;
     		 stopT = startT + nIntervals*packet_interval;
     		 offset = 0.000001; // 1us
     		 packetSize = 1500; // TX + ACK = 330us
     		 packetCount = 1;
     		 maxRetry = 1024;
     		 p = 0.7;
     		 channel_pn.assign(nRT-1,p); // for unreliable transmissions
     		 q = 0.9;
     		 qn.assign(nRT-1,q);
     		 R = 10;
     		 //alpha = 0.45;
     		 alphan.assign(nRT-1, alpha);
     		 maxPacketCount = 6;
     		 lambda = ((double(1 + maxPacketCount))/2.0)*alpha;
     		 arrivalRate.assign(nRT-1, lambda);
     		 //algcode = RTLinkParams::AlgorithmCode::ALG_FCSMA;
     		 arrcode = RTLinkParams::ArrivalCode::ARR_BERNUNIF;
     		 CWMin = 32;
     		 CWLevelCount = 6;
     		 Rmax = exp(5);
     	     std::stringstream sstream;
     	     sstream << std::fixed << std::setprecision(2) << alpha;
     	     debtlogpath = "RT-delivery-debt-" + std::to_string(testId) +"-" + std::to_string(nIntervals) + "-" + policy + "-" + "alpha=" + sstream.str() + ".txt";
     		 break;
     	 }

     	 case 2: {
     		 /*  Testcase 1: 20 links, symmetric, Bern-Unif arrivals between 1~6, pn=0.7
     		  *  fix alpha = 0.55, change qn
     		  * */
     		 nRT = 21; // AP is 00:00:00:00:00:01
     		 packet_interval = 0.02; // 20ms
     		 startT = 2.5;
     		 stopT = startT + nIntervals*packet_interval;
     		 offset = 0.000001; // 1us
     		 packetSize = 1500; // TX + ACK = 330us
     		 packetCount = 1;
     		 maxRetry = 1024;
     		 p = 0.7;
     		 channel_pn.assign(nRT-1,p);  // for unreliable transmissions
     		 //q = 0.95;
     		 qn.assign(nRT-1,q);
     		 R = 10;
     		 alpha = 0.55;
     		 alphan.assign(nRT-1, alpha);
     		 maxPacketCount = 6;
     		 lambda = ((double(1 + maxPacketCount))/2.0)*alpha;
     		 arrivalRate.assign(nRT-1, lambda);
     		 //algcode = RTLinkParams::AlgorithmCode::ALG_LDF;
     		 arrcode = RTLinkParams::ArrivalCode::ARR_BERNUNIF;
     		 CWMin = 32;
     		 CWLevelCount = 6;
     		 Rmax = exp(5);
     	     std::stringstream sstream;
     	     sstream << std::fixed << std::setprecision(2) << q;
     	     debtlogpath = "RT-delivery-debt-" + std::to_string(testId) + "-" + std::to_string(nIntervals) + "-" + policy + "-" + "q=" + sstream.str() + ".txt";
     		 break;
     	 }
     	 case 3: {
     		 /*  Testcase 3: 20 links, asymmetric, Bern-Unif arrivals between 1~6
     		  *  2 groups
     		  *  group 1: pn = 0.8, alphan = alpha
     		  *  group 2: pn = 0.5, alphan = 0.5*alpha
     		  *  fix qn=0.9, change alpha
     		  * */
     		 nRT = 21; // AP is 00:00:00:00:00:01
     		 packet_interval = 0.02; // 20ms
     		 startT = 2.5;
     		 stopT = startT + nIntervals*packet_interval;
     		 offset = 0.000001; // 1us
     		 packetSize = 1500; // TX + ACK = 330us
     		 packetCount = 1;
     		 maxRetry = 1024;
     		 p1 = 0.8;
     		 p2 = 0.5;
     		 channel_pn.assign((nRT-1)/2, p1);
     		 channel_pn.insert(channel_pn.begin(), (nRT-1)/2, p2); // for unreliable transmissions
     		 q = 0.9;
     		 qn.assign(nRT-1, q);
     		 R = 10;
     		 //alpha = 0.77;
     		 alphan.assign((nRT-1)/2, alpha);
     		 alphan.insert(alphan.begin(), (nRT-1)/2, 0.5*alpha);
     		 maxPacketCount = 6;
     		 lambda1 = ((double(1 + maxPacketCount))/2.0)*alpha;
     		 lambda2 = ((double(1 + maxPacketCount))/2.0)*0.5*alpha;
     		 arrivalRate.assign((nRT-1)/2, lambda1);
     		 arrivalRate.insert(arrivalRate.begin(), (nRT-1)/2, lambda2);
     		 //algcode = RTLinkParams::AlgorithmCode::ALG_LDF;
     		 arrcode = RTLinkParams::ArrivalCode::ARR_BERNUNIF;
     		 CWMin = 32;
     		 CWLevelCount = 6;
     		 Rmax = exp(5);
     	     std::stringstream sstream;
     	     sstream << std::fixed << std::setprecision(2) << alpha;
     	     debtlogpath = "RT-delivery-debt-" + std::to_string(testId) + "-" + std::to_string(nIntervals) + "-" + policy + "-" + "alpha=" + sstream.str() + ".txt";
     		 break;
     	 }
     	case 4: {
     	     		 /*  Testcase 4: 20 links, deadline 20ms, asymmetric, Bern-Unif arrivals between 1~6
     	     		  *  2 groups
     	     		  *  group 1: pn = 0.8, alphan = alpha
     	     		  *  group 2: pn = 0.5, alphan = 0.5*alpha
     	     		  *  fix alpha=0.7, change qn
     	     		  * */
    		 nRT = 21; // AP is 00:00:00:00:00:01
    		 packet_interval = 0.02; // 20ms
    		 startT = 2.5;
    		 stopT = startT + nIntervals*packet_interval;
    		 offset = 0.000001; // 1us
    		 packetSize = 1500; // TX + ACK = 330us
    		 packetCount = 1;
    		 maxRetry = 1024;
    		 p1 = 0.8;
    		 p2 = 0.5;
     		 channel_pn.assign((nRT-1)/2, p1);
     		 channel_pn.insert(channel_pn.begin(), (nRT-1)/2, p2); // for unreliable transmissions
    		 //q = 0.9;
     		 qn.assign(nRT-1, q);
    		 R = 10;
    		 alpha = 0.7;
     		 alphan.assign((nRT-1)/2, alpha);
     		 alphan.insert(alphan.begin(), (nRT-1)/2, 0.5*alpha);
    		 maxPacketCount = 6;
     		 lambda1 = ((double(1 + maxPacketCount))/2.0)*alpha;
     		 lambda2 = ((double(1 + maxPacketCount))/2.0)*0.5*alpha;
     		 arrivalRate.assign((nRT-1)/2, lambda1);
     		 arrivalRate.insert(arrivalRate.begin(), (nRT-1)/2, lambda2);
    		 //algcode = RTLinkParams::AlgorithmCode::ALG_LDF;
    		 arrcode = RTLinkParams::ArrivalCode::ARR_BERNUNIF;
    		 CWMin = 32;
    		 CWLevelCount = 6;
    		 Rmax = exp(5);
     	     std::stringstream sstream;
     	     sstream << std::fixed << std::setprecision(2) << q;
     	     debtlogpath = "RT-delivery-debt-" + std::to_string(testId) + "-" + std::to_string(nIntervals) + "-" + policy + "-" + "q=" + sstream.str() + ".txt";
    		 break;
     	     	 }
      	 case 5: {
   	     		 /*  Testcase 5: 10 links, deadline = 2ms, 100Byte, symmetric, Bernoulli arrivals, pn = 0.7
   	     		  *  fix qn=0.99, change arrival rate (lambda)
   	     		  * */
        		 nRT = 11; // AP is 00:00:00:00:00:01
        		 packet_interval = 0.002; // 2ms
        		 startT = 2.5;
        		 stopT = startT + nIntervals*packet_interval;
        		 offset = 0.000001; // 1us
        		 packetSize = 100; // TX + ACK = 120us
        		 packetCount = 1;
        		 maxRetry = 1024;
        		 p = 0.7;
        		 channel_pn.assign(nRT-1,p); // for unreliable transmissions
 	     		 q = 0.99;
 	     		 qn.assign(nRT-1,q);
        		 R = 10;
         		 alpha = 0.55;
         		 alphan.assign(nRT-1,alpha);
         		 maxPacketCount = 6;
         		 //lambda = 0.84;
         		 arrivalRate.assign(nRT-1,lambda);
        		 //algcode = RTLinkParams::AlgorithmCode::ALG_DBDP;
        		 arrcode = RTLinkParams::ArrivalCode::ARR_BERN;
        		 CWMin = 32;
        		 CWLevelCount = 6;
        		 Rmax = exp(5);
         	     std::stringstream sstream;
         	     sstream << std::fixed << std::setprecision(2) << lambda;
         	     debtlogpath = "RT-delivery-debt-" + std::to_string(testId) + "-" + std::to_string(nIntervals) + "-" + policy + "-" + "lambda=" + sstream.str() + ".txt";
        		 break;
        	 }
    	 case 6: {
	     		 /*  Testcase 6: 10 links, deadline = 2ms, 100Byte, symmetric, Bernoulli arrivals, pn = 0.7
	     		  *  fix arrival rate (lambda)=0.78, change qn
	     		  * */
    		 nRT = 11; // AP is 00:00:00:00:00:01
    		 packet_interval = 0.002; // 2ms
    		 startT = 2.5;
    		 stopT = startT + nIntervals*packet_interval;
    		 offset = 0.000001; // 1us
    		 packetSize = 100; // TX + ACK = 120us
    		 packetCount = 1;
    		 maxRetry = 1024;
    		 p = 0.7;
    		 channel_pn.assign(nRT-1,p); // for unreliable transmissions
    		 q = 0.99;
    		 qn.assign(nRT-1,q);
    		 R = 10;
     		 alpha = 0.55;
     		 alphan.assign(nRT-1,alpha);
     		 maxPacketCount = 6;
     		 lambda = 0.78;
     		 arrivalRate.assign(nRT-1,lambda);
    		 //algcode = RTLinkParams::AlgorithmCode::ALG_LDF;
    		 arrcode = RTLinkParams::ArrivalCode::ARR_BERN;
    		 CWMin = 32;
    		 CWLevelCount = 6;
    		 Rmax = exp(5);
     	     std::stringstream sstream;
     	     sstream << std::fixed << std::setprecision(2) << q;
     	     debtlogpath = "RT-delivery-debt-" + std::to_string(testId) + "-" + std::to_string(nIntervals) + "-" + policy + "-" + "q=" + sstream.str() + ".txt";
    		 break;
     	 }

     };

         RTScheduler* scheduler = new RTScheduler();

    std::string backoffLog ("RT-backoff.log");
    std::random_device rd;
    std::default_random_engine generator (rd());
    std::uniform_int_distribution<uint32_t> distribution(1, nRT - 2);

    //if (!CheckIfFileExists(debtlogpath)) {
    	//Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper>(debtlogpath, std::ios::app);
    	//*(stream->GetStream()).clear();
    	//std::remove(debtlogpath.c_str());
    //}

    Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper>(debtlogpath, std::ios::trunc);
    PrintHeadersToFile(stream);

    //CommandLine cmd;
    //cmd.AddValue ("verbose", "Tell echo application to log if true", verbose);
    //cmd.AddValue ("nRT", "Number of real-time distributed WiFi devices, excluding AP", nRT);

    //cmd.Parse (argc, argv);

    WifiHelper wifi;

    if (verbose)
    {
        /*
         * Ping-Chun: disable WiFi logging for faster simulations
         */
    	//wifi.EnableLogComponents ();  // Turn on all Wifi logging

    	//LogComponentEnableAll(LOG_PREFIX_NODE);
        //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
        //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
        //LogComponentEnable ("DcfManager", LOG_LEVEL_INFO);
    }

    /* Create log streams */
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> backoff_stream = ascii.CreateFileStream (backoffLog);

    /* Create nRT station node objects */
    NodeContainer wifiRTStaNodes;
    wifiRTStaNodes.Create (nRT); 

    /* Create a channel helper and phy helper, and then create the channel */
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create());

    std::string phyMode ("OfdmRate54Mbps");
    //std::string phyMode ("OfdmRate6Mbps");
    //wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode));
    wifi.SetStandard (WIFI_PHY_STANDARD_80211a);

    NetDeviceContainer wifiStaDevices;

    /* Non-QoS Wifi helper */
    NqosWifiMacHelper mac;
    // There are 3 major mac tpes for WiFi:
    // ns3::ApWifiMac, ns3::StaWifiMac, and ns3::AdhocWifiMac
    // All the above mac models are children of ns3::regularWifiMac
    mac.SetType ("ns3::AdhocWifiMac");
    wifiStaDevices = wifi.Install (phy, mac, wifiRTStaNodes);


    /* Mobility */
    MobilityHelper mobility;
    // Setup grid: objects are layed out, starting from (0,0) with 3 objects per row
    // the x interval between each object is 5 meters
    // the y interval between each object is 10 meters

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (0.5),
                                   "DeltaY", DoubleValue (0.5),
                                   "GridWidth", UintegerValue (3),
                                   "LayoutType", StringValue ("RowFirst"));

    // each object will be put in a static position
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                              // "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));

    mobility.Install (wifiRTStaNodes);

    /* Internet Stack */
    InternetStackHelper stack;
    stack.Install (wifiRTStaNodes);

    Ipv4InterfaceContainer wifiInterfaces;
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    wifiInterfaces = address.Assign (wifiStaDevices);


    /* Sockets */
    /*
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    Ptr<Socket> recvSink = Socket::CreateSocket (wifiRTStaNodes.Get(0), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), 80);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

    Ptr<Socket> source = Socket::CreateSocket (wifiRTStaNodes.Get(nRT - 1), tid);
    //InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
    InetSocketAddress remote = InetSocketAddress (wifiInterfaces.GetAddress(0), 80);
    source->SetAllowBroadcast (true);
    source->Connect (remote);
	*/

    /* UDP Server */
    /*
    UdpServerHelper echoServer (9);

    ApplicationContainer serverApps = echoServer.Install (wifiRTStaNodes.Get(0));
    serverApps.Start (Seconds (startT - 1.0));
    serverApps.Stop (Seconds (stopT));
    */

    /* UDP Client */
    /*
    UdpClientHelper echoClient (wifiInterfaces.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (packet_interval)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));

    ApplicationContainer clientApps = echoClient.Install (wifiRTStaNodes.Get(nRT-1));
    clientApps.Start (Seconds (startT));
    clientApps.Stop (Seconds (endT));
    */

    /* Routing */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    /* Initialize per-link parameters */
    std::vector<RTLinkParams*> paramVec;
    std::vector<Ptr<OutputStreamWrapper>> streamVec;

    for (uint32_t i =1 ; i < nRT; i++){
        RTLinkParams* p =  new RTLinkParams();
        /*
         * Ping-Chun: assume initial priority equals link ID
         */
        p->DoInitialize(wifiStaDevices.Get(i)->GetObject<WifiNetDevice>(),
        		wifiStaDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac() -> GetObject<AdhocWifiMac>(),
				 packetSize, packetCount, i, qn[i-1], R, channel_pn[i-1], stream, i, uint32_t(0),
				 arrcode, arrivalRate[i-1], algcode, CWMin, CWLevelCount, Rmax, scheduler, maxPacketCount, alphan[i-1]);
        paramVec.push_back(p);
        scheduler->AddOneNewRTLink(p);
    }

    /* Simulator events: configuration */
    for (uint32_t i = 1; i < nRT; i++)
    {
    	Ptr<WifiNetDevice> netdev = wifiStaDevices.Get(i-1)->GetObject<WifiNetDevice>();
    	Simulator::ScheduleWithContext(i, Seconds(startT - offset),
    	        			&ConfigRTdecentralized, paramVec[i-1], maxRetry);
    }

    /* Simulator events: packet transmissions */
    for (uint32_t t = 0; t < nIntervals; t++)
    {
    	/* Choose one swapping pair in each interval*/
    	// links at priority (rand_number, rand_number+1) is the swapping pair
        uint32_t rand_number = distribution(generator);

        /* Schedule events for centralized link scheduler */
        if (scheduler->GetRTLinkCount() > 0){
        	Simulator::ScheduleWithContext(t, Seconds(startT + packet_interval*double(t)+(2.0)*offset),
        		&StartNewIntervalForScheduler, scheduler, double(packet_interval-(2.0)*offset));
        }

        /* Tracing for MAC events */
        for (uint32_t i = 0; i < nRT - 1; i++)
        {
        	Ptr<WifiNetDevice> netdev = wifiStaDevices.Get(i)->GetObject<WifiNetDevice>();

        	// Suppose uplink traffic: AP is the destination of all the clients
        	Ptr<AdhocWifiMac> mac_dest = wifiStaDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac() -> GetObject<AdhocWifiMac>();
        	//Simulator::ScheduleWithContext(i, Seconds(startT + packet_interval*double(t)+offset),
        	        			//&FlushMacQueue, netdev);
        	Simulator::ScheduleWithContext(i, Seconds(startT + packet_interval*double(t)+offset),
        	        			&FlushMacQueue, paramVec[i]);

        	Simulator::ScheduleWithContext(i, Seconds(startT + packet_interval*double(t)+(2.0)*offset),
        			&StartNewIntervalForDistributedLink, paramVec[i], double(packet_interval-(2.0)*offset), nRT, rand_number);


        	//for (uint32_t j = 0; j < packetCount; j++){
        	    //Simulator::ScheduleWithContext(i, Seconds(startT + interval*double(t) + offset*double(j)), &GenerateTraffic, source, packetSize, uint32_t(1));
        	//}
            //Ptr<RegularWifiMac> regmac = wifiApDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac() -> GetObject<RegularWifiMac>() ;
        	//Ptr<EdcaTxopN> edca = regmac -> GetBEQueue();
            //std::cout << "Max CW = " << edca->GetMaxCw() << std::endl;
            //DcfManager* dcf_manager = regmac -> GetDcfManager();
            //std::cout << ""dcf_manager->m_states.size() > 0)

            //Ptr<DcfManager> manager = wifiApDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac() -> GetObject<RegularWifiMac>() -> GetDcfManager();
        	//Ptr<WifiMac> theObject = wifiApDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac();
            //theObject->TraceConnectWithoutContext("MacTX", MakeCallback (&MacTX));
            //Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTX", MakeCallback(&MacTX));
        }
/*
        Simulator::ScheduleWithContext(t, Seconds(startT + packet_interval*double(t)+(1.5)*offset),
                			&PrintDashLines, stream);
*/
    }

    Simulator::Stop(Seconds (stopT));
    //Simulator::ScheduleWithContext (source->GetNode ()->GetId(), Seconds(stopT), &CloseSocket, source);
    //Simulator::ScheduleWithContext (recvSink->GetNode ()->GetId(), Seconds(stopT), &CloseSocket, recvSink);

    if (tracing)
    {
        phy.EnablePcap ("RT-decentralized", wifiStaDevices);
        //wifi.EnableAsciiAll (ascii.CreateFileStream ("RT-decentralized.tr"));
    }


    Simulator::Run();
    DeleteCustomObjects(paramVec, scheduler);
    Simulator::Destroy();

    std::cout << "Time taken: " << std::setprecision(5) << (double)(clock() - tStart)/CLOCKS_PER_SEC << " seconds" << "\n";
    return 0;
}

