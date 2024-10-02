/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
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
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 * and Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#ifndef TCP_ADAPTIVERENO_H
#define TCP_ADAPTIVERENO_H

#include "tcp-congestion-ops.h"
#include "tcp-westwood-plus.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/sequence-number.h"
#include "ns3/traced-value.h"

namespace ns3
{

class Time;
class Packet;
class TcpHeader;
class EventId;

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP Adaptive Reno.
 *

 */
class TcpAdaptiveReno : public TcpWestwoodPlus
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    TcpAdaptiveReno();
    /**
     * \brief Copy constructor
     * \param sock the object to copy
     */
    TcpAdaptiveReno(const TcpAdaptiveReno& sock);
    virtual ~TcpAdaptiveReno(void);

    /**
     * \brief Filter type (None or Tustin)
     */
    enum FilterType
    {
        NONE,
        TUSTIN
    };

    virtual uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight);

    virtual void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt) ;

    virtual Ptr<TcpCongestionOps> Fork() override;

  private:
    /**
     * Update the total number of acknowledged packets during the current RTT
     *
     * \param [in] acked the number of packets the currently received ACK acknowledges
     */
   // void UpdateAckedSegments(int acked);

    /**
     * Estimate the network's bandwidth
     *
     * \param [in] rtt the RTT estimation.
     * \param [in] tcb the socket state.
     */
    void EstimateBW(const Time& rtt, Ptr<TcpSocketState> tcb);

    double EstimateCongestionLevel();

    void EstimateIncWnd(Ptr<TcpSocketState> tcb);

  protected:

    virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

    Time                   m_minRtt;                 //!< Minimum RTT
    Time                   m_currentRtt;             //!< Current RTT
    Time                   m_jPacketLRtt;            //!< RTT of j packet loss
    Time                   m_conjRtt;                //!< Conjestion RTT (j th event)
    Time                   m_prevConjRtt;            //!< Previous Conjestion RTT (j-1 th event)

    // Window calculations
    int32_t                m_incWnd;                 //!< Increment Window
    uint32_t               m_baseWnd;                //!< Base Window
    int32_t                m_probeWnd;               //!< Probe Window 

    
};

} // namespace ns3

#endif /* TCP_ADPATIVE_RENO_H */
