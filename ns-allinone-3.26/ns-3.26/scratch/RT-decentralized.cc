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


#include <iostream>
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
FlushMacQueue  (Ptr<WifiNetDevice> netdev)
{
	Ptr<AdhocWifiMac> mac_source = netdev->GetMac()->GetObject<AdhocWifiMac>();
	Ptr<DcaTxop> dca = mac_source->GetDcaTxop();
	Ptr<WifiMacQueue> m_queue = dca->GetQueue();
	/* Flush the expired packets */
	//m_queue->Flush();

	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Queue size = " << m_queue->GetSize());
	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Is empty? " << m_queue->IsEmpty());
}

void
SetDeterministicBackoffNow (Ptr<WifiNetDevice> netdev, uint32_t backoff)
{
	Ptr<AdhocWifiMac> mac_source = netdev->GetMac()->GetObject<AdhocWifiMac>();
	Ptr<DcaTxop> dca = mac_source->GetDcaTxop();
	dca->SetDeterministicBackoff(backoff);
}

void
StartNewInterval (Ptr<WifiNetDevice> netdev, Ptr<AdhocWifiMac> mac_dest, uint32_t pktSize, uint32_t pktCount, double qn)
{
	Ptr<AdhocWifiMac> mac_source = netdev->GetMac()->GetObject<AdhocWifiMac>();
	Ptr<DcaTxop> dca = mac_source->GetDcaTxop();
	Ptr<WifiMacQueue> m_queue = dca->GetQueue();

	/* Cancel on-going transmissions or should we check timing before sending?*/

	/* Flush the expired packets */
	//m_queue->Flush();

	/* Reset backoff timer */
	uint32_t backoff = 37;
	dca->SetDeterministicBackoff(backoff);

	/* Get packet arrivals */
	for (uint32_t i = 0; i < pktCount; i++){
		mac_source->Enqueue(Create<Packet> (pktSize), mac_dest->GetAddress());
		//netdev->Send(Create<Packet> (pktSize), mac_dest->GetAddress(), uint16_t(0));
		/* Update delivery debt*/
		dca->UpdateDeliveryDebt (qn);
	}

	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Queue size = " << m_queue->GetSize());
	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Is empty? " << m_queue->IsEmpty());
	NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Delivery Debt =  " << dca->GetDeliveryDebt());


}

void
ConfigRTdecentralized (Ptr<WifiNetDevice> netdev, double pn)
{
	Ptr<AdhocWifiMac> mac_source = netdev->GetMac()->GetObject<AdhocWifiMac>();
	Ptr<DcaTxop> dca = mac_source->GetDcaTxop();
	dca->SetRTdecentralized(true);
	dca->SetChannelPn(pn);
}

void
ReceivePacket (Ptr<Socket> socket)
{
	if (socket->Recv()) {
		NS_LOG_UNCOND ("At " << Simulator::Now().GetSeconds() << ": Received one packet! \n");
	}
}

/*
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount)
{
	if (pktCount > 0){
		for (uint32_t i = 0; i < pktCount; i++){
			socket->Send(Create<Packet> (pktSize));
		}
		//Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize, pktCount-1, pktInterval);
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
    bool verbose = true;
    uint32_t nRT = 2;
    bool tracing = true;
    //double interval = 0.001; // Interval length in seconds
    double packet_interval = 0.001;
    double startT = 2.5;
    uint32_t nIntervals = 20;
    //double endT = startT + nIntervals*packet_interval;
    double stopT = 10.0;
    double offset = 0.00001;
    uint32_t packetSize = 1500;
    uint32_t pktCount = 5;
    double channel_pn[2] = {1, 1}; // for unreliable transmissions

    std::string backoffLog ("RT-backoff.log");

    CommandLine cmd;
    cmd.AddValue ("verbose", "Tell echo application to log if true", verbose);
    cmd.AddValue ("nRT", "Number of real-time distributed WiFi devices, excluding AP", nRT);

    cmd.Parse (argc, argv);

    WifiHelper wifi;

    if (verbose)
    {
    	//LogComponentEnableAll(LOG_PREFIX_NODE);
        //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
        //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
        //LogComponentEnable ("DcfManager", LOG_LEVEL_INFO);
        wifi.EnableLogComponents ();  // Turn on all Wifi logging
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
                                   "DeltaX", DoubleValue (5.0),
                                   "DeltaY", DoubleValue (10.0),
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


    /* Simulator events: configuration */
    for (uint32_t i = 0; i < nRT; i++)
    {
    	Ptr<WifiNetDevice> netdev = wifiStaDevices.Get(i)->GetObject<WifiNetDevice>();
    	Simulator::ScheduleWithContext(i, Seconds(startT - offset),
    	        			&ConfigRTdecentralized, netdev, channel_pn[i]);
    }

    /* Simulator events: packet transmissions */
    for (uint32_t t = 0; t < nIntervals; t++)
    {
        /* Tracing for MAC events */
        for (uint32_t i = 1; i < nRT; i++)
        {
        	Ptr<WifiNetDevice> netdev = wifiStaDevices.Get(i)->GetObject<WifiNetDevice>();
        	//Ptr<AdhocWifiMac> mac_source = wifiStaDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac() -> GetObject<AdhocWifiMac>();
        	Ptr<AdhocWifiMac> mac_dest = wifiStaDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac() -> GetObject<AdhocWifiMac>();
        	Simulator::ScheduleWithContext(i, Seconds(startT + packet_interval*double(t)+offset),
        	        			&FlushMacQueue, netdev);
        	Simulator::ScheduleWithContext(i, Seconds(startT + packet_interval*double(t)+(2.0)*offset),
        			&StartNewInterval, netdev, mac_dest, packetSize, pktCount, double(0.85));

        	//for (uint32_t j = 0; j < pktCount; j++){
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
    Simulator::Destroy();
    return 0;
}














 



