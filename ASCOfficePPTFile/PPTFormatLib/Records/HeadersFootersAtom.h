#pragma once
#include "../Reader/Records.h"
#include "CString.h"

class CRecordHeadersFootersAtom : public CUnknownRecord
{
public:
	WORD m_nFormatID;
	
	bool m_bHasDate;
	bool m_bHasTodayDate;
	bool m_bHasUserDate;
	bool m_bHasSlideNumber;
	bool m_bHasHeader;
	bool m_bHasFooter;

	CRecordHeadersFootersAtom()
	{
	}

	~CRecordHeadersFootersAtom()
	{
	}

	virtual void ReadFromStream(SRecordHeader & oHeader, POLE::Stream* pStream)
	{
		m_oHeader = oHeader;

		m_nFormatID = StreamUtils::ReadWORD(pStream);

		BYTE nFlag			= StreamUtils::ReadBYTE(pStream);
		m_bHasDate			= ((nFlag & 0x01) == 0x01);
		m_bHasTodayDate		= ((nFlag & 0x02) == 0x02);
		m_bHasUserDate		= ((nFlag & 0x04) == 0x04);
		m_bHasSlideNumber	= ((nFlag & 0x08) == 0x08);
		m_bHasHeader		= ((nFlag & 0x10) == 0x10);
		m_bHasFooter		= ((nFlag & 0x20) == 0x20);

		StreamUtils::ReadBYTE(pStream);//reserved
	}
};


class CRecordRoundTripHeaderFooterDefaults12Atom : public CUnknownRecord
{
public:
	WORD m_nFormatID;
	
	bool m_bIncludeDate;
	bool m_bIncludeFooter;
	bool m_bIncludeHeader;
	bool m_bIncludeSlideNumber;

	CRecordRoundTripHeaderFooterDefaults12Atom()
	{
	}

	~CRecordRoundTripHeaderFooterDefaults12Atom()
	{
	}

	virtual void ReadFromStream(SRecordHeader & oHeader, POLE::Stream* pStream)
	{
		m_oHeader = oHeader;

		BYTE nFlag				= StreamUtils::ReadBYTE(pStream);

		m_bIncludeDate			= ((nFlag & 0x04) == 0x04);
		m_bIncludeFooter		= ((nFlag & 0x08) == 0x08);
		m_bIncludeHeader		= ((nFlag & 0x10) == 0x10);
		m_bIncludeSlideNumber	= ((nFlag & 0x20) == 0x20);
	}
};

class CRecordMetaCharacterAtom : public CUnknownRecord
{
public:
	DWORD m_nPosition;


	CRecordMetaCharacterAtom()
	{
		m_nPosition = -1;
	}

	~CRecordMetaCharacterAtom()
	{
	}

	virtual void ReadFromStream(SRecordHeader & oHeader, POLE::Stream* pStream)
	{
		m_oHeader = oHeader;

		if (oHeader.RecLen >=4)
		{
			m_nPosition =  StreamUtils::ReadDWORD(pStream);
		}
	}
};

class CRecordGenericDateMetaAtom : public CRecordMetaCharacterAtom
{
};

class CRecordRTFDateTimeMetaAtom : public CRecordGenericDateMetaAtom
{
public:
	std::string m_strFormat;
	
	virtual void ReadFromStream(SRecordHeader & oHeader, POLE::Stream* pStream)
	{
		CRecordGenericDateMetaAtom::ReadFromStream(oHeader, pStream);

		m_strFormat = StreamUtils::ReadStringA(pStream, 128);
	}
};

class CRecordDateTimeMetaAtom : public CRecordGenericDateMetaAtom
{
public:
	DWORD m_FormatID;
	
	virtual void ReadFromStream(SRecordHeader & oHeader, POLE::Stream* pStream)
	{
		int lPosition = pStream->tell();

		CRecordGenericDateMetaAtom::ReadFromStream(oHeader, pStream);

		if (oHeader.RecLen == 8)
		{
			m_FormatID =  StreamUtils::ReadDWORD(pStream);
		}
		else if (oHeader.RecLen >4)
		{
			m_FormatID = StreamUtils::ReadBYTE(pStream);
			StreamUtils::StreamSeek(lPosition + m_oHeader.RecLen, pStream);
		}
	}
};


class CRecordFooterMetaAtom : public CRecordMetaCharacterAtom
{
};

class CRecordHeaderMetaAtom : public CRecordMetaCharacterAtom
{
};

class CRecordSlideNumberMetaAtom : public CRecordMetaCharacterAtom
{
};

class CRecordHeadersFootersContainer : public CRecordsContainer
{
public:
	std::vector<std::wstring> m_HeadersFootersString[3]; //0-dates, 1 - headers, 2 - footers


	CRecordHeadersFootersAtom *m_oHeadersFootersAtom;

	CRecordHeadersFootersContainer()
	{
		m_oHeadersFootersAtom = NULL;
	}

	virtual void ReadFromStream(SRecordHeader & oHeader, POLE::Stream* pStream)
	{
		CRecordsContainer::ReadFromStream(oHeader, pStream);

		for (int i = 0 ; i < m_arRecords.size(); i++)
		{
			switch(m_arRecords[i]->m_oHeader.RecType)
			{
			case 0x0FDA:
				m_oHeadersFootersAtom = dynamic_cast<CRecordHeadersFootersAtom *>(m_arRecords[i]);
				break;
			case 0xFBA:
				{
					CRecordCString *str = dynamic_cast<CRecordCString *>(m_arRecords[i]);
					switch(m_arRecords[i]->m_oHeader.RecInstance)
					{
					case 0x000: m_HeadersFootersString[0].push_back(str->m_strText);	break;
					case 0x001: m_HeadersFootersString[1].push_back(str->m_strText);	break;
					case 0x002: m_HeadersFootersString[2].push_back(str->m_strText);	break;
					}
				}break;
			}
		}

	}
};
