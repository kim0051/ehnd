#include "stdafx.h"
#include "filter.h"


filter::filter()
{
}


filter::~filter()
{
}

bool filter::pre_load()
{
	WCHAR lpEztPath[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	FILTERSTRUCT fs;
	wstring Path;

	DWORD dwStart, dwEnd;
	dwStart = GetTickCount();

	GetLoadPath(lpEztPath, MAX_PATH);
	Path = lpEztPath;
	Path += L"\\Ehnd\\PreFilter*.txt";

	int pre_line = 0;
	bool IsUnicode;
	PreFilter.clear();

	HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);
	
	do
	{
		if (hFind == INVALID_HANDLE_VALUE) break;
		else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			continue;

		if (!wcsncmp(FindFileData.cFileName, L"PreFilterU", 10))
			IsUnicode = true;
		else IsUnicode = false;

		Path = lpEztPath;
		Path += L"\\Ehnd\\";

		filter_load(PreFilter, Path.c_str(), FindFileData.cFileName, PREFILTER, IsUnicode, pre_line);

	} while (FindNextFile(hFind, &FindFileData));

	// 정렬
	sort(PreFilter.begin(), PreFilter.end());
	WriteLog(L"PreFilterRead : 필터 정렬을 완료했습니다.\n");

	// 소요시간 계산
	dwEnd = GetTickCount(); WriteLog(L"PreFilterRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
	return true;
}

bool filter::post_load()
{
	WCHAR lpEztPath[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	FILTERSTRUCT fs;
	wstring Path;

	DWORD dwStart, dwEnd;
	dwStart = GetTickCount();

	GetLoadPath(lpEztPath, MAX_PATH);
	Path = lpEztPath;
	Path += L"\\Ehnd\\PostFilter*.txt";

	int post_line = 0;
	bool IsUnicode;
	PostFilter.clear();

	HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

	do
	{
		if (hFind == INVALID_HANDLE_VALUE) break;
		else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			continue;

		if (!wcsncmp(FindFileData.cFileName, L"PostFilterU", 11))
			IsUnicode = true;
		else IsUnicode = false;

		Path = lpEztPath;
		Path += L"\\Ehnd\\";

		filter_load(PostFilter, Path.c_str(), FindFileData.cFileName, POSTFILTER, IsUnicode, post_line);

	} while (FindNextFile(hFind, &FindFileData));

	// 정렬
	sort(PostFilter.begin(), PostFilter.end());
	WriteLog(L"PostFilterRead : 필터 정렬을 완료했습니다.\n");

	// 소요시간 계산
	dwEnd = GetTickCount();
	WriteLog(L"PostFilterRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
	return true;
}

bool filter::userdic_load()
{
	WCHAR lpEztPath[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	wstring Path;

	DWORD dwStart, dwEnd;
	dwStart = GetTickCount();

	GetLoadPath(lpEztPath, MAX_PATH);
	Path = lpEztPath;
	Path += L"\\Ehnd\\UserDict*.txt";

	int userdic_line = 0;
	UserDic.clear();

	// load userdict.jk
	bool use_jkdic = true;
	if (use_jkdic) jkdic_load();
	HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

	do
	{
		if (hFind == INVALID_HANDLE_VALUE) break;
		else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			continue;

		Path = lpEztPath;
		Path += L"\\Ehnd\\";

		userdic_load2(Path.c_str(), FindFileData.cFileName, userdic_line);

	} while (FindNextFile(hFind, &FindFileData));

	// 정렬
	sort(UserDic.begin(), UserDic.end());
	WriteLog(L"UserDicRead : 사용자 사전 정렬을 완료했습니다.\n");

	// 소요시간 계산
	dwEnd = GetTickCount();
	WriteLog(L"UserDicRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);

	// 엔드 임시파일 삭제
	ehnddic_cleanup();

	// 엔드 임시파일 생성
	ehnddic_create();

	return true;
}

bool filter::jkdic_load()
{
	WCHAR lpEztPath[MAX_PATH], Buffer[1024];
	FILE *fp;
	wstring Path;

	DWORD dwStart, dwEnd;
	dwStart = GetTickCount();

	GetLoadPath(lpEztPath, MAX_PATH);
	Path = lpEztPath;
	Path += L"\\Dat\\UserDict.jk";

	if (_wfopen_s(&fp, Path.c_str(), L"rt,ccs=UTF-8") != 0)
	{
		WriteLog(L"JkDicLoad : DAT 사용자 사전 파일 \"UserDict.jk\"이 없습니다.\n");

		// userdict.jk 파일이 없으면 빈 파일 만듦
		if (_wfopen_s(&fp, Path.c_str(), L"wb") == 0) fclose(fp);
		return false;
	}

	//
	// ezTrans XP UserDict.jk Struct
	// [0] hidden (1 byte)
	// [1-31] jpn (31 bytes)
	// [32-62] kor (31 bytes)
	// [63-67] part of speech (5 bytes)
	// [68-109] attributes (42 bytes)
	//
	USERDICSTRUCT us;
	int vaild_line = 0;
	while (1)
	{
		if (!fread(Buffer, sizeof(char), 1, fp)) break;
		else
			(Buffer[0] == 0x00) ? us.hidden = false : us.hidden = true;

		if (!fread(Buffer, sizeof(char), 31, fp)) break;
		else
			memcpy(us.jpn, Buffer, 31);

		if (!fread(Buffer, sizeof(char), 31, fp)) break;
		else
			memcpy(us.kor, Buffer, 31);

		if (!fread(Buffer, sizeof(char), 5, fp)) break;
		else
			memcpy(us.part, Buffer, 5);

		if (!fread(Buffer, sizeof(char), 42, fp)) break;
		else
			memcpy(us.attr, Buffer, 42);

		UserDic.push_back(us);
		vaild_line++;
	}
	fclose(fp);

	WriteLog(L"JkDicRead : %d개의 DAT 사용자 사전 \"UserDict.jk\"를 읽었습니다.\n", vaild_line);

	// 소요시간 계산
	dwEnd = GetTickCount();
	WriteLog(L"JkDicRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
	return true;
}

bool filter::ehnddic_cleanup()
{
	WCHAR lpTmpPath[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	wstring Path;

	DWORD dwStart, dwEnd;
	dwStart = GetTickCount();

	GetTempPath(MAX_PATH, lpTmpPath);
	Path = lpTmpPath;
	Path += L"\\UserDict*.ehnd";

	HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

	do
	{
		WriteLog(L"EhndDicCleanUp : %s\n", FindFileData.cFileName);
		if (hFind == INVALID_HANDLE_VALUE) break;
		else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			continue;

		int i = wcslen(FindFileData.cFileName) - 1;
		while (i--)
		{
			if (FindFileData.cFileName[i] == L'.' && 
				!wcscmp(FindFileData.cFileName + i, L".ehnd"))
			{
				wstring DelFile = lpTmpPath;
				DelFile += FindFileData.cFileName;
				DeleteFile(DelFile.c_str());
				break;
			}
		}

	} while (FindNextFile(hFind, &FindFileData));

	// 소요시간 계산
	dwEnd = GetTickCount();
	WriteLog(L"EhndDicCleanup : --- Elasped Time : %dms ---\n", dwEnd - dwStart);

	return true;
}

bool filter::ehnddic_create()
{
	WCHAR lpTmpPath[MAX_PATH], lpText[12];
	wstring Path;
	FILE *fp;
	DWORD dwStart, dwEnd;
	_itow_s(g_initTick, lpText, 10);
	dwStart = GetTickCount();

	GetTempPath(MAX_PATH, lpTmpPath);
	Path = lpTmpPath;
	Path += L"\\UserDict_";
	Path += lpText;
	Path += L".ehnd";

	if (_wfopen_s(&fp, Path.c_str(), L"wb") != 0)
		WriteLog(L"EhndDicCreate : 사용자사전 바이너리 \"UserDict_%s.ehnd\" 파일을 생성하는데 실패했습니다.\n", lpText);

	for (UINT i = 0; i < UserDic.size(); i++)
	{
		fwrite(&UserDic[i].hidden, sizeof(char), 1, fp);
		fwrite(&UserDic[i].jpn, sizeof(char), 31, fp);
		fwrite(&UserDic[i].kor, sizeof(char), 31, fp);
		fwrite(&UserDic[i].part, sizeof(char), 5, fp);
		fwrite(&UserDic[i].attr, sizeof(char), 42, fp);
	}
	WriteLog(L"EhndDicCreate : 사용자사전 바이너리 \"UserDict_%s.ehnd\" 생성 완료.\n", lpText);
	fclose(fp);

	// 소요시간 계산
	dwEnd = GetTickCount();
	WriteLog(L"EhndDicCreate : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
	return true;
}

bool filter::skiplayer_load()
{
	WCHAR lpEztPath[MAX_PATH], Buffer[1024], Context[1024];
	wstring Path;
	FILE *fp;

	DWORD dwStart, dwEnd;
	dwStart = GetTickCount();

	GetLoadPath(lpEztPath, MAX_PATH);
	Path = lpEztPath;
	Path += L"\\Ehnd\\SkipLayer.txt";

	int vaild_line = 0;

	SkipLayer.clear();

	if (_wfopen_s(&fp, Path.c_str(), L"rt,ccs=UTF-8") != 0)
	{
		WriteLog(L"[SkipLayerRead] 스킵레이어 로드 실패.\n");
		return false;
	}
	WriteLog(L"SkipLayerRead : SkipLayer 로드.\n");
	
	for (int line = 0; fgetws(Buffer, 1000, fp) != NULL; line++)
	{
		int tab = 0;
		if (Buffer[0] == L'/' && Buffer[1] == L'/') continue;

		SKIPLAYERSTRUCT ss;
		ss.line = line;

		for (UINT i = 0, prev = 0; i < wcslen(Buffer) + 1; i++)
		{
			if (Buffer[i] == L'\t' || Buffer[i] == L'\n' || (Buffer[i] == L'/' && Buffer[i - 1] == L'/') || i == wcslen(Buffer))
			{
				switch (tab)
				{
				case 0:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					tab++;
					prev = i + 1;
					ss.wtype = Context;
					if (!wcsncmp(Context, L"PRE", 3)) ss.type = 1;
					else if (!wcsncmp(Context, L"POST", 4)) ss.type = 2;
					else i = wcslen(Buffer) + 1;
					break;
				case 1:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					tab++;
					prev = i + 1;
					ss.layer = _wtoi(Context);
					ss.wlayer = Context;
					break;
				case 2:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					tab++;
					prev = i + 1;
					ss.cond = Context;
					break;
				}
			}
		}
		if (tab < 2) continue;

		try
		{
			wregex ex(ss.cond);
		}
		catch (regex_error ex)
		{
			WCHAR lpWhat[255];
			int len = MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
			MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

			WriteLog(L"SkipLayerRead : 정규식 오류! : [%s:%d] %s | %s | %s\n", L"SkipLayer.txt", line, ss.wtype, ss.wlayer, ss.cond);

			continue;
		}
		vaild_line++;
		SkipLayer.push_back(ss);
	}
	fclose(fp);
	WriteLog(L"SkipLayerRead : %d개의 스킵레이어를 읽었습니다.\n", vaild_line);

	// 소요시간 계산
	dwEnd = GetTickCount();
	WriteLog(L"SkipLayerRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
	return true;
}

bool filter::filter_load(vector<FILTERSTRUCT> &Filter, LPCWSTR lpPath, LPCWSTR lpFileName, int FilterType, bool IsUnicode, int &g_line)
{
	FILE *fp;
	WCHAR Buffer[1024], Context[1024];

	wstring Path;
	Path = lpPath;
	Path += lpFileName;
	int vaild_line = 0;

	if (_wfopen_s(&fp, Path.c_str(), L"rt,ccs=UTF-8") != 0)
	{
		if (FilterType == PREFILTER && IsUnicode) WriteLog(L"PreFilterRead : 전처리 유니코드 전용 필터 '%s' 로드 실패!\n", lpFileName);
		else if (FilterType == PREFILTER && !IsUnicode) WriteLog(L"PreFilterRead : 전처리 필터 '%s' 로드 실패!\n", lpFileName);
		else if (FilterType == POSTFILTER && IsUnicode) WriteLog(L"PostFilterRead : 후처리 유니코드 전용 필터 '%s' 로드 실패!\n", lpFileName);
		else if (FilterType == POSTFILTER && !IsUnicode) WriteLog(L"PostFilterRead : 후처리 필터 '%s' 로드 실패!\n", lpFileName);
		return false;
	}

	if (FilterType == PREFILTER && IsUnicode) WriteLog(L"PreFilterRead : 전처리 유니코드 전용 필터 \"%s\" 로드.\n", lpFileName);
	if (FilterType == PREFILTER && !IsUnicode) WriteLog(L"PreFilterRead : 전처리 필터 \"%s\" 로드.\n", lpFileName);
	else if (FilterType == POSTFILTER && IsUnicode) WriteLog(L"PostFilterRead : 후처리 유니코드 전용 필터 \"%s\" 로드.\n", lpFileName);
	else if (FilterType == POSTFILTER && !IsUnicode) WriteLog(L"PostFilterRead : 후처리 필터 \"%s\" 로드.\n", lpFileName);

	for (int line = 0; fgetws(Buffer, 1000, fp) != NULL; line++, g_line++)
	{
		if (Buffer[0] == L'/' && Buffer[1] == L'/') continue;	// 주석

		FILTERSTRUCT fs;
		fs.g_line = g_line;
		fs.line = line;
		fs.db = lpFileName;
		fs.unicode = IsUnicode;
		fs.src = L"";
		fs.dest = L"";
		
		int tab = 0;
		for (UINT i = 0, prev = 0; i < wcslen(Buffer); i++)
		{
			if (Buffer[i] == L'\t' || Buffer[i] == L'\n' || (Buffer[i] == L'/' && Buffer[i - 1] == L'/') || i == wcslen(Buffer))
			{
				switch (tab)
				{
				case 0:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					fs.src = Context;
					break;
				case 1:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					fs.dest = Context;
					break;
				case 2:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					fs.layer = _wtoi(Context);
					break;
				case 3:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					fs.regex = _wtoi(Context);
					break;
				}
				if (Buffer[i] == L'/' && Buffer[i - 1] == L'/') break;
			}
		}

		if (tab < 3) continue;
		if (fs.regex == 1)
		{
			try
			{
				wregex ex(fs.src);
			}
			catch (regex_error ex)
			{
				WCHAR lpWhat[255];
				int len = MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
				MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

				if (FilterType == 1) WriteLog(L"PreFilterRead : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n", lpFileName, line, fs.src.c_str(), fs.dest.c_str(), fs.layer, fs.regex);
				else if (FilterType == 2) WriteLog(L"PostFilterRead : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n", lpFileName, line, fs.src.c_str(), fs.dest.c_str(), fs.layer, fs.regex);
				continue;
			}
		}
		vaild_line++;
		Filter.push_back(fs);
	}
	fclose(fp);
	if (FilterType == PREFILTER && IsUnicode) WriteLog(L"PreFilterRead : %d개의 유니코드 전용 전처리 필터 \"%s\"를 읽었습니다.\n", vaild_line, lpFileName);
	if (FilterType == PREFILTER && !IsUnicode) WriteLog(L"PreFilterRead : %d개의 전처리 필터 \"%s\"를 읽었습니다.\n", vaild_line, lpFileName);
	else if (FilterType == POSTFILTER && IsUnicode) WriteLog(L"PostFilterRead : %d개의 유니코드 전용 후처리 필터 \"%s\"를 읽었습니다.\n", vaild_line, lpFileName);
	else if (FilterType == POSTFILTER && !IsUnicode) WriteLog(L"PostFilterRead : %d개의 후처리 필터 \"%s\"를 읽었습니다.\n", vaild_line, lpFileName);
	return true;
}

bool filter::userdic_load2(LPCWSTR lpPath, LPCWSTR lpFileName, int &g_line)
{
	FILE *fp;
	WCHAR Buffer[1024], Context[1024];
	char szBuffer[128];
	wstring Path, Jpn, Kor, Part, Attr;
	int vaild_line = 0;
	Path = lpPath;
	Path += lpFileName;

	if (_wfopen_s(&fp, Path.c_str(), L"rt,ccs=UTF-8") != 0)
	{
		WriteLog(L"UserDicRead : 사용자 사전 '%s' 로드 실패!\n", lpFileName);
		return false;
	}

	WriteLog(L"UserDicRead : 사용자 사전 \"%s\" 로드.\n", lpFileName);

	for (int line = 0; fgetws(Buffer, 1000, fp) != NULL; line++, g_line++)
	{
		if (Buffer[0] == L'/' && Buffer[1] == L'/') continue;	// 주석

		USERDICSTRUCT us;
		us.hidden = 0x00;
		memset(us.jpn, 0, sizeof(us.jpn));
		memset(us.kor, 0, sizeof(us.kor));
		memset(us.part, 0, sizeof(us.part));
		memset(us.attr, 0, sizeof(us.attr));

		int tab = 0;
		for (UINT i = 0, prev = 0; i < wcslen(Buffer); i++)
		{
			if (Buffer[i] == L'\t' || Buffer[i] == L'\n' || (Buffer[i] == L'/' && Buffer[i - 1] == L'/') || i == wcslen(Buffer))
			{
				switch (tab)
				{
				case 0:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					Jpn = Context;
					break;
				case 1:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					Kor = Context;
					break;
				case 2:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					if (_wtoi(Context))
						Part = L"I110";
					else
						Part = L"A9D0";
					break;
				case 3:
					wcsncpy_s(Context, Buffer + prev, i - prev);
					prev = i + 1;
					tab++;
					Attr = Context;
					break;
				}
				if (Buffer[i] == L'/' && Buffer[i - 1] == L'/') break;
			}
		}

		if (tab < 3) continue;
		int len;

		if ((len = WideCharToMultiByte(932, 0, Jpn.c_str(), -1, NULL, NULL, NULL, NULL)) > 31)
		{
			WriteLog(L"UserDicRead : 오류 : 원문 단어의 길이는 15자(30Byte)를 초과할 수 없습니다.\n");
			WriteLog(L"UserDicRead : 오류 : [%s:%d] : <<%s>> | %s | %s | %s\n", lpFileName, line, Jpn.c_str(), Kor.c_str(), Part.c_str(), Attr.c_str());
		}
		WideCharToMultiByte(932, 0, Jpn.c_str(), -1, szBuffer, len, NULL, NULL);
		strcpy_s(us.jpn, szBuffer);

		if ((len = WideCharToMultiByte(949, 0, Kor.c_str(), -1, NULL, NULL, NULL, NULL)) > 31)
		{
			WriteLog(L"UserDicRead : 오류 : 역문 단어의 길이는 15자(30Byte)를 초과할 수 없습니다.\n");
			WriteLog(L"UserDicRead : 오류 : [%s:%d] : %s | <<%s>> | %s | %s\n", lpFileName, line, Jpn.c_str(), Kor.c_str(), Part.c_str(), Attr.c_str());
		}
		WideCharToMultiByte(949, 0, Kor.c_str(), -1, szBuffer, len, NULL, NULL);
		strcpy_s(us.kor, szBuffer);

		if ((len = WideCharToMultiByte(932, 0, Attr.c_str(), -1, NULL, NULL, NULL, NULL)) > 31)
		{
			WriteLog(L"UserDicRead : 오류 :  단어 속성은 42Byte를 초과할 수 없습니다.\n");
			WriteLog(L"UserDicRead : 오류 : [%s:%d] : %s | %s | %s | <<%s>>\n", lpFileName, line, Jpn.c_str(), Kor.c_str(), Part.c_str(), Attr.c_str());
		}
		WideCharToMultiByte(932, 0, Attr.c_str(), -1, szBuffer, len, NULL, NULL);
		strcpy_s(us.attr, szBuffer);

		len = WideCharToMultiByte(932, 0, Part.c_str(), -1, NULL, NULL, NULL, NULL);
		WideCharToMultiByte(932, 0, Part.c_str(), -1, szBuffer, len, NULL, NULL);
		strcpy_s(us.part, szBuffer);

		vaild_line++;
		UserDic.push_back(us);
	}
	WriteLog(L"UserDictRead : %d개의 사용자 사전 \"%s\"를 읽었습니다.\n", vaild_line, lpFileName);
	fclose(fp);
	return true;
}
bool filter::pre(wstring &wsText)
{
	if (g_PreUsable == false)
	{
		WriteLog(L"PreFilter : 전처리가 꺼져 있습니다.\n");
		return false;
	}
	return filter_proc(PreFilter, PREFILTER, wsText);
}

bool filter::post(wstring &wsText)
{
	if (g_PostUsable == false)
	{
		WriteLog(L"PostFilter : 후처리가 꺼져 있습니다.\n");
		return false;
	}
	return filter_proc(PostFilter, POSTFILTER, wsText);
}

bool filter::filter_proc(vector<FILTERSTRUCT> &Filter, const int FilterType, wstring &wsText)
{
	DWORD dwStart, dwEnd;
	bool g_UnicodeMode = false;
	wstring Str = wsText;
	int layer_prev = -1;
	bool layer_pass = false;
	bool pass_once = false;

	wstring pass_log;
	dwStart = GetTickCount();

	for (UINT i = 0; i < Filter.size(); i++)
	{
		// 유니코드 모드일떄 유니코드 전용 필터 사용 가능
		if (Filter[i].unicode && !g_UnicodeMode) continue;

		// 저장된 차수와 현재 필터의 차수가 다를 때 조건식을 돌려 PASS 여부 확인
		if (Filter[i].layer != layer_prev)
		{
			layer_prev = Filter[i].layer;
			layer_pass = false;

			for (UINT j = 0; j < SkipLayer.size(); j++)
			{
				if (SkipLayer[j].type == FilterType &&
					SkipLayer[j].layer == Filter[i].layer)
				{
					try
					{
						wregex ex(SkipLayer[j].cond);

						if (!regex_search(wsText, ex))
						{
							layer_pass = true;
							pass_once = true;
							pass_log += (SkipLayer[j].wlayer + L", ");
							break;
						}
					}
					catch (regex_error ex)
					{
						WCHAR lpWhat[255];
						int len = MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
						MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

						WriteLog(L"SkipLayerRead : 정규식 오류! : [%s:%d] %s | %d | %s\n", L"SkipLayer.txt", SkipLayer[i].line, SkipLayer[i].wtype.c_str(), SkipLayer[i].layer, SkipLayer[i].cond.c_str());
						continue;
					}
				}

			}
		}
		if (Filter[i].layer == layer_prev && layer_pass) continue;

		if (!Filter[i].regex)
		{
			wsText = replace_all(wsText, Filter[i].src, Filter[i].dest);
			if (Str.compare(wsText))
			{
				if (FilterType == PREFILTER) WriteLog(L"PreFilter : [%s:%d] | %s | %s | %d | %d\n", Filter[i].db.c_str(), Filter[i].line, Filter[i].src.c_str(), Filter[i].dest.c_str(), Filter[i].layer, Filter[i].regex);
				else if (FilterType == POSTFILTER) WriteLog(L"PostFIlter : [%s:%d] | %s | %s | %d | %d\n", Filter[i].db.c_str(), Filter[i].line, Filter[i].src.c_str(), Filter[i].dest.c_str(), Filter[i].layer, Filter[i].regex);
			}
				
		}
		else
		{
			try
			{
				wregex ex(Filter[i].src);

				if (regex_search(wsText, ex))
				{
					wsText = regex_replace(wsText, ex, Filter[i].dest, regex_constants::match_default);
				}
			}
			catch (regex_error ex)
			{
				WCHAR lpWhat[255];
				int len = MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
				MultiByteToWideChar(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

				if (FilterType == PREFILTER) WriteLog(L"PreFilter : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n", Filter[i].db.c_str(), Filter[i].line, Filter[i].src.c_str(), Filter[i].dest.c_str(), Filter[i].layer, Filter[i].regex);
				else if (FilterType == POSTFILTER) WriteLog(L"PostFilter : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n", Filter[i].db.c_str(), Filter[i].line, Filter[i].src.c_str(), Filter[i].dest.c_str(), Filter[i].layer, Filter[i].regex);
				continue;
			}

			if (Str != wsText)
			{
				if (FilterType == PREFILTER) WriteLog(L"PreFilter : [%s:%d] %s | %s | %d | %d\n", Filter[i].db.c_str(), Filter[i].line, Filter[i].src.c_str(), Filter[i].dest.c_str(), Filter[i].layer, Filter[i].regex);
				else if (FilterType == POSTFILTER) WriteLog(L"PostFilter : [%s:%d] %s | %s | %d | %d\n", Filter[i].db.c_str(), Filter[i].line, Filter[i].src.c_str(), Filter[i].dest.c_str(), Filter[i].layer, Filter[i].regex);
				Str = wsText;
			}
		}
	}

	pass_log = pass_log.substr(0, pass_log.rfind(L", "));

	if (pass_once)
	{
		if (FilterType == PREFILTER) WriteLog(L"PreSkipLayer : %s\n", pass_log.c_str());
		else if (FilterType == POSTFILTER) WriteLog(L"PostSkipLayer : %s\n", pass_log.c_str());
	}

	dwEnd = GetTickCount();
	if (FilterType == PREFILTER) WriteLog(L"PreFilter : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
	else if (FilterType == POSTFILTER) WriteLog(L"PostFIlter : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
	return true;
}

bool filter::cmd(wstring &wsText)
{
	if (wsText[0] != L'/') return false;
	if (!wsText.compare(L"/ver") || !wsText.compare(L"/version"))
	{
		wsText = L"Ehnd ";
		wsText += WIDEN(EHND_VER);
		wsText += L", ";
		wsText += WIDEN(__DATE__);
		wsText += L", ";
		wsText += WIDEN(__TIME__);
		wsText += L"\r\n";

		SetLogText(wsText.c_str(), RGB(168, 25, 25), RGB(255, 255, 255));
		return true;
	}
	else if (!wsText.compare(L"/log"))
	{
		wsText = L"Log Window.\r\n";
		ShowLogWin(!IsShownLogWin());
		return true;
	}
	else if (!wsText.compare(L"/pre"))
	{
		if (g_PreUsable)
		{
			g_PreUsable = false;
			wsText = L"전처리 끔.";
		}
		else
		{
			g_PreUsable = true;
			wsText = L"전처리 켬.";
		}
	}
	else if (!wsText.compare(L"/post"))
	{
		if (g_PostUsable)
		{
			g_PostUsable = false;
			wsText = L"후처리 끔.";
		}
		else
		{
			g_PostUsable = true;
			wsText = L"후처리 켬.";
		}
	}
	return false;
}

wstring filter::replace_all(const wstring &str, const wstring &pattern, const wstring &replace)
{
	wstring result = str;
	wstring::size_type pos = 0, offset = 0;

	while ((pos = result.find(pattern, offset)) != wstring::npos)
	{
		result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);
		offset = pos + replace.size();
	}
	return result;
}