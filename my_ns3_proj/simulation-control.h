#ifndef FEEDBACK_H
#define FEEDBACK_H

#include "ns3/simulator.h"
#include "udpsockethelper.h"
#include "protocol.h"

namespace ns3{
    class SimulationControl
    {
    public:
        SimulationControl(uint64_t *watch, bool feedback = true, uint16_t port_src = NS3_SERVER_PORT, uint16_t port_dest = CONTROLLER_PORT);
        void sendInfo(uint64_t *watch);
        void sendEnd();
    private:
        UDPSocketHelper *udp_socket;
    };
}


#endif // FEEDBACK_H
