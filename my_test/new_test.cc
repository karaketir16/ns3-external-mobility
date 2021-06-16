#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/netanim-module.h"

using namespace ns3;
using namespace std;
int main_5(int argc, char *argv[])
{
    Packet::EnablePrinting ();
//    int m_nSinks = 10;
    double txp = 7.5;
    std::string m_CSVfileName = "test";

    int nWifis = 50;

    double TotalTime = 200.0;
    std::string rate ("2048bps");
    std::string phyMode ("DsssRate11Mbps");
    std::string tr_name ("manet-routing-compare");
//    int nodeSpeed = 20; //in m/s
//    int nodePause = 0; //in s
    std::string m_protocolName = "protocol";

//    Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("64"));
//    Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));

    //Set Non-unicastMode rate to unicast mode
//    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));

    NodeContainer adhocNodes;
    adhocNodes.Create (nWifis);

    // setting up wifi phy and channel using helpers
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211b);

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel (wifiChannel.Create ());

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (phyMode),
                                  "ControlMode",StringValue (phyMode));

    wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
    wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

    wifiMac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);

    MobilityHelper mobilityAdhoc;
    int64_t streamIndex = 0; // used to get consistent mobility across scenarios

    ObjectFactory pos;
    pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
    pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
    pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));



    Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
    streamIndex += taPositionAlloc->AssignStreams (streamIndex);

    mobilityAdhoc.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Mode", StringValue ("Time"),
                               "Time", StringValue ("2s"),
                               "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=50.0]"),
                               "Bounds", RectangleValue (Rectangle (0.0, 300.0, 0.0, 1500.0)));

    mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
    mobilityAdhoc.Install (adhocNodes);

//    std::stringstream ssSpeed;
//    ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
//    std::stringstream ssPause;
//    ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
//    mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
//                                    "Speed", StringValue (ssSpeed.str ()),
//                                    "Pause", StringValue (ssPause.str ()),
//                                    "PositionAllocator", PointerValue (taPositionAlloc));
//    mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
//    mobilityAdhoc.Install (adhocNodes);
//    streamIndex += mobilityAdhoc.AssignStreams (adhocNodes, streamIndex);
//    NS_UNUSED (streamIndex); // From this point, streamIndex is unused

//    AodvHelper aodv;
//    OlsrHelper olsr;
//    DsdvHelper dsdv;
//    DsrHelper dsr;
//    DsrMainHelper dsrMain;
//    Ipv4ListRoutingHelper list;
//    InternetStackHelper internet;

//    switch (m_protocol)
//      {
//      case 1:
//        list.Add (olsr, 100);
//        m_protocolName = "OLSR";
//        break;
//      case 2:
//        list.Add (aodv, 100);
//        m_protocolName = "AODV";
//        break;
//      case 3:
//        list.Add (dsdv, 100);
//        m_protocolName = "DSDV";
//        break;
//      case 4:
//        m_protocolName = "DSR";
//        break;
//      default:
//        NS_FATAL_ERROR ("No such protocol:" << m_protocol);
//      }

//    if (m_protocol < 4)
//      {
//        internet.SetRoutingHelper (list);
//        internet.Install (adhocNodes);
//      }
//    else if (m_protocol == 4)
//      {
//        internet.Install (adhocNodes);
//        dsrMain.Install (dsr, adhocNodes);
//      }

//    NS_LOG_INFO ("assigning ip address");

//    Ipv4AddressHelper addressAdhoc;
//    addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
//    Ipv4InterfaceContainer adhocInterfaces;
//    adhocInterfaces = addressAdhoc.Assign (adhocDevices);

//    OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address ());
//    onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
//    onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

//    for (int i = 0; i < nSinks; i++)
//      {
//        Ptr<Socket> sink = SetupPacketReceive (adhocInterfaces.GetAddress (i), adhocNodes.Get (i));

//        AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (i), port));
//        onoff1.SetAttribute ("Remote", remoteAddress);

//        Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
//        ApplicationContainer temp = onoff1.Install (adhocNodes.Get (i + nSinks));
//        temp.Start (Seconds (var->GetValue (100.0,101.0)));
//        temp.Stop (Seconds (TotalTime));
//      }

//    std::stringstream ss;
//    ss << nWifis;
//    std::string nodes = ss.str ();

//    std::stringstream ss2;
//    ss2 << nodeSpeed;
//    std::string sNodeSpeed = ss2.str ();

//    std::stringstream ss3;
//    ss3 << nodePause;
//    std::string sNodePause = ss3.str ();

//    std::stringstream ss4;
//    ss4 << rate;
//    std::string sRate = ss4.str ();

    //NS_LOG_INFO ("Configure Tracing.");
    //tr_name = tr_name + "_" + m_protocolName +"_" + nodes + "nodes_" + sNodeSpeed + "speed_" + sNodePause + "pause_" + sRate + "rate";

    //AsciiTraceHelper ascii;
    //Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (tr_name + ".tr").c_str());
    //wifiPhy.EnableAsciiAll (osw);
//    AsciiTraceHelper ascii;
//    MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name + ".mob"));

    //Ptr<FlowMonitor> flowmon;
    //FlowMonitorHelper flowmonHelper;
    //flowmon = flowmonHelper.InstallAll ();


//    NS_LOG_INFO ("Run Simulation.");

//    CheckThroughput ();

    Simulator::Stop (Seconds (TotalTime));
    Simulator::Run ();

    AnimationInterface anim ("animation_3.xml");

    //flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), false, false);

    Simulator::Destroy ();
    return 0;
}
