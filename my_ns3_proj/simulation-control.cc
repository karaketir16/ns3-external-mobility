#include "simulation-control.h"

using namespace ns3;

SimulationControl::SimulationControl(uint64_t *watch, uint16_t port_src, uint16_t port_dest)
{
    udp_socket = new UDPSocketHelper();
    udp_socket->Create();
    udp_socket->Bind(port_src);
    udp_socket->SetDest(port_dest);

    Protocol p;

    int n;

    back:
    while( ( n = udp_socket->RecvNotBlock(p.buffer, 500)) < 0){
        p.state = StateType::Waiting;
        p.totalReceivedData = 0;
        udp_socket->Send(p.buffer, p.encode(PackageType::Information));
        std::cout << "Waiting For Start Message" << std::endl;
    }
    if( p.decode(n) != (uint8_t) PackageType::Start){
        goto back;
    }

    Simulator::Schedule(MilliSeconds(1000), &SimulationControl::sendInfo, this, watch);
    Simulator::ScheduleDestroy(&SimulationControl::sendEnd, this);
}

void SimulationControl::sendInfo(uint64_t *watch)
{
    Protocol p;
    p.totalReceivedData = *watch;
    p.state = StateType::Running;
    udp_socket->Send(p.buffer, p.encode(PackageType::Information));
    Simulator::Schedule(MilliSeconds(500), &SimulationControl::sendInfo, this, watch);
}

void SimulationControl::sendEnd()
{
    Protocol p;
    p.state = StateType::End;
    udp_socket->Send(p.buffer, p.encode(PackageType::Information));
}
