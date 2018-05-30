#pragma once

class Settings
{
public:
	bool bESPBox;
	bool bESPHealth;
	bool bESPName;
	bool bESPDistance;
	bool bESPHead;
	bool bESPSnapline;

	bool bVisualsCrosshair;

	bool bAccuracyNoSpread;
	float fSpread;
	bool bAccuracyNoRecoil;
	float fRecoil;

	bool Menu;

	static Settings* GetInstance();

private:
	Settings();
	~Settings();

	static Settings* m_pInstance;
};