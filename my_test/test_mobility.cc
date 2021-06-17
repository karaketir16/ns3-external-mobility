/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of Washington
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
 */

// Test that mobility tracing works

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"

#include "ns3/netanim-module.h"

#include "external-mobility-model.h"

using namespace ns3;

uint64_t timeSinceEpochMillisec() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t start;

void test(){
    std::cout   << "Simulator Time: "<< Simulator::Now().GetMilliSeconds()
                << " Differance Between Real-Simulation Time(ms): "
                << timeSinceEpochMillisec() - Simulator::Now().GetMilliSeconds()  - start << std::endl;

    Simulator::Schedule (MilliSeconds(100), &test);
}

#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"

#include "ns3/aodv-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/dsdv-helper.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Mob");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  GlobalValue::Bind ("SimulatorImplementationType",
                     StringValue ("ns3::RealtimeSimulatorImpl"));

  int number_of_nodes = 12;

  NodeContainer c;
  c.Create (number_of_nodes); //20 wireless nodes

 WifiHelper wifi;
//wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
wifi.SetStandard (WIFI_STANDARD_80211b);
//80211a, 80211b, 80211n, 2.4g and 5G, 80211ac, 80211ax is also supported.80211p (VANETs, WAVE)



//  WifiMacHelper mac;
//  mac.SetType ("ns3::AdhocWifiMac");
//  //AdhocWifiMac, StaWifiMac, ApWifiMac
//  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
//                                "DataMode", StringValue ("OfdmRate54Mbps"));
////                                "DataMode", StringValue ("DsssRate54Mbps"));

 std::string phyMode = "DsssRate11Mbps";
 double txp = 7.5; // dbm

 // Add a mac and disable rate control
 WifiMacHelper wifiMac;
 wifiMac.SetType ("ns3::AdhocWifiMac");
 wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                               "DataMode",StringValue (phyMode),
                               "ControlMode",StringValue (phyMode));



  /** Wifi PHY **/
  /***************************************************************************/
  YansWifiPhyHelper wifiPhy;

 wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
 wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

  /** wifi channel **/
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  NetDeviceContainer cDevices = wifi.Install (wifiPhy, wifiMac, c);
 //
  NS_LOG_INFO ("Enabling AODV routing on all backbone nodes");
//  AodvHelper routing;
//  routing.Set("ActiveRouteTimeout", TimeValue(MilliSeconds(100)));
  //AODV protocol is being using FANETs.

//  OlsrHelper routing;
  DsdvHelper routing;
    routing.Set("PeriodicUpdateInterval", TimeValue(MilliSeconds(500)));

  InternetStackHelper internet;
  internet.SetRoutingHelper (routing); // has effect on the next Install ()
  internet.Install (c);

  //
  // Assign IPv4 addresses to the device drivers (actually to the associated
  // IPv4 interfaces) we just created.
  //
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");
  Ipv4InterfaceContainer cInterfaces;
  cInterfaces=ipAddrs.Assign (cDevices);

  Config::Set("/NodeList/*/$ns3::Ipv4L3Protocol/InterfaceList/*/ArpCache/DeadTimeout", TimeValue(MilliSeconds(100)));
  Config::Set("/NodeList/*/$ns3::Ipv4L3Protocol/InterfaceList/*/ArpCache/WaitReplyTimeout", TimeValue(MilliSeconds(10)));


 /*
//Mobility Model - 2D
MobilityHelper mobility;

mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
  "MinX", DoubleValue (0.0),
  "MinY", DoubleValue (0.0),
  "DeltaX", DoubleValue (5.0),
  "DeltaY", DoubleValue (10.0),
  "GridWidth", UintegerValue (3),
  "LayoutType", StringValue ("RowFirst"));

mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",  "Bounds", RectangleValue (Rectangle (-100, 100, -100, 100)));
mobility.Install (c);
 */

 //Mobility Model -3D
//    MobilityHelper mobility;
//    mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
//    "Bounds", BoxValue (Box (0, 100, 0, 100, 0, 100)),
//    "TimeStep", TimeValue (Seconds (0.5)),
//    "Alpha", DoubleValue (0.85),
//    "MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=800|Max=1200]"),
//    "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
//    "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
//    "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
//    "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
//    "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));

  //Mobility Model External
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ExternalMobilityModel");

mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
  "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=100]"),
  "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=100]"),
  "Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=100]"));
mobility.Install (c);

 UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (c.Get(0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (20.0));

 UdpEchoClientHelper echoClient (cInterfaces.GetAddress(0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
  echoClient.SetAttribute ("Interval", TimeValue (MilliSeconds (100)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (100));

  ApplicationContainer clientApps[number_of_nodes - 1];
  for(int i = 0; i < number_of_nodes - 1; i++){
      clientApps[i] = echoClient.Install (c.Get(i + 1));
      clientApps[i].Start (Seconds (2.0));
      clientApps[i].Stop (Seconds (20.0));
  }

//wifiPhy.EnablePcapAll ("Fanet3DExternal"); //Packet Capture.
wifiPhy.EnablePcapAll ("Fanet3DTest2"); //Packet Capture.
//Network Animation using NetAnim.
//AnimationInterface anim("Fanet3DExternal.xml");
AnimationInterface anim("Fanet3DTest2.xml");
anim.SetMaxPktsPerTraceFile(99999999999999);
//Ascii Trace Metrics can be processed using Tracemetrics Software.
//AsciiTraceHelper ascii;
//wifiPhy.EnableAsciiAll(ascii.CreateFileStream("Fanet3DExternal.tr"));
//wifiPhy.EnableAsciiAll(ascii.CreateFileStream("Fanet3DTest.tr"));

Simulator::Schedule(MilliSeconds(100), &test);

  Simulator::Stop (Seconds (20.0));
  start = timeSinceEpochMillisec();
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


int main_first (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  GlobalValue::Bind ("SimulatorImplementationType",
                     StringValue ("ns3::RealtimeSimulatorImpl"));
  
  NodeContainer sta;
  sta.Create (50);
  MobilityHelper mobility;
//  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
//                                 "MinX", DoubleValue (1.0),
//                                 "MinY", DoubleValue (1.0),
//                                 "DeltaX", DoubleValue (5.0),
//                                 "DeltaY", DoubleValue (5.0),
//                                 "GridWidth", UintegerValue (3),
//                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
  "X", StringValue
    ("ns3::UniformRandomVariable[Min=0.0|Max=20.0]"),
  "Y", StringValue
    ("ns3::UniformRandomVariable[Min=0.0|Max=20.0]"));

//  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
//                             "Mode", StringValue ("Time"),
//                             "Time", StringValue ("2s"),
//                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
//                             "Bounds", RectangleValue (Rectangle (0.0, 20.0, 0.0, 20.0)));


    mobility.SetMobilityModel ("ns3::ExternalMobilityModel");

//    mobility.

//  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (sta);
  // Set mobility random number streams to fixed values
  mobility.AssignStreams (sta, 0);
  for (NodeContainer::Iterator j = sta.Begin ();
       j != sta.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<ExternalMobilityModel> position = DynamicCast<ExternalMobilityModel>(object->GetObject<MobilityModel> ());
      NS_ASSERT (position != 0);
      Vector pos = position->GetPosition ();
//      position->SetPosition(Vector(1000,1000,0));
//      Vector vel = position->GetVelocity();
//      position->SetVelocity(Vector(10,10,0));
      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
//      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }


  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("mobility-trace-example.mob"));

  Simulator::Schedule(MilliSeconds(100), &test);

  Simulator::Stop (Seconds (100.0));
  AnimationInterface anim ("animation_13.xml");
  start = timeSinceEpochMillisec();
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
