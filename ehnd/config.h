#pragma once
class config
{
private:
	bool firstInit = true;

	bool cfg_prefilter_switch = true;
	bool cfg_postfilter_switch = true;
	bool cfg_userdic_switch = true;
	bool cfg_jkdic_switch = true;
	bool cfg_ehndwatch_switch = true;
	bool cfg_command_switch = true;
	bool cfg_log_detail = true;
	bool cfg_log_time = true;
	bool cfg_log_skiplayer = true;
	bool cfg_log_userdic = true;
	bool cfg_filelog_switch = false;
	int cfg_filelog_size = 300;
	bool cfg_filelog_eztrans_loc = true;
	bool cfg_filelog_startup_clear = true;
	bool cfg_console_switch = true;
	int cfg_console_maxline = 300;
	wchar_t cfg_console_fontname[255];
	int cfg_console_fontsize = 12;
public:
	config();
	~config();
	bool LoadConfig();
	bool SaveConfig();

	bool ReadINI(const wchar_t *key, const wchar_t *section, wchar_t *buf, wchar_t *file);
	bool WriteINI(const wchar_t *key, const wchar_t *section, wchar_t *buf, wchar_t *file);

	bool GetPreSwitch() { return cfg_prefilter_switch; }
	void SetPreSwitch(bool b) { cfg_prefilter_switch = b; }

	bool GetPostSwitch() { return cfg_postfilter_switch; }
	void SetPostSwitch(bool b) { cfg_postfilter_switch = b; }

	bool GetJKDICSwitch() { return cfg_jkdic_switch; }
	void SetJKDICSwitch(bool b) { cfg_jkdic_switch = b; }

	bool GetUserDicSwitch() { return cfg_userdic_switch; }
	void SetUserDicSwitch(bool b) { cfg_userdic_switch = b; }

	bool GetEhndWatchSwitch() { return cfg_ehndwatch_switch; }
	void SetEhndWatchSwitch(bool b) { cfg_ehndwatch_switch = b; }

	bool GetCommandSwitch() { return cfg_command_switch; }
	void SetCommandSwitch(bool b) { cfg_command_switch = b; }

	bool GetLogDetail() { return cfg_log_detail; }
	void SetLogDetail(bool b) { cfg_log_detail = b; }

	bool GetLogTime() { return cfg_log_time; }
	void SetLogTime(bool b) { cfg_log_time = b; }

	bool GetLogSkipLayer() { return cfg_log_skiplayer; }
	void SetLogSkipLayer(bool b) { cfg_log_skiplayer = b; }

	bool GetLogUserDic() { return cfg_log_userdic; }
	void SetLogUserDic(bool b) { cfg_log_userdic = b; }

	bool GetFileLogSwitch() { return cfg_filelog_switch; }
	void SetFileLogSwitch(bool b) { cfg_filelog_switch = b; }

	int GetFileLogSize() { return cfg_filelog_size; }
	void SetFileLogSize(int n) { cfg_filelog_size = n; }

	bool GetFileLogEztLoc() { return cfg_filelog_eztrans_loc; }
	void SetFileLogEztLoc(bool b) { cfg_filelog_eztrans_loc = b; }

	bool GetFileLogStartupClear() { return cfg_filelog_startup_clear; }
	void SetFileLogStartupClear(bool b) { cfg_filelog_startup_clear = b; }

	bool GetConsoleSwitch() { return cfg_console_switch; }
	void SetConsoleSwitch(bool b) { cfg_console_switch = b; ShowLogWin(b); }

	wchar_t *GetConsoleFontName() { return cfg_console_fontname; }
	void SetConsoleFontName(wchar_t *str) { wcscpy_s(cfg_console_fontname, str); }

	int GetConsoleMaxLine() { return cfg_console_maxline; }
	void SetConsoleMaxLine(int n) { cfg_console_maxline = n; }

	int GetConsoleFontSize() { return cfg_console_fontsize; }
	void SetConsoleFontSize(int n) { cfg_console_fontsize = n; }

};

