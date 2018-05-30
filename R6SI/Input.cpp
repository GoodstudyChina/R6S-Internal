#pragma once

#include "Input.h"
#include "Settings.h"

Input* Input::m_pInstance;

Input::Input()
{

}

Input::~Input()
{

}

void Input::StartThread()
{
	m_hThread = CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(KeyMonitor), NULL, NULL, NULL);
}

void Input::StopThread()
{
	TerminateThread(m_hThread, 0);
}

void Input::KeyMonitor()
{
	HWND gameWindow = FindWindowA(NULL, "Rainbow Six");

	while (true)
	{
		if (GetAsyncKeyState(VK_INSERT))
		{
			Settings::GetInstance()->Menu = !Settings::GetInstance()->Menu;

			Helpers::Log("Something Changed :o");

			std::this_thread::sleep_for(
				std::chrono::milliseconds(250));
		}
	}
}

Input* Input::GetInstance()
{
	if (!m_pInstance)
		m_pInstance = new Input();

	return m_pInstance;
}