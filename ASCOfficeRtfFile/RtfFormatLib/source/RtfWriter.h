#pragma once

#include "../../../Common/FileWriter.h"

#include "UniversalConverterUtils.h"

class RtfDocument;

class RtfWriter
{
public:
	
	CString m_sTempFolder;
	std::vector<CString> m_aTempFiles;
	std::vector<CString> m_aTempFilesSectPr;

	RtfWriter( RtfDocument& oDocument , std::wstring sFilename, std::wstring sFolder ):m_oDocument(oDocument)
	{
		m_sFilename = std_string2string(sFilename);
		m_sTempFolder = std_string2string(sFolder);
		m_bFirst = true;
		m_oCurTempFileWriter = NULL;
		m_oCurTempFileSectWriter = NULL;
	}
	~RtfWriter()
	{
		RELEASEOBJECT( m_oCurTempFileWriter );
		RELEASEOBJECT( m_oCurTempFileSectWriter );
		for( int i = 0; i < (int)m_aTempFiles.size(); i++ )
			Utils::RemoveDirOrFile( m_aTempFiles[i] );
		for( int i = 0; i < (int)m_aTempFilesSectPr.size(); i++ )
			Utils::RemoveDirOrFile( m_aTempFilesSectPr[i] );
		m_aTempFiles.clear();
	}
	bool Save();
	bool SaveByItemStart();
	bool SaveByItem();
	bool SaveByItemEnd();
private: 
	RtfDocument& m_oDocument;
	CString m_sFilename;

	bool m_bFirst;
	int GetCount();
	NFileWriter::CBufferedFileWriter* m_oCurTempFileWriter;
	NFileWriter::CBufferedFileWriter* m_oCurTempFileSectWriter;

	CString CreateRtfStart();
	CString CreateRtfEnd( );
};
