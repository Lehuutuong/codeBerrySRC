void	SaveAccountInfo();
void	LoadAccountInfo();
void	SaveAdminInfo();
void	LoadAdminInfo();
void	BackUpAccountFile();
void	LoadStateInfo();

BOOL	CheckHuntUserExpiration(int nIndex);
int		CheckHuntUserInfo(LPWSTR szID, LPWSTR szPwd, LPBYTE byKey = NULL);
