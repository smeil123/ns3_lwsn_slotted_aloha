#include "lwsn-header.h"

namespace ns3{

LwsnHeader::LwsnHeader()
{
}

LwsnHeader::~LwsnHeader()
{
}

TypeId
LwsnHeader::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::LwsnHeader")
	  	.SetParent<Header> ()
	  	.AddConstructor<LwsnHeader> ()
	;

	return tid;
}

TypeId
LwsnHeader::GetInstanceTypeId (void) const
{
	return GetTypeId();
}

void
LwsnHeader::Print(std::ostream &os) const
{
	os << "MsgType -- Osid -- Psid -- e -- r  -- Did -- CreateTime " 
	<< m_Type << " "<< m_Osid << " " << m_Psid << " " 
	<< m_r << " " << m_e << " " << m_Did << " " << m_StartTime << std::endl;
}

void
LwsnHeader::Serialize(Buffer::Iterator start) const
{
	start.WriteHtonU16(m_Type);
	start.WriteHtonU16(m_Osid);
	start.WriteHtonU16(m_Psid);
	start.WriteHtonU16(m_e);
	start.WriteHtonU16(m_r);
	start.WriteHtonU16(m_Did);
	start.WriteHtonU16(m_StartTime);
	start.WriteHtonU16(m_Osid2);
	start.WriteHtonU16(m_Did2);
	start.WriteHtonU16(m_StartTime2);
}

uint32_t
LwsnHeader::Deserialize(Buffer::Iterator start)
{
	m_Type = start.ReadNtohU16();
	m_Osid = start.ReadNtohU16();
	m_Psid = start.ReadNtohU16();
	m_e = start.ReadNtohU16();
	m_r = start.ReadNtohU16();
	m_Did = start.ReadNtohU16();
	m_StartTime = start.ReadNtohU16();
	m_Osid2 = start.ReadNtohU16();
	m_Did2 = start.ReadNtohU16();
	m_StartTime2 = start.ReadNtohU16();

	return 20;
}

uint32_t
LwsnHeader::GetSerializedSize(void) const
{
	return 20;
}

void
LwsnHeader::SetType(enum LwsnType lwsnType)
{
	m_Type = lwsnType;
}

enum LwsnHeader::LwsnType 
LwsnHeader::GetType(void) const
{
	if(m_Type == 0)
		return G_ANC;
	else if(m_Type == 1)
		return ORIGINAL_TRANSMISSION;
	else if(m_Type == 2)
		return FORWARDING;
	else if(m_Type == 3)
		return IACK;
	else if(m_Type == 4)
		return NETWORK_CODING;
	else
		return IACK;
}

void 
LwsnHeader::SetOsid(uint16_t Osid)
{
	m_Osid = Osid;
}

uint16_t 
LwsnHeader::GetOsid(void) const
{
	return m_Osid;
}

void
LwsnHeader::SetOsid2(uint16_t Osid2)
{
	m_Osid2 = Osid2;
} 

uint16_t 
LwsnHeader::GetOsid2(void) const
{
	return m_Osid2;
}

void 
LwsnHeader::SetPsid(uint16_t Psid)
{
	m_Psid = Psid;
}

uint16_t 
LwsnHeader::GetPsid(void) const
{
	return m_Psid;
}

void 
LwsnHeader::SetE(uint16_t e)
{
	m_e = e;
}

uint16_t 
LwsnHeader::GetE(void) const
{
	return m_e;
}


void 
LwsnHeader::SetR(uint16_t r)
{
	m_r = r;
}

uint16_t 
LwsnHeader::GetR(void) const
{
	return m_r;
}

void
LwsnHeader::SetDid(uint16_t Did)
{

	m_Did = Did;
}

uint16_t 
LwsnHeader::GetDid(void) const
{
	return m_Did;
}

void 
LwsnHeader::SetDid2(uint16_t Did2)
{
	m_Did2 = Did2;
}

uint16_t 
LwsnHeader::GetDid2(void) const
{
	return m_Did2;
}

void 
LwsnHeader::SetStartTime(uint16_t time)
{
	m_StartTime = time;
}

uint16_t 
LwsnHeader::GetStartTime(void) const
{
	return m_StartTime;
}


void 
LwsnHeader::SetStartTime2(uint16_t time)
{
	m_StartTime2 = time;
}

uint16_t 
LwsnHeader::GetStartTime2(void) const
{
	return m_StartTime2;
}


bool
LwsnHeader::IsGAnc(void) const
{
	return (m_Type == G_ANC);
}
/*
bool
LwsnHeader::IsSensingData(void) const
{
	return (m_Type == SENSING_DATA);
}
*/

void
LwsnHeader::SetSource(Mac48Address source)
{
	m_source = source;
}

void 
LwsnHeader::SetDestination(Mac48Address destination)
{
	m_destination = destination;
}

Mac48Address
LwsnHeader::GetSource() const
{
	return m_source;
}

Mac48Address
LwsnHeader::GetDestination() const
{
	return m_destination;
}


	
}
