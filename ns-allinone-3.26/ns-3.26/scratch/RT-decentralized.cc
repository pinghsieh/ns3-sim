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
// Default Network Topology
// 
// AP
// *      *
// n0     n1
//


using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("RT-decentralized");


int
main (int argc, char *argv[])
{
    bool verbose = true;
    uint32_t nRT = 2;
    uint32_t nAP = 1;
    bool tracing = true;

    CommandLine cmd;
    cmd.AddValue ("verbose", "Tell echo application to log if true", verbose);
    cmd.AddValue ("nRT", "Number of real-time distributed WiFi devices, excluding AP", nRT);

    cmd.Parse (argc, argv);

    if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    // Create nRT station node objects
    NodeContainer wifiRTStaNodes;
    wifiRTStaNodes.Create (nRT); 

    // Create 1 AP node objects
    NodeContainer wifiRTApNodes;
    wifiRTApNodes.Create (nAP);

    // Create a channel helper and phy helper, and then create the channel
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create());


    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
    wifi.SetStandard (WIFI_PHY_STANDARD_80211a);

    NetDeviceContainer wifiStaDevices;
    NetDeviceContainer wifiApDevices;

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
    wifiStaDevices = wifi.Install (phy, mac, wifiRTStaNodes);

    mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
    wifiApDevices = wifi.Install (phy, mac, wifiRTApNodes);

    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (5.0),
                                   "DeltaY", DoubleValue (10.0),
                                   "GridWidth", UintegerValue (3),
                                   "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (wifiRTStaNodes);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiRTApNodes);

    InternetStackHelper stack;
    stack.Install (wifiRTStaNodes);
    stack.Install (wifiRTApNodes);

    Ipv4InterfaceContainer wifiApInterfaces;
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    address.Assign (wifiStaDevices);
    wifiApInterfaces = address.Assign (wifiApDevices);

    // UDP Server
    UdpEchoServerHelper echoServer (9);

    ApplicationContainer serverApps = echoServer.Install (wifiRTApNodes.Get(0));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    // UDP Client
    UdpEchoClientHelper echoClient (wifiApInterfaces.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (wifiRTStaNodes.Get(0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Simulator events
    Simulator::Stop(Seconds (10.0));

    if (tracing)
    {
        phy.EnablePcap ("RT-decentralized", wifiStaDevices.Get(0));
        phy.EnablePcap ("RT-decentralized", wifiApDevices.Get(0));
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}














 



