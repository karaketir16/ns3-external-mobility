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
ExternalMobilityModel::KILL_ME(){
    kill_t.test_and_set();
    st3->Join();
    std::cout <<"Rest in peace " << st3 << std::endl;
    delete st3;
}

ExternalMobilityModel::ExternalMobilityModel ()
{
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;

    static unsigned PORT = 5570;
    servaddr.sin_port = htons(PORT++);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    // Create and start udp thread
    st3 = new SystemThread (
                MakeCallback (&ExternalMobilityModel::UdpServerThread, this));

    Simulator::ScheduleDestroy(&ExternalMobilityModel::KILL_ME, this);
    st3->Start();
}

void ExternalMobilityModel::UdpServerThread () {
    socklen_t len;
    int n;
    while (true) {
        if(kill_t.test()){
            break;
        }

        fd_set set;
        struct timeval timeout;


        /* Initialize the file descriptor set. */
        FD_ZERO (&set);
        FD_SET (sockfd, &set);

        /* Initialize the timeout data structure. */
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000 * 500;

        /* select returns 0 if timeout, 1 if input available, -1 if error. */
        int res =  (select (FD_SETSIZE,
                            &set, NULL, NULL,
                            &timeout));

        if(res == 1){

            len = sizeof(cliaddr);  //len is value/resuslt

            n = recvfrom(sockfd, (char *)protocol.buffer, MAX_PKG_LEN,
                         MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                         &len);
            if(n < 0){
                std::cout << "Read Val: " << n << std::endl << std::flush;
                NS_ASSERT_MSG(false, "what is it?");
            }

            uint8_t tst;
            if( (tst = protocol.decode(n)) == PackageType::Position){

                Simulator::Schedule(MilliSeconds(10),&ExternalMobilityModel::SetPosition, this, Vector(protocol.x, protocol.y, protocol.z));
                //            Simulator::ScheduleNow(&ExternalMobilityModel::SetPosition, this, Vector(protocol.x, protocol.y, protocol.z));
            } else {
                NS_ASSERT_MSG(false, "NOT POSSIBLE");
            }
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
