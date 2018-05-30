#pragma once
#include "Entity.h"
#include "GameManager.h"
#include "GameRenderer.h"
#include "Settings.h"
#include "Renderer.h"

class Accuracy
{
public:
	void DoNoSpread();
	void DoNoRecoil();
	void AccuracyMain();

	static Accuracy* GetInstance();

private:
	Accuracy();
	~Accuracy();

	HANDLE m_hTriggerThread;

	static Accuracy* m_pInstance;
};