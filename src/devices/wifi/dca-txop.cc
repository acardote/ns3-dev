/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include <cassert>

#include "ns3/packet.h"

#include "dca-txop.h"
#include "dcf.h"
#include "mac-parameters.h"
#include "mac-low.h"
#include "wifi-mac-queue.h"
#include "mac-tx-middle.h"
#include "wifi-phy.h"



#define noDCA_TXOP_TRACE 1

#ifdef DCA_TXOP_TRACE
#include "ns3/simulator.h"
#include <iostream>
# define TRACE(x) \
  std::cout <<"DCA TXOP now="<<Simulator::NowUs ()<<"us "<<x<<std::endl;
#else /* DCA_TXOP_TRACE */
# define TRACE(x)
#endif /* DCA_TXOP_TRACE */

namespace ns3 {

class DcaTxop::NavListener : public ns3::MacLowNavListener {
public:
  NavListener (ns3::Dcf *dcf)
    : m_dcf (dcf) {}
  virtual ~NavListener () {}
  virtual void NavStart (Time now, Time duration) {
    m_dcf->NotifyNavStart (now, duration);
  }
  virtual void NavContinue (Time now, Time duration) {
    m_dcf->NotifyNavContinue (now, duration);
  }
  virtual void NavReset (Time now, Time duration) {
    m_dcf->NotifyNavReset (now, duration);
  }
private:
  ns3::Dcf *m_dcf;
};
class DcaTxop::PhyListener : public ns3::WifiPhyListener {
public:
  PhyListener (ns3::Dcf *dcf)
    : m_dcf (dcf) {}
  virtual ~PhyListener () {}
  virtual void NotifyRxStart (Time duration) {
    m_dcf->NotifyRxStartNow (duration);
  }
  virtual void NotifyRxEndOk (void) {
    m_dcf->NotifyRxEndOkNow ();
  }
  virtual void NotifyRxEndError (void) {
    m_dcf->NotifyRxEndErrorNow ();
  }
  virtual void NotifyTxStart (Time duration) {
    m_dcf->NotifyTxStartNow (duration);
  }
  virtual void NotifyCcaBusyStart (Time duration) {
    m_dcf->NotifyCcaBusyStartNow (duration);
  }
private:
  ns3::Dcf *m_dcf;
};


class DcaTxop::AccessListener : public DcfAccessListener {
public:
  AccessListener (DcaTxop *txop)
    : DcfAccessListener (),
      m_txop (txop) {}

  virtual ~AccessListener () {}

  virtual void AccessGrantedNow (void)
  {
    m_txop->AccessGrantedNow ();
  }
  virtual bool AccessNeeded (void)
  {
    return m_txop->AccessNeeded ();
  }
  virtual bool AccessingAndWillNotify (void)
  {
    return m_txop->AccessingAndWillNotify ();
  }

private:
  DcaTxop *m_txop;
};

class DcaTxop::TransmissionListener : public MacLowTransmissionListener {
public:
  TransmissionListener (DcaTxop *txop)
    : MacLowTransmissionListener (),
      m_txop (txop) {}
      
  virtual ~TransmissionListener () {}

  virtual void GotCts (double snr, WifiMode txMode) {
    m_txop->GotCts (snr, txMode);
  }
  virtual void MissedCts (void) {
    m_txop->MissedCts ();
  }
  virtual void GotAck (double snr, WifiMode txMode) {
    m_txop->GotAck (snr, txMode);
  }
  virtual void MissedAck (void) {
    m_txop->MissedAck ();
  }
  virtual void StartNext (void) {
    m_txop->StartNext ();
  }

  virtual void Cancel (void) {
    assert (false);
  }

private:
  DcaTxop *m_txop;
};

DcaTxop::DcaTxop ()
  : m_accessListener (0),
    m_hasCurrent (false),
    m_ssrc (0),
    m_slrc (0)
{
  m_transmissionListener = new DcaTxop::TransmissionListener (this);
  m_dcf = new Dcf ();
  m_accessListener = new DcaTxop::AccessListener (this);
  m_dcf->RegisterAccessListener (m_accessListener);
  m_queue = new WifiMacQueue ();
}

DcaTxop::~DcaTxop ()
{
  delete m_accessListener;
  delete m_transmissionListener;
  delete m_navListener;
  delete m_phyListener;
  delete m_queue;
}

void 
DcaTxop::SetLow (MacLow *low)
{
  m_low = low;
  m_navListener = new DcaTxop::NavListener (m_dcf);
  m_low->RegisterNavListener (m_navListener);
}
void
DcaTxop::SetPhy (WifiPhy *phy)
{
  m_phyListener = new DcaTxop::PhyListener (m_dcf);
  phy->RegisterListener (m_phyListener);
}
void 
DcaTxop::SetParameters (MacParameters *parameters)
{
  m_parameters = parameters;
  m_dcf->SetParameters (parameters);
}
void 
DcaTxop::SetTxMiddle (MacTxMiddle *txMiddle)
{
  m_txMiddle = txMiddle;
}
void 
DcaTxop::SetTxOkCallback (TxOk callback)
{
  m_txOkCallback = callback;
}
void 
DcaTxop::SetTxFailedCallback (TxFailed callback)
{
  m_txFailedCallback = callback;
}

void 
DcaTxop::SetDifs (Time difs)
{
  m_dcf->SetDifs (difs);
}
void 
DcaTxop::SetEifs (Time eifs)
{
  m_dcf->SetEifs (eifs);
}
void 
DcaTxop::SetCwBounds (uint32_t min, uint32_t max)
{
  m_dcf->SetCwBounds (min, max);
}
void 
DcaTxop::SetMaxQueueSize (uint32_t size)
{
  m_queue->SetMaxSize (size);
}
void 
DcaTxop::SetMaxQueueDelay (Time delay)
{
  m_queue->SetMaxDelay (delay);
}

void 
DcaTxop::Queue (Packet packet, WifiMacHeader const &hdr)
{
  m_queue->Enqueue (packet, hdr);
  m_dcf->RequestAccess ();
}


MacLow *
DcaTxop::Low (void)
{
  return m_low;
}

MacParameters *
DcaTxop::Parameters (void)
{
  return m_parameters;
}


bool
DcaTxop::NeedRts (void)
{
  if (m_currentPacket.GetSize () > Parameters ()->GetRtsCtsThreshold ()) 
    {
      return true;
    } 
  else 
    {
      return false;
    }
}

bool
DcaTxop::NeedFragmentation (void)
{
  if (m_currentPacket.GetSize () > Parameters ()->GetFragmentationThreshold ()) 
    {
      return true;
    } 
  else 
    {
      return false;
    }
}

uint32_t
DcaTxop::GetNFragments (void)
{
  uint32_t nFragments = m_currentPacket.GetSize () / Parameters ()->GetFragmentationThreshold () + 1;
  return nFragments;
}
void
DcaTxop::NextFragment (void)
{
  m_fragmentNumber++;
}

uint32_t
DcaTxop::GetLastFragmentSize (void)
{
  uint32_t lastFragmentSize = m_currentPacket.GetSize () %
    Parameters ()->GetFragmentationThreshold ();
  return lastFragmentSize;
}

uint32_t
DcaTxop::GetFragmentSize (void)
{
  return Parameters ()->GetFragmentationThreshold ();
}
bool
DcaTxop::IsLastFragment (void) 
{
  if (m_fragmentNumber == (GetNFragments () - 1)) 
    {
      return true;
    } 
  else 
    {
      return false;
    }
}

uint32_t
DcaTxop::GetNextFragmentSize (void) 
{
  if (IsLastFragment ()) 
    {
      return 0;
    }
  
  uint32_t nextFragmentNumber = m_fragmentNumber + 1;
  if (nextFragmentNumber == (GetNFragments () - 1)) 
    {
      return GetLastFragmentSize ();
    } 
  else 
    {
      return GetFragmentSize ();
    }
}

Packet 
DcaTxop::GetFragmentPacket (WifiMacHeader *hdr)
{
  *hdr = m_currentHdr;
  hdr->SetFragmentNumber (m_fragmentNumber);
  uint32_t startOffset = m_fragmentNumber * GetFragmentSize ();
  Packet fragment;
  if (IsLastFragment ()) 
    {
      hdr->SetNoMoreFragments ();
      fragment = m_currentPacket.CreateFragment (startOffset, 
                                                 GetLastFragmentSize ());
    } 
  else 
    {
      hdr->SetMoreFragments ();
      fragment = m_currentPacket.CreateFragment (startOffset, 
                                                 GetFragmentSize ());
    }
  return fragment;
}

bool 
DcaTxop::AccessingAndWillNotify (void)
{
  if (m_hasCurrent) 
    {
      return true;
    } 
  else 
    {
      return false;
    }
}

bool 
DcaTxop::AccessNeeded (void)
{
  if (!m_queue->IsEmpty () ||
      m_hasCurrent) 
    {
      TRACE ("access needed here");
      return true;
    } 
  else 
    {
      TRACE ("no access needed here");
      return false;
    }
}

void
DcaTxop::AccessGrantedNow (void)
{
  if (!m_hasCurrent) 
    {
      if (m_queue->IsEmpty ()) 
        {
          TRACE ("queue empty");
          return;
        }
      bool found;
      m_currentPacket = m_queue->Dequeue (&m_currentHdr, &found);
      assert (found);
      m_hasCurrent = true;
      assert (m_hasCurrent);
      uint16_t sequence = m_txMiddle->GetNextSequenceNumberfor (&m_currentHdr);
      m_currentHdr.SetSequenceNumber (sequence);
      m_currentHdr.SetFragmentNumber (0);
      m_currentHdr.SetNoMoreFragments ();
      m_ssrc = 0;
      m_slrc = 0;
      m_fragmentNumber = 0;
      TRACE ("dequeued size="<<m_currentPacket.GetSize ()<<
             ", to="<<m_currentHdr.GetAddr1 ()<<
             ", seq="<<m_currentHdr.GetSequenceControl ()); 
    }
  MacLowTransmissionParameters params;
  params.DisableOverrideDurationId ();
  if (m_currentHdr.GetAddr1 ().IsBroadcast ()) 
    {
      params.DisableRts ();
      params.DisableAck ();
      params.DisableNextData ();
      Low ()->StartTransmission (m_currentPacket,
                                 &m_currentHdr,
                                 params,
                                 m_transmissionListener);
      m_hasCurrent = false;
      m_dcf->ResetCw ();
      m_dcf->StartBackoff ();
      TRACE ("tx broadcast");
    } 
  else 
    {
      params.EnableAck ();
      
      if (NeedFragmentation ()) 
        {
          params.DisableRts ();
          WifiMacHeader hdr;
          Packet fragment = GetFragmentPacket (&hdr);
          if (IsLastFragment ()) 
            {
              TRACE ("fragmenting last fragment size="<<fragment->GetSize ());
              params.DisableNextData ();
            } 
          else 
            {
              TRACE ("fragmenting size="<<fragment->GetSize ());
              params.EnableNextData (GetNextFragmentSize ());
            }
          Low ()->StartTransmission (fragment, &hdr, params, 
                                     m_transmissionListener);
        } 
      else 
        {
          if (NeedRts ()) 
            {
              params.EnableRts ();
              TRACE ("tx unicast rts");
            } 
          else 
            {
              params.DisableRts ();
              TRACE ("tx unicast");
            }
          params.DisableNextData ();
          // We need to make a copy in case we need to 
          // retransmit the packet: the MacLow modifies the input
          // Packet so, we would retransmit a modified packet
          // if we were not to make a copy.
          // XXX the comment above and the code below do not
          // make sense anymore. So, we should remove both.
          Packet copy = m_currentPacket;
          Low ()->StartTransmission (copy, &m_currentHdr,
                                     params, m_transmissionListener);
        }
    }
}


void 
DcaTxop::GotCts (double snr, WifiMode txMode)
{
  TRACE ("got cts");
  m_ssrc = 0;
}
void 
DcaTxop::MissedCts (void)
{
  TRACE ("missed cts");
  m_ssrc++;
  m_ctstimeoutTrace (m_ssrc);
  if (m_ssrc > Parameters ()->GetMaxSsrc ()) 
    {
      // to reset the dcf.
      m_dcf->ResetCw ();
      m_dcf->StartBackoff ();
      m_hasCurrent = false;
    } 
  else 
    {
      m_dcf->UpdateFailedCw ();
      m_dcf->StartBackoff ();
    }
}
void 
DcaTxop::GotAck (double snr, WifiMode txMode)
{
  m_slrc = 0;
  if (!NeedFragmentation () ||
      IsLastFragment ()) 
    {
      TRACE ("got ack. tx done.");
      if (!m_txOkCallback.IsNull ()) 
        {
          m_txOkCallback (m_currentHdr);
        }

      /* we are not fragmenting or we are done fragmenting
       * so we can get rid of that packet now.
       */
      m_hasCurrent = false;
      m_dcf->ResetCw ();
      m_dcf->StartBackoff ();
    } 
  else 
    {
      TRACE ("got ack. tx not done, size="<<m_currentPacket.GetSize ());
    }
}
void 
DcaTxop::MissedAck (void)
{
  TRACE ("missed ack");
  m_slrc++;
  m_acktimeoutTrace (m_slrc);
  if (m_slrc > Parameters ()->GetMaxSlrc ()) 
    {
      // to reset the dcf.    
      m_dcf->ResetCw ();
      m_dcf->StartBackoff ();
      m_hasCurrent = false;
    } 
  else 
    {
      // XXX
      //SetRetry (m_currentTxPacket); 
      if (!m_txFailedCallback.IsNull ()) 
        {
          m_txFailedCallback (m_currentHdr);
        }
      m_dcf->UpdateFailedCw ();
      m_dcf->StartBackoff ();
    }
  
}
void 
DcaTxop::StartNext (void)
{
  TRACE ("start next packet fragment");
  /* this callback is used only for fragments. */
  NextFragment ();
  WifiMacHeader hdr;
  Packet fragment = GetFragmentPacket (&hdr);
  MacLowTransmissionParameters params;
  params.EnableAck ();
  params.DisableRts ();
  params.DisableOverrideDurationId ();
  if (IsLastFragment ()) 
    {
      params.DisableNextData ();
    } 
  else 
    {
      params.EnableNextData (GetNextFragmentSize ());
    }
  Low ()->StartTransmission (fragment, &hdr, params, m_transmissionListener);
}

} // namespace ns3
