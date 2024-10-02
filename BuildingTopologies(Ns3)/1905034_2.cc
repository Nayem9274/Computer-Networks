#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-error-rate-model.h"


// Default Network Topology ( STA'S are moving while AP's are fixed )
//
//   Wifi 10.1.2.0
//                     AP
//  *    *    *    *   *
//  |    |    |    |   |    10.1.1.0
// n6   n7   n8   n9   n0 -------------- n1   n2   n3   n4   n5 
//                        point-to-point  |    |    |    |    |
//                                        *    *    *    *    * 
//                                       AP
//                                           Wifi 10.1.3.0
#define uint_32 uint
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("1905034_2");

// MyApp is a custom application class that inherits from the Application class provided by ns-3.
class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  // the declaration of the MyApp class. It includes various methods for setting up the application, starting and stopping it, scheduling packet transmissions, and sending packets.
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  /*
  ScheduleTx(): This method is used to schedule the transmission of packets. It is called from StartApplication() to initiate packet transmissions periodically.
  SendPacket(): This method is used to send a single packet. It is called by ScheduleTx() to send a packet at a scheduled time.
  */

  void ScheduleTx (void);  
  void SendPacket (void);

  Ptr<Socket>     m_socket;  // A pointer to a Socket object, which will be used to send packets.
  Address         m_peer;     //An Address object representing the peer address to which packets will be sent.
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent; //  An EventId object used to schedule packet transmissions
  bool            m_running;    // whether app running or not 
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}
/* This function starts the application by setting the m_running flag to true, binding the socket, and connecting it to the peer address. 
Then, it calls the SendPacket function to start sending packets
*/

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}
double simulationTime = 20;                        /* Simulation time in seconds. */

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  Time curr = Simulator :: Now();
  //Check if the current simulation time (in seconds) is less than the specified simulation time (simulationTime)
  if(curr.GetSeconds() < simulationTime){
      ScheduleTx();
  }  

}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
        // // Calculate the time interval for the next packet transmission
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      //Schedule the transmission of the next packet by calling the SendPacket() method
      // The transmission will occur after the calculated time interval (tNext)
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}


// p-packet which is dropped, f- used to specify the pcap file where the dropped packet will be logged.
static void
RxDrop (Ptr<PcapFileWrapper> f, Ptr<const Packet> p)
{   
  std::cout << "Packet Dropped\n";
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ()); // printing drop time
  f->Write (Simulator::Now (), p); // writing in p-cap file
}


Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes for throughput calculation*/

void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     
  /*  calculates the current throughput in megabits per second (Mbit/s) using the total received bytes.
   The calculation is as follows:
sink->GetTotalRx(): This method retrieves the total number of received bytes by the PacketSink (sink).
lastTotalRx: This variable holds the value of the total received bytes at the last measurement point. 
*8- bytes to bit
*/
  std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}



int main(int argc, char *argv[]){
    uint nWifi = 0;
    uint nFlow = 20;
    uint nNodes = 20;

    uint nPacketsPerSecond = 500;
    uint payloadSize = 1024;                      
    uint nPackets = 2000;
    
    double speed = 5.0; //
    std::string filename = ".scratch/1905034_2";


   
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize)); // it determines the size of data that TCP will send in each individual segment

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes", "Number of nodes", nNodes);
    cmd.AddValue("nFlow", "Number of Flow", nFlow);
    cmd.AddValue("nPacketsPerSecond", "Number of packets per second", nPacketsPerSecond);
    cmd.AddValue("speed", "Speed of mobile station nodes", speed);
    cmd.AddValue("filename", "Filename", filename);
    cmd.Parse(argc, argv);



    std::string dataRate = std::to_string(nPacketsPerSecond*payloadSize*8/1024) + "Kbps";/* Application layer datarate. */
    
   
    /*For the static wireless network (bottleneck dumbbell topology), the left side consists of sender nodes (STA)
     and the right side consists of receiver nodes (STA). 
     To achieve this, half of the total number of flows should be created between the left and right sides of the network.
    */

    nWifi = nNodes/2 - 1; // Nodes in each side of AP. So 9 in each side . 9+9+2(AP)=20
    uint halfFlow = nFlow/2;

    //try and create nodes
    NodeContainer p2pNodes;
    p2pNodes.Create(2); //AP nodes

    PointToPointHelper pointToPoint;
    // Setting BottleNeckDataRate and BottleNeckdelay 
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (p2pNodes);
    
    // 3 newtorks. 1 ap-ap, other 2 ap with respective wifi stations
    
    NodeContainer wifiStaNodesL, wifiStaNodesR;   // LEFT & RIGHT of 2 AP
    NodeContainer wifiApNodeL = p2pNodes.Get (0); // LEFT
    wifiStaNodesL.Create (nWifi); 

    NodeContainer wifiApNodeR = p2pNodes.Get (1); //RIGHT
    wifiStaNodesR.Create (nWifi);

    
    // a helper class for configuration of wiresless channels
    YansWifiChannelHelper LwifiChannel = YansWifiChannelHelper::Default ();
    YansWifiChannelHelper RwifiChannel = YansWifiChannelHelper::Default ();

    //for simple range-based loss- RangePropagationLossModel
    LwifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");
    RwifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");
    YansWifiPhyHelper LwifiPhy; // represents the physical layer of left side
    YansWifiPhyHelper RwifiPhy; // // represents the physical layer of right side


    LwifiPhy.SetChannel (LwifiChannel.Create ());
    RwifiPhy.SetChannel (RwifiChannel.Create ());
    

    WifiMacHelper LwifiMac;
    WifiMacHelper RwifiMac;

    WifiHelper LwifiHelper;
    WifiHelper RwifiHelper;
    
   // RSM is responsible for managing the rate adaptation and transmission parameters for Wi-Fi devices during communication.  
   LwifiHelper.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");
   RwifiHelper.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");

    Ssid ssid= Ssid ("ns-3-ssid"); //e Service Set Identifier (SSID) used to identify a particular Wi-Fi network. 
    
    LwifiMac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false)); // setting ssid and activeprobing false- it wont search for available networks

    RwifiMac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
    
    NetDeviceContainer staDevicesL;
    NetDeviceContainer staDevicesR;
    staDevicesL = LwifiHelper.Install(LwifiPhy, LwifiMac, wifiStaNodesL);
    staDevicesR = RwifiHelper.Install(RwifiPhy, RwifiMac, wifiStaNodesR);


    LwifiMac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

    RwifiMac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
    
    NetDeviceContainer apDevicesL;
    NetDeviceContainer apDevicesR;

    
    apDevicesL = LwifiHelper.Install(LwifiPhy, LwifiMac, wifiApNodeL);
    apDevicesR = RwifiHelper.Install(RwifiPhy, RwifiMac, wifiApNodeR);

    std::cout << "NodeCount: " << ns3::NodeList::GetNNodes() << std::endl;
    
    


    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (0.0),
                                    "MinY", DoubleValue (0.0),
                                    "DeltaX", DoubleValue (0.5),
                                    "DeltaY", DoubleValue (1.0),
                                    "GridWidth", UintegerValue (3),
                                    "LayoutType", StringValue ("RowFirst"));

    

     // tell STA nodes how to move
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)),
                             "Speed", StringValue("ns3::ConstantRandomVariable[Constant="+std::to_string(speed)+"]"));

    mobility.Install (wifiStaNodesL);
    mobility.Install (wifiStaNodesR);

    
    // tell AP node to stay still
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   // install on AP node
    mobility.Install(wifiApNodeL);
    mobility.Install (wifiApNodeR);
    
    //*IP Layer
    
    InternetStackHelper stack;   // set up internet protocol stack on nodes
    stack.Install(wifiApNodeR);
    stack.Install(wifiApNodeL);
    stack.Install(wifiStaNodesL);
    stack.Install(wifiStaNodesR);

    //*set address
    Ipv4AddressHelper address;

    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pDevices); //Address of Bottleneck (p2p devices) //*L: 10.1.1.1, R: 10.1.1.2

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterfacesL = address.Assign (apDevicesL);     //* Left AP Device address: 10.1.2.1 
    Ipv4InterfaceContainer staInterfacesL = address.Assign (staDevicesL);   //*10.1.2.2 - 10.1.2.(nWifi+1)

    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterfacesR = address.Assign (apDevicesR);     //* Right AP Device address: 10.1.3.1
    Ipv4InterfaceContainer staInterfacesR = address.Assign (staDevicesR);   //*10.1.3.2 - 10.1.3.(nWifi+1)



    uint idx; // no. of initial flows
    if(halfFlow < nWifi)idx=halfFlow; //ensuring that we don't attempt to create more flows than the available Wi-Fi client nodes on one side.
    else{ 
        idx = nWifi;  // enough Wi-Fi client nodes on one side to establish all the initial flows.
    }
    for(uint i = 0; i< idx; i++){

        uint16_t port = 8000+i;
        // packetsink helper to handle incoming packets on R side
        PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
        ApplicationContainer sinkApp = sinkHelper.Install (wifiStaNodesR.Get(i));
        
        sinkApp.Start (Seconds (0.));
        sinkApp.Stop (Seconds(simulationTime));

        // TCP socket at left side for transmission from L to R
        Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (wifiStaNodesL.Get (i), TcpSocketFactory::GetTypeId ());
        Ptr<MyApp> app = CreateObject<MyApp> ();
        // setting up the custom Application object with desired values
        app->Setup (ns3TcpSocket, InetSocketAddress(staInterfacesR.GetAddress(i), port), payloadSize, nPackets, DataRate (dataRate));
        /*Attaching the custom application (MyApp) to the left side (L) Wi-Fi access point (wifiApNodeL). 
        This means that the L Wi-Fi access point will be responsible for generating traffic and sending it to the R Wi-Fi client nodes.*/
        wifiApNodeL.Get (0)->AddApplication (app);
        app->SetStartTime (Seconds (1.));// to avoid conjestion starts at 1
        app->SetStopTime (Seconds (simulationTime-1.0));
        

    }

    uint flowLeft = halfFlow - nWifi; 
    bool flowDone = false;
    if(idx == nWifi){

        //*need more flows

        //* loop each node. (L1 -> R1 Flow has been established. Now create flows from L1 -> Rx, L2 -> Rx until flow is not left)
        for(uint i = 0; i < nWifi; i++){

            for(uint j = 0; j<nWifi; j++){
                if(j==i)continue; // This condition ensures that a flow from a Wi-Fi client node to itself is not created, which is not required.
                //* ADD FLOW

                uint16_t port = 8100+i+j;
                PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
                ApplicationContainer sinkApp = sinkHelper.Install (wifiStaNodesR.Get(j));
                
                sinkApp.Start (Seconds (0.));
                sinkApp.Stop (Seconds(simulationTime));

                Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (wifiStaNodesL.Get (i), TcpSocketFactory::GetTypeId ());
                Ptr<MyApp> app = CreateObject<MyApp> ();
                app->Setup (ns3TcpSocket, InetSocketAddress(staInterfacesR.GetAddress(j), port), payloadSize, nPackets, DataRate (dataRate));
                wifiApNodeL.Get (0)->AddApplication (app);
                app->SetStartTime (Seconds (1.));
                app->SetStopTime (Seconds (simulationTime-1.0));
                
                //*update flow
                flowLeft--;
                if(flowLeft == 0){
                    flowDone = true;
                    break;
                }
            }
            if(flowDone == true)break;
            
        }
            
        

    }



    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    std::ofstream out;

    //Add Flow Monitor
    FlowMonitorHelper fmhelp;
    Ptr<FlowMonitor> monitor = fmhelp.InstallAll(); // monitor variable holds the pointer to the FlowMonitor object that will record and collect flow statistics during the simulation.


    PcapHelper pcapHelper;// store captured packets from simulation
    Ptr<PcapFileWrapper> file = pcapHelper.CreateFile ("scratch/1905034_2.pcap", std::ios::out, PcapHelper::DLT_PPP);
    p2pDevices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&RxDrop, file));
    
    Simulator::Stop (Seconds (simulationTime+2.0));
    Simulator::Run ();
    


    Ptr<Ipv4FlowClassifier> classifier = DynamicCast <Ipv4FlowClassifier> (fmhelp.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    uint32_t PacketsSent = 0;
    uint32_t PacketsReceived = 0;
    uint32_t PacketsDrop = 0;
    double AvgThroughput = 0;
    double AvgSendThroughput = 0;
    double CurrThroughput = 0;
    Time Avgdelay;
    
    uint32_t j = 0;
    uint32_t total = 0;
    for (auto iter = stats.begin(); iter != stats.end(); iter++)
    { total++;}
    for (auto iter = stats.begin(); iter != stats.end(); iter++)
    { 
      
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);
      CurrThroughput = (iter->second.rxBytes*8.0/1024);
      
      if(j<=total/2){

        AvgSendThroughput += CurrThroughput;

      }
      
      NS_LOG_UNCOND("\nFlow ID: " << iter->first);
      NS_LOG_UNCOND("Src Addr: " << t.sourceAddress << "\t Destination Addr: " << t.destinationAddress);
      NS_LOG_UNCOND("Sent Packets: " << iter->second.txPackets );
      NS_LOG_UNCOND("Received Packets: " << iter->second.rxPackets );
      NS_LOG_UNCOND("Lost Packets: " << iter->second.lostPackets );
      NS_LOG_UNCOND("Packet Delivery Ratio: " << iter->second.rxPackets*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Packet Drop Ratio: " << (iter->second.lostPackets)*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Delay: " << iter->second.delaySum);
      NS_LOG_UNCOND("Flow Throughput: " << CurrThroughput << "Kbps");


      PacketsSent += iter->second.txPackets;
      PacketsReceived += iter->second.rxPackets ;
      PacketsDrop +=  iter->second.lostPackets;
      AvgThroughput += CurrThroughput;
      Avgdelay += iter->second.delaySum;
      j++;
    } 
      AvgThroughput/=(simulationTime+2);
      Avgdelay=Avgdelay/PacketsReceived;
    //   Avgdelay /= 1e+9;
      AvgSendThroughput = AvgSendThroughput/(simulationTime+2/2);

      NS_LOG_UNCOND("\n\nTOTAL SIMULATION: ");
      NS_LOG_UNCOND("Sent Packets: " << PacketsSent);
      NS_LOG_UNCOND("Received Packets: " << PacketsReceived);
      NS_LOG_UNCOND("Dropped Packets: " << PacketsDrop );
      NS_LOG_UNCOND("Packet Delivery Ratio: " << PacketsReceived*100.0/PacketsSent << "%");
      NS_LOG_UNCOND("Packet Drop Ratio: " << PacketsDrop*100.0/PacketsSent << "%");
      NS_LOG_UNCOND("Average Throughput: " <<AvgThroughput<< "Kbps");
      NS_LOG_UNCOND("Average delay: " <<Avgdelay);
      NS_LOG_UNCOND("Average Throughput without ACK: " <<AvgSendThroughput<< "Kbps");

    out.open(filename, std::ios_base::app);
    // 
    out << nNodes << " " << nFlow << " " << nPacketsPerSecond << " " << speed  << " ";
  // 
    out << AvgThroughput << " " <<PacketsDrop*100.0/PacketsSent <<std::endl;

    Simulator::Destroy ();
    



    return 0;


}
