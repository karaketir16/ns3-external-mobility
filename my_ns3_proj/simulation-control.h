#ifndef FEEDBACK_H
#define FEEDBACK_H

#include "ns3/simulator.h"
#include "udpsockethelper.h"
#include "protocol.h"

namespace ns3{
    class SimulationControl
    {
    public:
        SimulationControl(uint64_t *watch, uint16_t port_src = 9090, uint16_t port_dest = 9999);
        void sendInfo(uint64_t *watch);
    private:
        UDPSocketHelper *udp_socket;
    };
}


#endif // FEEDBACK_H
