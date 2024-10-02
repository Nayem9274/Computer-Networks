
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/callback.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("1905034");

// ===========================================================================
//
//            t0----      -----h0
//  senders - t1---r0 --- r1---h1 - receivers
//            t2----      -----h2
//
// ===========================================================================
//
// MyApp is a custom application class that inherits from the Application class provided by ns-3.

/* the MyApp class represents a custom application that can send packets using a socket. It has methods to set up the application, start and
 stop packet transmission, and handle packet scheduling and transmission. The member variables store the required parameters
  and state of the application.*/
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
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate);

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
  DataRate        m_dataRate;
  EventId         m_sendEvent;  //  An EventId object used to schedule packet transmissions
  bool            m_running;  // whether running or not
  uint32_t        m_packetsSent; 
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
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
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
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
double simulationTime=20;
//  This function sends a packet using the socket and schedules the next packet transmission if the simulation time has not reached the specified simulationTime.
void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  /*
   Time curr = Simulator :: Now();
  //Check if the current simulation time (in seconds) is less than the specified simulation time (simulationTime)
  if(curr.GetSeconds() < simulationTime){
      ScheduleTx();
  } 
  */

  if(Simulator::Now().GetSeconds() < simulationTime){
     ScheduleTx();
     }
}

// This function schedules the next packet transmission based on the packet size and data rate.
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

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  // NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " " << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
}



int main(int argc, char *argv[]){
    uint32_t payloadSize = 1024;
    std::string TCPvariant1 = "ns3::TcpNewReno"; // TcpNewReno
    std::string TCPvariant2 = "ns3::TcpAdaptiveReno"; //TcpAdaptiveReno TcpWestwood, TcpHighSpeed
    int nLeaf = 2;
    int nFlows = 2;
    std::string senderDataRate = "1Gbps";
    std::string senderDelay = "1ms";
    std::string output_folder = "./scratch/1905034";
//C:\Users\Asus\Desktop\ABDULLAH WASI\3-2\CSE 321 322\Lab\ns-allinone-3.39\ns-3.39\scratch
   // int simulationTime = 60;
   
    int BottleNeckDataRate = 50;
    int BottleNeckDelay = 100;//100 ms
    int packet_loss_exp = 6;
    int exp = 1;

     // Consider the packet size to be 1024 bytes. Suppose, packets are not segmented so setting the SegmentSize attribute of TCPSocket should work.

    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize)); // it determines the size of data that TCP will send in each individual segment


    
    CommandLine cmd (__FILE__);
    cmd.AddValue ("tcp2","Name of TCP variant 2", TCPvariant2);
    cmd.AddValue ("nLeaf","Number of  leaf nodes", nLeaf);
    cmd.AddValue ("BottleNeckDataRate","Boottle Neck Data Rate", BottleNeckDataRate);
    cmd.AddValue ("packetLossRate", "Packet loss rate", packet_loss_exp);
    cmd.AddValue ("output_folder","Folder to store dara", output_folder);
    cmd.AddValue ("exp","1 for bottleneck, 2 for packet loss rate", exp);
    cmd.Parse (argc,argv);

    std::string file = output_folder + "/data_" + std::to_string(exp) + ".txt";
    nFlows = nLeaf;
    double packet_loss_rate = (1.0 / std::pow(10, packet_loss_exp));
    std::string bottleNeckDataRate = std::to_string(BottleNeckDataRate) + "Mbps";
    std::string bottleNeckDelay = std::to_string(BottleNeckDelay) + "ms";
    

    NS_LOG_UNCOND("USING TCP 1 = "<<TCPvariant1<<" ; TCP 2 = "<<TCPvariant2<<" ; nLeaf = "<<nLeaf<<
                  " ; bottleneck rate = "<<bottleNeckDataRate<<
                  " ; packet loss rate = "<<packet_loss_rate<<
                  " ; sender data rate = "<<senderDataRate);

   

    // SETUP NODE AND DEVICE
    // Create the point-to-point link helpers
    PointToPointHelper pointToPoint;
      // Setting BottleNeckDataRate and BottleNeckdelay 
    pointToPoint.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckDataRate));
    pointToPoint.SetChannelAttribute ("Delay", StringValue (bottleNeckDelay));


    PointToPointHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute  ("DataRate", StringValue (senderDataRate));
    pointToPointLeaf.SetChannelAttribute ("Delay", StringValue (senderDelay));
    //To set the router buffer capacity to the bandwidth delay product and to get the buffer employ drop-tail 
    //discarding as mentioned in the reference paper, add this line to your senders and receivers’ P2P Helper:
    pointToPointLeaf.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue (std::to_string (BottleNeckDelay * BottleNeckDataRate) + "p"));

    PointToPointDumbbellHelper d (nLeaf, pointToPointLeaf,nLeaf, pointToPointLeaf,pointToPoint);

    // Add the RateErrorModel to the devices of the bottleneck link.
    Ptr<RateErrorModel> errorModel = CreateObject<RateErrorModel> ();
    errorModel->SetAttribute ("ErrorRate", DoubleValue (packet_loss_rate));
    // set receive error for bottleneck -> 0 dont work
    d.m_routerDevices.Get(1)->SetAttribute ("ReceiveErrorModel", PointerValue (errorModel)); 

    NS_LOG_UNCOND("USING TCP 11 = "<<TCPvariant1<<" ; TCP 22 = "<<TCPvariant2<<" ; nLeaf = "<<nLeaf<<
                  " ; bottleneck rate = "<<bottleNeckDataRate<<
                  " ; packet loss rate = "<<packet_loss_rate<<
                  " ; sender data rate = "<<senderDataRate);

   

    // *IP Layer
    // tcp variant 1
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (TCPvariant1));
    InternetStackHelper stack1;
    for (uint32_t i = 0; i < d.LeftCount (); i+=2)
      {
        stack1.Install (d.GetLeft (i)); // left leaves
      }
    for (uint32_t i = 0; i < d.RightCount (); i+=2)
      {
        stack1.Install (d.GetRight (i)); // right leaves
      }
    stack1.Install (d.GetLeft ());
    stack1.Install (d.GetRight ());

    // tcp variant 2
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (TCPvariant2));
    InternetStackHelper stack2;
    for (uint32_t i = 1; i < d.LeftCount (); i+=2)
      {
        stack2.Install (d.GetLeft (i)); // left leaves
      }
    for (uint32_t i = 1; i < d.RightCount (); i+=2)
      {
        stack2.Install (d.GetRight (i)); // right leaves
      }

    // ASSIGN IP Addresses
    d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"), // left nodes
                          Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),  // right nodes
                          Ipv4AddressHelper ("10.3.1.0", "255.255.255.0")); // routers 
    
    
    for(int i=0;i<nFlows; i++){
      uint16_t port = 8000+i;
      // packetsink helper to handle incoming packets on R side
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
      ApplicationContainer sinkApp = sinkHelper.Install (d.GetRight (i));

      sinkApp.Start (Seconds (0.));
      sinkApp.Stop (Seconds (simulationTime+2.0));

      // TCP socket at left side for transmission from L to R
      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (d.GetLeft (i), TcpSocketFactory::GetTypeId ());
      Ptr<MyApp> app = CreateObject<MyApp> ();
      // setting up the custom Application object with desired values
      app->Setup (ns3TcpSocket, (InetSocketAddress (d.GetRightIpv4Address (i), port)), payloadSize, DataRate (senderDataRate));
         /*Attaching the custom application (MyApp) to the left side (L) . 
        This means that the L access point will be responsible for generating traffic and sending it to the R nodes.*/
      d.GetLeft (i)->AddApplication (app);
      app->SetStartTime (Seconds (1.));
      app->SetStopTime (Seconds (simulationTime-1.0));

      std::ostringstream oss;
      oss << output_folder << "/flow" << i+1 <<  ".cwnd";
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (oss.str());
      ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
    }
    

    Ipv4GlobalRoutingHelper::PopulateRoutingTables (); //   ensuring that the nodes have the necessary routing information to forward packets properly.
    
    // install flow monitor
    FlowMonitorHelper fmhelp;
    Ptr<FlowMonitor> monitor = fmhelp.InstallAll ();

  
    Simulator::Stop (Seconds (simulationTime+2.0));
    Simulator::Run ();


    // flow monitor statistics
    int j = 0;
    float AvgThroughput = 0;
    float CurrThroughput = 0;
    // float CurPacketLossRate = 0;
    float CurThroughputArr[] = {0, 0};

    double jain_index_numerator = 0;
    double jain_index_denominator = 0;

    std::ofstream MyFile(output_folder, std::ios_base::app);

    // variables for output measurement
    uint32_t PacketsSent = 0;
    uint32_t PacketsReceived = 0;
    uint32_t PacketsDrop = 0;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelp.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    for (auto iter = stats.begin (); iter != stats.end (); ++iter) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); 

      CurrThroughput = iter->second.rxBytes * 8.0/(1024); // bytes- bits(*8), /1024- to convert to kilobitspersecond

      if(j%2 == 0) { CurThroughputArr[0] += iter->second.rxBytes; }
      if(j%2 == 1) { CurThroughputArr[1] += iter->second.rxBytes; }
      

      NS_LOG_UNCOND("\nFlow ID:" <<iter->first);
      NS_LOG_UNCOND("Src Addr" <<t.sourceAddress << "\t Destination Addr "<< t.destinationAddress);
      NS_LOG_UNCOND("Sent Packets = " <<iter->second.txPackets);
      NS_LOG_UNCOND("Received Packets = " <<iter->second.rxPackets);
      NS_LOG_UNCOND("Lost Packets = " <<iter->second.lostPackets);
      NS_LOG_UNCOND("Packet Delivery Ratio = " <<iter->second.rxPackets*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Packet Drop Ratio = " << (iter->second.lostPackets)*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Delay = " <<iter->second.delaySum / iter->second.rxPackets);
      NS_LOG_UNCOND("Jitter = " <<iter->second.jitterSum);
      NS_LOG_UNCOND("Throughput = " << CurrThroughput <<"Kbps");
    

    


      PacketsSent += iter->second.txPackets;
      PacketsReceived += iter->second.rxPackets ;
      PacketsDrop +=  iter->second.lostPackets;
      AvgThroughput += CurrThroughput;
       j++;

      jain_index_numerator += CurrThroughput;
      jain_index_denominator += (CurrThroughput * CurrThroughput);

      
    }

    double jain_index = (jain_index_numerator * jain_index_numerator) / ( j * jain_index_denominator);
   
    AvgThroughput/=(simulationTime+2); 

    // cols : bttlneck rate | random packet loss | throughput1 | throughput2 | jain_index
    MyFile << bottleNeckDataRate.substr(0, bottleNeckDataRate.length()-4) << " " << -1*packet_loss_exp << " " << CurThroughputArr[0] << " " << CurThroughputArr[1] << " " << " " << jain_index <<std::endl;
    

    NS_LOG_UNCOND("\n\nTOTAL SIMULATION: ");
    NS_LOG_UNCOND("Total Sent Packets  = " << PacketsSent);
    NS_LOG_UNCOND("Total Received Packets = " << PacketsReceived);
    NS_LOG_UNCOND("Total Dropped Packets = " << PacketsDrop);
    NS_LOG_UNCOND("Packet Delivery Ratio = " << ((PacketsReceived*100.00)/PacketsSent)<< "%");
    NS_LOG_UNCOND("Packet Loss ratio = " << ((PacketsDrop*100.00)/PacketsSent)<< "%");
    NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
    NS_LOG_UNCOND("Jain's Fairness Index = " << jain_index);
    
    
    Simulator::Destroy ();

    return 0;
}