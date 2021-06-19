/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2007 INRIA
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
#include "external-mobility-model.h"
#include "ns3/simulator.h"

#include "protocol.h"

#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ExternalMobilityModel);

TypeId ExternalMobilityModel::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ExternalMobilityModel")
            .SetParent<MobilityModel> ()
            .SetGroupName ("Mobility")
            .AddConstructor<ExternalMobilityModel> ();
    return tid;
}

void
ExternalMobilityModel::thread_safe_stop(){
    kill_thread_flag.test_and_set();
    st3->Join();
    std::cout <<"Rest in peace " << st3 << std::endl;
    delete st3;
}

ExternalMobilityModel::ExternalMobilityModel ()
{
    static unsigned PORT = NS3_NODE_PORT_BASE;

    udp_port = PORT++;


    // Create and start udp thread
    st3 = new SystemThread (
                MakeCallback (&ExternalMobilityModel::UdpServerThread, this));

    Simulator::ScheduleDestroy(&ExternalMobilityModel::thread_safe_stop, this);
    st3->Start();
}

void ExternalMobilityModel::UdpServerThread () {

    udp_sock = new UDPSocketHelper();

    //SOMEHOE NS_ASSERT_MSG(func(), "msg") not works
    if( ! udp_sock->Create()){
        NS_ASSERT_MSG(false, "PORT create fail");
    }

    if( ! udp_sock->Bind(udp_port)) {
        NS_ASSERT_MSG(false, "PORT bind fail");
    }

    while (true) {
        if(kill_thread_flag.test()){
            break;
        }
        int n = udp_sock->RecvNotBlock((char *)protocol.buffer, 500);

        if(n > 0){
            if( protocol.decode(n)== PackageType::Position){

                Simulator::Schedule(MilliSeconds(20),&ExternalMobilityModel::SetPosition, this, Vector(protocol.x, protocol.y, protocol.z));

                //This is probably more accurate, but somehow breaks the program sometimes.
                //Simulator::ScheduleNow(&ExternalMobilityModel::SetPosition, this, Vector(protocol.x, protocol.y, protocol.z));
            } else {
                NS_ASSERT_MSG(false, "Wrong Package Type Received");
            }
        } else {

        }
    }
}

ExternalMobilityModel::~ExternalMobilityModel ()
{
}


Vector
ExternalMobilityModel::DoGetPosition (void) const
{
    return m_position;
}

void 
ExternalMobilityModel::DoSetPosition (const Vector &position)
{
    m_position = position;
    NotifyCourseChange ();
}

Vector
ExternalMobilityModel::DoGetVelocity (void) const
{
    return Vector(0.0, 0.0, 0.0);
}

} // namespace ns3
