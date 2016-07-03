#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

// Default Network Topology
//                  server
//       10.1.1.0     |
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      WIFI 10.1.3.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Third_project_2_ScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 3;

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (nWifi > 250)
    {
      std::cout << "Number of wifi nodes " << nWifi << 
                   " specified exceeds the mobility bounding box" << std::endl;
      exit (1);
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);
  std::cout<<"p2p create successfully"<<std::endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);
  std::cout<<"wifi create successfully"<<std::endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  //STA move as constant velocity.
  mobility.SetMobilityModel ("ns3::ConstantAccelerationMobilityModel");
  mobility.Install (wifiStaNodes);
  for(uint n=0;n<wifiStaNodes.GetN();n++)
{
   Ptr<ConstantAccelerationMobilityModel> mob=wifiStaNodes.Get(n)->GetObject<ConstantAccelerationMobilityModel>();
   mob->SetVelocityAndAcceleration(Vector(20,0,0),Vector(1,0,0));
}

  //AP don't move.
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  std::cout<<"mobility set successfully"<<std::endl;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  InternetStackHelper stack;
  stack.Install (p2pNodes);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);
  std::cout<<"ip_Netstack set successfully"<<std::endl;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (p2pNodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  std::cout<<"server set successfully"<<std::endl;

//****************************************************************************
  UdpEchoClientHelper echoClient ( p2pInterfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
    echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  ApplicationContainer clientApps2 = 
    echoClient.Install (wifiStaNodes.Get (nWifi - 2));
  clientApps2.Start (Seconds (4.0));
  clientApps2.Stop (Seconds (10.0));


ApplicationContainer clientApps3 = 
    echoClient.Install (p2pNodes.Get (0));
  clientApps3.Start (Seconds (6.0));
  clientApps3.Stop (Seconds (10.0));

ApplicationContainer clientApps4 = 
    echoClient.Install (wifiStaNodes.Get (0));
  clientApps4.Start (Seconds (5.0));
  clientApps4.Stop (Seconds (10.0));

//////////////////////////////////////////////////////////////////////////////////////////////////////////

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  pointToPoint.EnablePcapAll ("second");
  phy.EnablePcap ("second", apDevices.Get (0));
  phy.EnablePcap ("second", staDevices.Get (nWifi-1));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
