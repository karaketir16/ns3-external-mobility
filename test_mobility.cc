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

#include "simulation-control.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("ExternalMobiltyTest");

uint64_t timeSinceEpochMillisec() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t start_time;
uint64_t receiveCountTotal = 0;

void test(){
    std::cout   << "Simulator Time: "<< Simulator::Now().GetMilliSeconds()
                << " Differance Between Real-Simulation Time(ms): "
                << timeSinceEpochMillisec() - Simulator::Now().GetMilliSeconds()  - start_time << std::endl;

    static uint64_t lastCount = 0;
    std::cout << "Receive Count(from last): " << receiveCountTotal - lastCount << std::endl;
    lastCount = receiveCountTotal;

    Simulator::Schedule (MilliSeconds(100), &test);
}



void ReceivePacket (Ptr<Socket> socket)
{
    while (socket->Recv ())
    {
        receiveCountTotal++;
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, Time pktInterval )
{
    socket->Send (Create<Packet> (pktSize));
    Simulator::Schedule (pktInterval, &GenerateTraffic,
                         socket, pktSize, pktInterval);

}

int main (int argc, char *argv[])
{
    std::string phyMode ("DsssRate11Mbps");
    std::string saveFile ("DisabledFeedback");
    uint32_t packetSize = 100; // bytes
    uint32_t interval = 50; // milliseconds
    bool feedback = false;
    double txp = 7.5; // dbm
    double simTime = 20; // simulation time

    CommandLine cmd (__FILE__);
    cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
    cmd.AddValue ("saveFile", "Save File name", saveFile);
    cmd.AddValue ("txp", "transmit power dBm", txp);
    cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
    cmd.AddValue ("interval", "interval (milliseconds) between packets", interval);
    cmd.AddValue ("feedback", "enable feedback", feedback);
    cmd.AddValue ("simTime", "simulation time(seconds)", simTime);

    cmd.Parse (argc, argv);
    Time intervalTime = MilliSeconds(interval);

    GlobalValue::Bind ("SimulatorImplementationType",
                       StringValue ("ns3::RealtimeSimulatorImpl"));

    int number_of_nodes = 12;

    NodeContainer server;
    server.Create(1);

    NodeContainer clients;
    clients.Create (number_of_nodes - 1);

    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211b);

    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::AdhocWifiMac");
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (phyMode),
                                  "ControlMode",StringValue (phyMode));



    // PHY
    YansWifiPhyHelper wifiPhy;

    wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
    wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

    // Channel
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel (wifiChannel.Create ());


    NetDeviceContainer sDevice = wifi.Install (wifiPhy, wifiMac, server);
    NetDeviceContainer cDevices = wifi.Install (wifiPhy, wifiMac, clients);


    DsdvHelper routing;
    routing.Set("PeriodicUpdateInterval", TimeValue(MilliSeconds(250)));

    InternetStackHelper internet;
    internet.SetRoutingHelper (routing);
    internet.Install(server);
    internet.Install (clients);


    //
    // Assign IPv4 addresses
    //
    Ipv4AddressHelper ipAddrs;
    ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");

    Ipv4InterfaceContainer sInterface;
    sInterface=ipAddrs.Assign (sDevice);

    Ipv4InterfaceContainer cInterfaces;
    cInterfaces=ipAddrs.Assign (cDevices);

    Config::Set("/NodeList/*/$ns3::Ipv4L3Protocol/InterfaceList/*/ArpCache/DeadTimeout", TimeValue(MilliSeconds(100)));
    Config::Set("/NodeList/*/$ns3::Ipv4L3Protocol/InterfaceList/*/ArpCache/WaitReplyTimeout", TimeValue(MilliSeconds(10)));

    //Mobility Model External
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ExternalMobilityModel");

    mobility.Install(server);
    mobility.Install (clients);

    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    Ptr<Socket> recvSink = Socket::CreateSocket (server.Get (0), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 9);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

    std::vector<Ptr<Socket>> sockets;

    for(int i = 0; i < number_of_nodes - 1; i++){
        Ptr<Socket> source = Socket::CreateSocket (clients.Get (i), tid);
        sockets.push_back(source);
        InetSocketAddress remote = InetSocketAddress(sInterface.GetAddress(0), 9);
        source->Connect (remote);
        Simulator::Schedule(Seconds (1.0), &GenerateTraffic,
                            source, packetSize, intervalTime);
    }

    wifiPhy.EnablePcap(saveFile, server);

    AnimationInterface anim(saveFile + ".xml");
    anim.SetMaxPktsPerTraceFile(99999999999999);

    Simulator::Schedule(MilliSeconds(100), &test);

    SimulationControl control(&receiveCountTotal, feedback);

    Simulator::Stop (Seconds (simTime));
    start_time = timeSinceEpochMillisec();
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
