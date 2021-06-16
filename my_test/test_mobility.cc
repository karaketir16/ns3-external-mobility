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

void handler (int &arg0, int &arg1, NodeContainer &nodes)
{
    std::cout << "handler called with argument arg0=" << arg0 << " and\
        arg1=" << arg1 << std::endl;

  for (NodeContainer::Iterator j = nodes.Begin ();
       j != nodes.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Vector pos = position->GetPosition ();
      position->SetPosition(Vector(1,2,3));
      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }

Simulator::Schedule(Seconds(1), &handler, 10, 5, nodes);
}



int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  GlobalValue::Bind ("SimulatorImplementationType",
                     StringValue ("ns3::RealtimeSimulatorImpl"));
  
  NodeContainer sta;
  sta.Create (10);
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

//  Simulator::Schedule(Seconds(1), &handler, 10, 5, sta);

  Simulator::Stop (Seconds (4.0));
  AnimationInterface anim ("animation_12.xml");
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}