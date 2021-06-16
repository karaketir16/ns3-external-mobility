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

std::vector<SystemMutex*> ExternalMobilityModel::m_mutexes;

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

    // Create a mutex for this node and set index
    static int idx = 0;
    m_mutexes.push_back(new SystemMutex()); // make sure deleted
    index = idx++;

    kill_t.clear();

    // Create and start udp thread
    st3 = new SystemThread (
        MakeCallback (&ExternalMobilityModel::UdpServerThread, this));
    st3->Start();
}

void ExternalMobilityModel::UdpServerThread () {
    socklen_t len;
    int n;
    while (true) {

        len = sizeof(cliaddr);  //len is value/resuslt

        fd_set set;
        struct timeval timeout;


        /* Initialize the file descriptor set. */
        FD_ZERO (&set);
        FD_SET (sockfd, &set);

        /* Initialize the timeout data structure. */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0000;

        /* select returns 0 if timeout, 1 if input available, -1 if error. */
        int res =  (select (FD_SETSIZE,
                                           &set, NULL, NULL,
                                           &timeout));

        if(res == 1){
            n = recvfrom(sockfd, (char *)protocol.buffer, MAX_PKG_LEN,
                        MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                        &len);
            if(n < 0){
                std::cout << "Read Val: " << n << std::endl << std::flush;
                NS_ASSERT_MSG(false, "what is it?");
            }

            std::cout << "Received from: " << inet_ntoa(cliaddr.sin_addr) <<":" << (int) ntohs(cliaddr.sin_port) <<" _ myport: " << (int) ntohs(servaddr.sin_port) << std::endl;
    //        sendto(sockfd, (const char *)protocol.buffer, n,
    //            0, (const struct sockaddr *) &cliaddr,
    //                len);
    //        std::cout << "Echo message sent." << std::endl;

            uint8_t tst;
            if( (tst = protocol.decode(n)) == PackageType::Position){
                auto temp = m_position;
                temp.x = protocol.x;
                temp.y = protocol.y;
                temp.z = protocol.z;
                DoSetPosition(temp);
            } else if (tst == PackageType::Kill_Thread){
                std::cout << "Received Kill " << this <<" "<<index << std::endl << std::flush;
                break;
            } else {
                NS_ASSERT_MSG(false, "NOT POSSIBLE");
            }

        } else if(res == 0){
            std::cout << "TimeOUT" << std::endl << std::flush;
            if(kill_t.test()){
                break;
            }
        } else {
            std::cout << "-----Err---- : " << res << std::endl << std::flush;
//            NS_ASSERT_MSG(false, "TEST");
        }
    }
}

ExternalMobilityModel::~ExternalMobilityModel ()
{
    m_mutexes[index]->Lock();

//    Protocol kill_t;
//    sendto(sockfd, (void *)kill_t.buffer, kill_t.encode(PackageType::Kill_Thread),
//        0, (const struct sockaddr *) &servaddr,
//           sizeof (servaddr));



    kill_t.test_and_set();
    st3->Join();

    // give memory back
//    delete m_mutexes[index];
}


Vector
ExternalMobilityModel::DoGetPosition (void) const
{
    std::cout << this <<" getpos " << Simulator::Now().GetSeconds() << std::endl;

    m_mutexes[index]->Lock();
    auto temp = m_position;
    m_mutexes[index]->Unlock();

    return temp;
}

void 
ExternalMobilityModel::DoSetPosition (const Vector &position)
{
    m_mutexes[index]->Lock();
    m_position = position;
    m_mutexes[index]->Unlock();

    NotifyCourseChange ();
}

Vector
ExternalMobilityModel::DoGetVelocity (void) const
{
    return Vector(0.0, 0.0, 0.0);
}

} // namespace ns3
