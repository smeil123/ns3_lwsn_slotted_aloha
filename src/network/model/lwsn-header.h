#ifndef LWSN_HEADER_H
#define LWSN_HEADER_H

#include <ns3/header.h>
#include <ns3/mac48-address.h>

namespace ns3{

class LwsnHeader:public Header
{
public:
    enum LwsnType
    {
    	G_ANC = 0,
    	ORIGINAL_TRANSMISSION = 1,
    	FORWARDING = 2,
    	IACK = 3,
    	NETWORK_CODING = 4
    };

	LwsnHeader ();
	virtual ~LwsnHeader();

	void SetType(enum LwsnType lwsnType);
	enum LwsnType GetType(void) const;

	void SetOsid(uint16_t Osid);
	uint16_t GetOsid(void) const;

	void SetOsid2(uint16_t Osid2);
	uint16_t GetOsid2(void) const;

	void SetPsid(uint16_t Psid);
	uint16_t GetPsid(void) const;

	void SetE(uint16_t e);
	uint16_t GetE(void) const;

	void SetR(uint16_t r);
	uint16_t GetR(void) const;

	void SetDid(uint16_t Did);
	uint16_t GetDid(void) const;

	void SetDid2(uint16_t Did2);
	uint16_t GetDid2(void) const;

	void SetStartTime(uint16_t time);
	uint16_t GetStartTime(void) const;

	void SetStartTime2(uint16_t time);
	uint16_t GetStartTime2(void) const;

	void SetSource(Mac48Address source);
	void SetDestination(Mac48Address destination);

	bool IsGAnc (void) const;
//	bool IsSensingData (void) const;

	Mac48Address GetSource() const;
	Mac48Address GetDestination() const;

	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual void Print(std::ostream &os) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);
	virtual uint32_t GetSerializedSize(void) const;

private:
	uint16_t m_Type;
	uint16_t m_Osid;
	uint16_t m_Osid2;
	uint16_t m_Psid;
	uint16_t m_r;
	uint16_t m_e;

	uint16_t m_Did;
	uint16_t m_Did2;
	uint16_t m_StartTime;
	uint16_t m_StartTime2;
	Mac48Address m_source;
	Mac48Address m_destination;






};


}

#endif
