#include "Accuracy.h"
#include "Renderer.h"
#include "stdafx.h"

Accuracy* Accuracy::m_pInstance;

Accuracy::Accuracy()
{
}

Accuracy::~Accuracy()
{
}

void Accuracy::AccuracyMain()
{
	if (Settings::GetInstance()->bAccuracyNoSpread)
	{
		Accuracy::DoNoSpread();
	}

	if (Settings::GetInstance()->bAccuracyNoRecoil)
	{
		Accuracy::DoNoRecoil();
	}
}

void Accuracy::DoNoSpread()
{
	auto pGameManager = Engine::GameManager::GetInstance();
	if (pGameManager->IsInGame())
	{
		float Spread = pGameManager->GetSpread();
		if (Spread > 0.f && Spread < 1.f)
		{
			if (Spread != Settings::GetInstance()->fSpread)
			{
				pGameManager->GetSpread() = Settings::GetInstance()->fSpread;
			}
		}
	}
}

void Accuracy::DoNoRecoil()
{
	auto pGameManager = Engine::GameManager::GetInstance();
	if (pGameManager->IsInGame())
	{
		float Recoil = pGameManager->GetRecoil();
		if (Recoil > 0.f && Recoil < 1.f)
		{
			if (Recoil != Settings::GetInstance()->fRecoil)
			{
				pGameManager->GetRecoil() = Settings::GetInstance()->fRecoil;
			}
		}
	}
}

Accuracy* Accuracy::GetInstance()
{
	if (!m_pInstance)
		m_pInstance = new Accuracy();

	return m_pInstance;
}