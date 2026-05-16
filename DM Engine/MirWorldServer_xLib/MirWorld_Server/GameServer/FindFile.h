#pragma once
class CFindFile
{
public:
	CFindFile(void) {}
	virtual ~CFindFile(void) {}
	BOOL StartFind(const char* pszPath, const char* pszFindName, BOOL bFindSubFolder = FALSE, UINT nParam = 0)
	{
		WIN32_FIND_DATA	wfd;
		HANDLE hFindFile = INVALID_HANDLE_VALUE;
		CHAR szFileName[1024];
		CHAR szExt[32];
		CHAR szExtTest[32];
		_makepath(szFileName, nullptr, pszPath, "*.*", nullptr);
		_splitpath(pszFindName, nullptr, nullptr, nullptr, szExt);
		hFindFile = FindFirstFile(szFileName, &wfd);
		if (hFindFile != INVALID_HANDLE_VALUE)
		{
			do {
				if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				{
					if (wfd.cFileName[0] != '.')
					{
						if (bFindSubFolder)
						{
							_makepath(szFileName, nullptr, pszPath, wfd.cFileName, nullptr);
							StartFind(szFileName, pszFindName, TRUE, nParam);
						}
					}
				}
				else
				{
					_splitpath(wfd.cFileName, nullptr, nullptr, nullptr, szExtTest);
					if (_stricmp(szExtTest, szExt) == 0)
					{
						_makepath(szFileName, nullptr, pszPath, wfd.cFileName, nullptr);
						OnFoundFile(szFileName, nParam);
					}
				}
			} while (FindNextFile(hFindFile, &wfd));
			FindClose(hFindFile);
		}
		return TRUE;
	}
protected:
	virtual VOID OnFoundFile(const char* pszFilename, UINT nParam = 0) {}
};