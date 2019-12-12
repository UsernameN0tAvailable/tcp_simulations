#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/packet-sink.h"
#include "ns3/simulator.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traffic-control-module.h"
#include "ns3/node.h"
#include "ns3/enum.h"
#include "ns3/config-store-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpBulkSend");

Ipv4AddressHelper ipv4;

/**
 * Assigns IP address to nodes
 *
 * @param a the second byte of the Ip adress
 * @param ndc NetDeviceContainer (Node) to which the
 * IP address is assigned
 * @return IP address interface
 */

Ipv4InterfaceContainer assignIp(int a, NetDeviceContainer ndc) {

    std::stringstream ss;
    ss << "10.1." << std::to_string(a) << ".0";
    std::string s = ss.str();

    char *tab2 = new char[s.length() + 1];
    strcpy(tab2, s.c_str());
    ipv4.SetBase(tab2, "255.255.255.0");

    return ipv4.Assign(ndc);
}

/**
 * Stops the OFF phase and
 * Starts the ON phase of the
 * ON/OFF Error Model
 * on a connection between 2 nodes
 *
 * @param e1 node where the connection starts
 * @param e2 node where the connection ends
 * @param ber Bit Error Ratio to apply
 */
void onPhase(int e1, int e2, double ber) {
    // set error unit
    std::stringstream ss1;
    ss1 << "/NodeList/" << std::to_string(e1) << "/$ns3::Node/DeviceList/" << std::to_string(e2)
        << "/$ns3::PointToPointNetDevice/ReceiveErrorModel/$ns3::RateErrorModel/ErrorUnit";
    Config::Set(ss1.str(), EnumValue(RateErrorModel::ERROR_UNIT_BIT));

    // set bit error ratio
    std::stringstream ss2;
    ss2 << "/NodeList/" << std::to_string(e1) << "/$ns3::Node/DeviceList/" << std::to_string(e2)
        << "/$ns3::PointToPointNetDevice/ReceiveErrorModel/$ns3::RateErrorModel/ErrorRate";
    Config::Set(ss2.str(), DoubleValue(ber));
}

/**
 *
 * Stops the On phase and starts the Off phase
 * of the ON/OFF Error Model on a connection
 * between 2 nodes
 *
 * @param e1 node where the connection starts
 * @param e2 node where the connection ends
 * @param per Packet Error Ratio to apply
 */
void offPhase(int e1, int e2, double per) {
    // set error unit
    std::stringstream ss1;
    ss1 << "/NodeList/" << std::to_string(e1) << "/$ns3::Node/DeviceList/" << std::to_string(e2)
        << "/$ns3::PointToPointNetDevice/ReceiveErrorModel/$ns3::RateErrorModel/ErrorUnit";
    Config::Set(ss1.str(), EnumValue(RateErrorModel::ERROR_UNIT_PACKET));

    // set packet error ratio
    std::stringstream ss2;
    ss2 << "/NodeList/" << std::to_string(e1) << "/$ns3::Node/DeviceList/" << std::to_string(e2)
        << "/$ns3::PointToPointNetDevice/ReceiveErrorModel/$ns3::RateErrorModel/ErrorRate";
    Config::Set(ss2.str(), DoubleValue(per));
}

/**
 *
 * Create ON/OFF sequence for ON/OFF Error Model
 * and for ON/OFF plus BER Error Model using
 * the NS3 Simulator::Schedule instance
 *
 * @param e1 node where the connection starts
 * @param e2 node where the connection ends
 * @param stopSeconds time at which the simulation ends
 * @param onRV ON phase duration distribution
 * @param offRV OFF phase duration distribution
 * @param ber Bit Error Ratio to apply to connection
 * @param per Packet Error Ratio to apply to connection
 */
void
createSequence(int e1, int e2, double stopSeconds, Ptr <NormalRandomVariable> onRV, Ptr <NormalRandomVariable> offRV, double ber, double per) {

    double sum = 0;

    for (uint32_t i = 0; sum < stopSeconds; i++) {

        //Turn ON
        sum += onRV->GetValue();
        Simulator::Schedule(Seconds(sum), &offPhase, e1, e2, per);
        //Turn OFF
        sum += offRV->GetValue();
        Simulator::Schedule(Seconds(sum), &onPhase, e1, e2, ber);
    }
}





int main(int argc, char *argv[]) {

    /*
     * General setups
     */
    // set routing strategy
    Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
    // set TCP socket
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    // set MTU
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1000));

    LogComponentEnable("TcpBulkSend", LOG_LEVEL_ERROR);

    // Network attributes
    uint32_t nSatelliteNodes = 6; // # of satellite nodes
    uint32_t nDishes1 = 4;        // # of dishes on one side
    uint32_t nDishes2 = 4;        // # of dishes on the other side
    uint32_t nRouters1 = 3;       // # of routers on one side
    uint32_t nRouters2 = 3;       // # of routers on other side
    uint32_t nTerminals1 = 5;     // # of terminals on sending end
    uint32_t nTerminals2 = 5;     // # of terminals on receiving end
    uint32_t nTerminals2_2 = nTerminals2;

    // App attributes
    double appStart = 0; // time at which the app starts
    double appStop = 20.0; // time at which the app stops (overridden by command line arguments)

    /*
     * Simulation parameters, ovverriden by command line arguments
     */
    double ber = 0.00001; // Bit Error Ratio for BER Error Model and ON phase
    double per = 0.01;    // Packet Error Ratio for OFF phase
    double meanONDuration = 6; // mean ON phase duration in seconds
    int connPerTerminal = 1;   // TCP connections per terminal

    /*
     * MODE 1 := BER Error Model
     * MODE 2 := ON/OFF Error Model
     * MODE 3 := ON/OFF plus BER Error Model
     */
    int mode = 1;


    // Satellite connection attributes
    std::string satsDelay = "275ms";
    std::string satsDataRate = "20Mbps";

    /*
     * Command line arguments
     */
    CommandLine cmd;
    cmd.AddValue("ber", "Bit Error Rate at Sat Link", ber);
    cmd.AddValue("per", "Packet Error Rate during the ON phase", per);
    cmd.AddValue("duration", "Sim Duration", appStop);
    cmd.AddValue("meanONDuration", "Mean Duration of the ON Mode", meanONDuration);
    cmd.AddValue("mode", "1 -> BER, 2 -> ON/OFF, 3, -> BER + ON/OFF", mode);
    cmd.AddValue("nSF", "TCP sessions per terminal", connPerTerminal);
    cmd.Parse(argc, argv);


    /*
     * Set time at which packet sink stops
     * We give 10 seconds in order to let
     * the packets in flight to arrive
     */
    double sinkStop = appStop + 10.0;


    /**
     *  Channel Settings
     */

    // Satellite to Dishes channel attributes
    std::string upSatDataRate = satsDataRate;
    std::string upSatDelay = satsDelay;
    std::string downSatDataRate = satsDataRate;
    std::string downSatDelay = satsDelay;

    // Router to Dishes channel attributes
    std::string r1d1DataRate = "500Mbps";
    std::string r1d1Delay = "10ms";
    std::string d2r2DataRate = "500Mbps";
    std::string d2r2Delay = "10ms";

    // Terminal to Routers channel attributes
    std::string t1r1DataRate = "500Mbps";
    std::string t1r1Delay = "10ms";
    std::string r2t2DataRate = "500Mbps";
    std::string r2t2Delay = "10ms";

    // TCP socket port
    uint16_t port = 999;

    // Initialize Internet Stack and Routing Protocols
    InternetStackHelper internet;

    // creating routers, source and destination. Installing internet stack
    PointToPointHelper p2p;
    NodeContainer allNodes;

    // Create Terminal Nodes on the left
    NodeContainer terminal1Nodes;
    terminal1Nodes.Create(nTerminals1);
    allNodes.Add(terminal1Nodes);

    //Create Router Nodes on the left
    NodeContainer router1Nodes;
    router1Nodes.Create(nRouters1);
    allNodes.Add(router1Nodes);

    // Create Dishes on the left
    NodeContainer dishes1Nodes;
    dishes1Nodes.Create(nDishes1);
    allNodes.Add(dishes1Nodes);

    // Create Satellite Nodes
    NodeContainer satNodes;
    satNodes.Create(nSatelliteNodes);
    allNodes.Add(satNodes);

    // Create Dishes on the right
    NodeContainer dishes2Nodes;
    dishes2Nodes.Create(nDishes2);
    allNodes.Add(dishes2Nodes);

    // Create Router Nodes on the right
    NodeContainer router2Nodes;
    router2Nodes.Create(nRouters2);
    allNodes.Add(router2Nodes);

    // Create Terminal Nodes on the right
    NodeContainer terminal2Nodes;
    terminal2Nodes.Create(nTerminals2);
    allNodes.Add(terminal2Nodes);


    /*
     * If ON/OFF Error Model we set the Bit Error Ratio to be zero
     * for the ON mode
     */
    if(mode == 2){
        ber = 0;
    }

    /*
     * Create an ON/OFF sequence, apply and schedule the NS3 Error Models
     */
    if (mode != 1) {

        // create distribution for the ON phase duration
        Ptr <NormalRandomVariable> onRV = CreateObject<NormalRandomVariable>();
        onRV->SetAttribute("Mean", DoubleValue(meanONDuration));
        onRV->SetAttribute("Variance", DoubleValue(0.5));

        // Create distribution for the OFF phase duration
        Ptr <NormalRandomVariable> offRV = CreateObject<NormalRandomVariable>();
        offRV->SetAttribute("Mean", DoubleValue(1));
        offRV->SetAttribute("Variance", DoubleValue(0.1));

        /*
         * Apply the Error Models to the indices which correspond
         * to the satellite connections.
         * The indices are simulation specific and can be found
         * in output-attributes.txt
         */
        for (int e1 = 8; e1 < 22; e1++) {
            if (e1 < 12) {
                for (int e2 = 4; e2 < 10; e2++) {
                    createSequence(e1, e2, sinkStop, onRV, offRV, ber, per);
                }
            } else if (e1 < 18) {
                for (int e2 = 1; e2 < 9; e2++) {
                    createSequence(e1, e2, sinkStop, onRV, offRV, ber, per);
                }

            } else {
                for (int e2 = 1; e2 < 7; e2++) {
                    createSequence(e1, e2, sinkStop, onRV, offRV, ber, per);
                }
            }
        }
    }

    // Install internet stack to all nodes
    internet.Install(allNodes);

    /*
     * Connect nodes and assign IP addresses to the nodes
     */

    int connectionsCount = nTerminals1 * nRouters1 + nRouters1 * nDishes1
                           + nDishes1 * nSatelliteNodes + nSatelliteNodes * nDishes2
                           + nDishes2 * nRouters2 + nRouters2 * nTerminals2;

    Ipv4InterfaceContainer allIpv4Interfaces[connectionsCount];
    int count = 1;

    // connect terminals to routers
    for (uint32_t i = 0; i < nTerminals1; i++) {
        for (uint32_t j = 0; j < nRouters1; j++) {
            p2p.SetDeviceAttribute("DataRate", StringValue(t1r1DataRate));
            p2p.SetChannelAttribute("Delay", StringValue(t1r1Delay));
            allIpv4Interfaces[count - 1] = assignIp(count,
                                                    p2p.Install(terminal1Nodes.Get(i), router1Nodes.Get(j)));
            count++;
        }
    }

    // connect routers to dishes
    for (uint32_t i = 0; i < nRouters1; i++) {
        for (uint32_t j = 0; j < nDishes1; j++) {
            p2p.SetDeviceAttribute("DataRate", StringValue(r1d1DataRate));
            p2p.SetChannelAttribute("Delay", StringValue(r1d1Delay));
            allIpv4Interfaces[count - 1] = assignIp(count,
                                                    p2p.Install(router1Nodes.Get(i), dishes1Nodes.Get(j)));
            count++;
        }
    }

    // connect dishes to satellites on the left of the network
    for (uint32_t i = 0; i < nDishes1; i++) {
        for (uint32_t j = 0; j < nSatelliteNodes; j++) {
            p2p.SetDeviceAttribute("DataRate", StringValue(upSatDataRate));
            p2p.SetChannelAttribute("Delay", StringValue(upSatDelay));

            NetDeviceContainer dds = p2p.Install(dishes1Nodes.Get(i), satNodes.Get(j));

            // Apply Error Model
            Ptr <RateErrorModel> rem = CreateObject<RateErrorModel>();
            Ptr <UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
            rem->SetRandomVariable(uv);
            rem->SetRate(ber);
            rem->SetUnit(RateErrorModel::ERROR_UNIT_BIT);
            Ptr <RateErrorModel> rem1 = CreateObject<RateErrorModel>();
            Ptr <UniformRandomVariable> uv1 = CreateObject<UniformRandomVariable>();
            rem1->SetRandomVariable(uv1);
            rem1->SetRate(ber);
            rem1->SetUnit(RateErrorModel::ERROR_UNIT_BIT);

            dds.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(rem));
            dds.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(rem1));


            allIpv4Interfaces[count - 1] = assignIp(count, dds);
            count++;
        }
    }

    // connect satellites to dishes on the right of the network
    for (uint32_t i = 0; i < nSatelliteNodes; i++) {
        for (uint32_t j = 0; j < nDishes2; j++) {
            p2p.SetDeviceAttribute("DataRate", StringValue(downSatDataRate));
            p2p.SetChannelAttribute("Delay", StringValue(downSatDelay));


            NetDeviceContainer dds = p2p.Install(satNodes.Get(i), dishes2Nodes.Get(j));

            // Apply Error Model
            Ptr <RateErrorModel> rem = CreateObject<RateErrorModel>();
            Ptr <UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
            rem->SetRandomVariable(uv);
            rem->SetRate(ber);
            rem->SetUnit(RateErrorModel::ERROR_UNIT_BIT);

            Ptr <RateErrorModel> rem1 = CreateObject<RateErrorModel>();
            Ptr <UniformRandomVariable> uv1 = CreateObject<UniformRandomVariable>();
            rem1->SetRandomVariable(uv1);
            rem1->SetRate(ber);
            rem1->SetUnit(RateErrorModel::ERROR_UNIT_BIT);


            dds.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(rem));
            dds.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(rem1));

            allIpv4Interfaces[count - 1] = assignIp(count, dds);
            count++;

        }
    }

    // connect dishes to routers
    for (uint32_t i = 0; i < nDishes2; i++) {
        for (uint32_t j = 0; j < nRouters2; j++) {
            p2p.SetDeviceAttribute("DataRate", StringValue(d2r2DataRate));
            p2p.SetChannelAttribute("Delay", StringValue(d2r2Delay));
            allIpv4Interfaces[count - 1] = assignIp(count,
                                                    p2p.Install(dishes2Nodes.Get(i), router2Nodes.Get(j)));
            count++;
        }
    }

    // connect routers to terminals
    for (uint32_t i = 0; i < nRouters2; i++) {
        for (uint32_t j = 0; j < nTerminals2; j++) {
            p2p.SetDeviceAttribute("DataRate", StringValue(r2t2DataRate));
            p2p.SetChannelAttribute("Delay", StringValue(r2t2Delay));
            allIpv4Interfaces[count - 1] = assignIp(count,
                                                    p2p.Install(router2Nodes.Get(i), terminal2Nodes.Get(j)));
            count++;
        }
    }

    // Create router nodes, initialize routing database and set up the routing
    // tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    ApplicationContainer allSendApps[connPerTerminal];
    ApplicationContainer allSendApps1[connPerTerminal];
    ApplicationContainer allSendApps2[connPerTerminal];
    ApplicationContainer allSendApps3[connPerTerminal];
    ApplicationContainer allSendApps4[connPerTerminal];


    // Initialize all send apps
    // we loop through in order to initialize multiple TCP connections at each terminal
    for (int i = 0; i < connPerTerminal; i++ ) {

        // First terminal apps
        BulkSendHelper oo("ns3::TcpSocketFactory", InetSocketAddress(allIpv4Interfaces[connectionsCount - 1].GetAddress(1), port));
        allSendApps[i] = oo.Install(terminal1Nodes.Get(0));
        allSendApps[i].Start(Seconds(0.0));
        allSendApps[i].Stop(Seconds(appStop));

        // Second terminal apps
        BulkSendHelper oo1("ns3::TcpSocketFactory", InetSocketAddress(allIpv4Interfaces[connectionsCount - 2].GetAddress(1), port));
        allSendApps1[i] = oo1.Install(terminal1Nodes.Get(1));
        allSendApps1[i].Start(Seconds(appStart));
        allSendApps1[i].Stop(Seconds(appStop));

        // Third terminal apps
        BulkSendHelper oo2("ns3::TcpSocketFactory", InetSocketAddress(allIpv4Interfaces[connectionsCount - 3].GetAddress(1), port));
        allSendApps2[i] = oo2.Install(terminal1Nodes.Get(2));
        allSendApps2[i].Start(Seconds(appStart));
        allSendApps2[i].Stop(Seconds(appStop));

        // Fourth terminal apps
        BulkSendHelper oo3("ns3::TcpSocketFactory", InetSocketAddress(allIpv4Interfaces[connectionsCount - 4].GetAddress(1), port));
        allSendApps3[i] = oo3.Install(terminal1Nodes.Get(3));
        allSendApps3[i].Start(Seconds(appStart));
        allSendApps3[i].Stop(Seconds(appStop));

        // Fifth terminal apps
        BulkSendHelper oo4("ns3::TcpSocketFactory", InetSocketAddress(allIpv4Interfaces[connectionsCount - 5].GetAddress(1), port));
        allSendApps4[i] = oo4.Install(terminal1Nodes.Get(4));
        allSendApps4[i].Start(Seconds(appStart));
        allSendApps4[i].Stop(Seconds(appStop));
    }

    // Initialize packet sinks at each receiving terminal
    ApplicationContainer allSinkApps[nTerminals2];

    for (uint32_t i = 0; i < nTerminals2; i++) {

        PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
        allSinkApps[i] = packetSinkHelper.Install(terminal2Nodes.Get(i));
        allSinkApps[i].Start(Seconds(appStart));
        allSinkApps[i].Stop(Seconds(sinkStop));
    }


    // Output config store to txt format
    Config::SetDefault("ns3::ConfigStore::Filename",
                       StringValue("output-attributes.txt"));
    Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("RawText"));
    Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Save"));
    ConfigStore outputConfig2;
    outputConfig2.ConfigureDefaults();
    outputConfig2.ConfigureAttributes();

    // initialize flowmonitor
    Ptr <FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Run simulation
    Simulator::Stop(Seconds(sinkStop));
    Simulator::Run();
    Simulator::Destroy();

    /*
     * Throughput computation
     */


    uint32_t totData = 0;
    double totTime = 0;

    // Loop through packet sinks to get total amount of data received
    for (uint32_t i = 0; i < nTerminals2_2; i++) {

        uint64_t sinkData = (DynamicCast<PacketSink>(allSinkApps[i].Get(0)))->GetTotalRx();
        double lastRx = (DynamicCast<PacketSink>(allSinkApps[i].Get(0)))->GetLastRxTime();
        totData += sinkData;
        totTime += lastRx;
    }

    // Compute throughput and convert from bytes to bits
    double outputData = (totData / (totTime / 5)) * 8;
    std::stringstream ss1;
    ss1 << std::to_string(outputData);
    std::string outputstream = ss1.str();

    // output throughput
    // NS_LOG_ERROR to distinguish log from other NS3 logs
    NS_LOG_ERROR(outputData);

    return 0;
}
