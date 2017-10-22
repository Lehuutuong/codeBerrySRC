void	UpdateAnglerHistory(PANGLERSTATE pOrimState);
void	GetAnglerState(STATEINFO* pStateInfo);
HISTORYINFO* GetAnglerHistory(SYSTEMTIME systime1, SYSTEMTIME systime2, int* pnCount);
void	SetAnglerStateInfo(PANGLERSTATE pState);
extern WCHAR	g_wszState[ANGLER_NONE + 1][100];
void MultiToWide(char *szBuf, WCHAR *wszBuf);
void CreateAnglerStateDBTable();
