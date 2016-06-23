#pragma once

#include "../Base/Base.h"

namespace OOX
{
	class CPath
	{
	public:
		CString	m_strFilename;
		//флаг введен, чтобы отличать относительные и абсолютные пути в rels
		bool m_bIsRoot;

	public:
		CPath();
		CPath(const CString& sName, bool bIsNorm = true);
		CPath(LPCSTR& sName, bool bIsNorm = true);
		CPath(LPCWSTR& sName, bool bIsNorm = true);

		CPath(const CPath& oSrc);

		CPath& operator=(const CPath& oSrc);
		CPath& operator=(const CString& oSrc);
        CPath& operator=(LPCSTR oSrc);
        CPath& operator=(LPCWSTR oSrc);

		friend CPath operator/(const CPath& path1, const CPath& path2)
		{
			CPath path(path1.m_strFilename + FILE_SEPARATOR_STR + path2.m_strFilename);
			path.Normalize();

			return path;
		}
		friend CPath operator/(const CPath& path1, const CString& path2)
		{
			CPath path(path1.m_strFilename + FILE_SEPARATOR_STR + path2);
			path.Normalize();

			return path;
		}

		friend CPath operator+(const CPath& path1, const CPath& path2)
		{
			CPath path(path1.m_strFilename + path2.m_strFilename);
			path.Normalize();
			return path;
		}
		friend CPath operator+(const CPath& path1, const CString& path2)
		{
			CPath path(path1.m_strFilename + path2);
			path.Normalize();
			return path;
		}
		friend CPath operator+(const CString& path1, const CPath& path2)
		{
			CPath path(path1 + path2.m_strFilename);
			path.Normalize();
			return path;
		}

        AVSINLINE CString GetExtention(bool bIsPoint = true) const
        {
            int nFind = m_strFilename.ReverseFind('.');
            if (-1 == nFind)
                return _T("");

            if (!bIsPoint)
                ++nFind;

            return m_strFilename.Mid(nFind);
        }
        AVSINLINE CString GetDirectory(bool bIsSlash = true) const
        {
                int nPos = m_strFilename.ReverseFind(FILE_SEPARATOR_CHAR);
                if (-1 == nPos)
                {
                    return m_strFilename;
                }
                else
                {
                    if (bIsSlash)
                        ++nPos;
                    return m_strFilename.Mid(0, nPos);
                }
        }
        AVSINLINE CString GetPath() const
        {
            return m_strFilename;
        }
        AVSINLINE CString GetFilename() const
		{
            int nPos = m_strFilename.ReverseFind(FILE_SEPARATOR_CHAR);
			if (-1 == nPos)
			{
				return m_strFilename;
			}
			else
			{
				int nLast = (int) m_strFilename.GetLength();
				return m_strFilename.Mid(nPos + 1, nLast);
			}
		}

#if defined(_WIN32) || defined (_WIN64)
		AVSINLINE void Normalize()
		{
			if (0 == m_strFilename.GetLength())
				return;

			TCHAR* pData = m_strFilename.GetBuffer();
			int nLen = m_strFilename.GetLength();

			TCHAR* pDataNorm = new TCHAR[nLen + 1];
			int* pSlashPoints = new int[nLen + 1];

			int nStart = 0;
			int nCurrent = 0;
			int nCurrentSlash = -1;
			int nCurrentW = 0;
			bool bIsUp = false;

			while (nCurrent < nLen)
			{
                if (pData[nCurrent] == (TCHAR) '\\' || pData[nCurrent] == (TCHAR)'/')
				{
					if (nStart < nCurrent)
					{
						bIsUp = false;
						if ((nCurrent - nStart) == 2)
						{
							if (pData[nStart] == (TCHAR)'.' && pData[nStart + 1] == (TCHAR)'.')
							{
								if (nCurrentSlash > 0)
								{
									--nCurrentSlash;
									nCurrentW = pSlashPoints[nCurrentSlash];
									bIsUp = true;
								}
							}
						}
						if (!bIsUp)
						{
                            pDataNorm[nCurrentW++] = (TCHAR) FILE_SEPARATOR_CHAR;
							++nCurrentSlash;
							pSlashPoints[nCurrentSlash] = nCurrentW;
						}
					}
					nStart = nCurrent + 1;					
					++nCurrent;
					continue;
				}
				pDataNorm[nCurrentW++] = pData[nCurrent];
				++nCurrent;
			}

			pDataNorm[nCurrentW] = (TCHAR)'\0';

			m_strFilename.ReleaseBuffer();
			m_strFilename = CString(pDataNorm, nCurrentW);

			delete []pSlashPoints;
			delete []pDataNorm;				
		}
		void CheckIsRoot()
		{
			if(m_strFilename.GetLength() > 0 && ('/' == m_strFilename[0] || '\\' == m_strFilename[0]))
				m_bIsRoot = true;
			else
				m_bIsRoot = false;
		}
#else
		AVSINLINE void Normalize()
		{
			if (0 == m_strFilename.GetLength())
				return;

			TCHAR* pData = m_strFilename.GetBuffer();
			int nLen = m_strFilename.GetLength();

			TCHAR* pDataNorm = new TCHAR[nLen + 1];
			int* pSlashPoints = new int[nLen + 1];

			int nStart = 0;
			int nCurrent = 0;
			int nCurrentSlash = -1;
			int nCurrentW = 0;
			bool bIsUp = false;

			if (pData[nCurrent] == (TCHAR)'/')
			   pDataNorm[nCurrentW++] = (TCHAR) FILE_SEPARATOR_CHAR;

			while (nCurrent < nLen)
			{
                if (pData[nCurrent] == (TCHAR)'/')
				{
					if (nStart < nCurrent)
					{
						bIsUp = false;
						if ((nCurrent - nStart) == 2)
						{
							if (pData[nStart] == (TCHAR)'.' && pData[nStart + 1] == (TCHAR)'.')
							{
								if (nCurrentSlash > 0)
								{
									--nCurrentSlash;
									nCurrentW = pSlashPoints[nCurrentSlash];
									bIsUp = true;
								}
							}
						}
						if (!bIsUp)
						{
                            pDataNorm[nCurrentW++] = (TCHAR) FILE_SEPARATOR_CHAR;
							++nCurrentSlash;
							pSlashPoints[nCurrentSlash] = nCurrentW;
						}
					}
					nStart = nCurrent + 1;					
					++nCurrent;
					continue;
				}
				pDataNorm[nCurrentW++] = pData[nCurrent];
				++nCurrent;
			}

			pDataNorm[nCurrentW] = (TCHAR)'\0';

			m_strFilename.ReleaseBuffer();
			m_strFilename = CString(pDataNorm, nCurrentW);

			delete []pSlashPoints;
			delete []pDataNorm;				
		}
		void CheckIsRoot()
		{
			if(m_strFilename.GetLength() > 0 && ( '/' == m_strFilename[0] ))
				m_bIsRoot = true;
			else
				m_bIsRoot = false;
		}

#endif
		void SetName(CString sName, bool bNormalize)
		{
			m_strFilename = sName;
			CheckIsRoot();
			if(bNormalize)
				Normalize();
		}

		bool GetIsRoot()
		{
			return m_bIsRoot;
		}
	};

	class CSystemUtility
	{
	public:
        static bool CreateFile(const CString& strFileName);
		static bool IsFileExist(const CString& strFileName);
		static bool IsFileExist(const CPath& sPath);
		static CString GetDirectoryName(const CString& strFileName);
        static int GetFilesCount(const CString& strDirPath, const bool& bRecursive = false);
		static CString GetFileExtention(const CString& strFileName);
		static bool CreateDirectories(const CPath& oPath);
		static void ReplaceExtention(CString& strName, CString& str1, CString& str2);
	};
}
