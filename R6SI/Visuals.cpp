#include "stdafx.h"
#include "Visuals.h"
#include "Renderer.h"

Visuals* Visuals::m_pInstance;

Visuals::Visuals()
{
}

Visuals::~Visuals()
{
}

void Visuals::VisualsMain()
{
	Visuals::DrawESP();
	Visuals::DrawVisuals();
}

void Visuals::DrawESP()
{
	RECT desktop;
	GetWindowRect(hDesktop, &desktop);
	int horizontal = desktop.right;
	int vertical = desktop.bottom;

	auto pGameManager = Engine::GameManager::GetInstance();
	auto pCamera = Engine::GameRenderer::GetInstance()->GetCamera();

	if (pGameManager->IsInGame())
	{
		for (uint64_t i = 0; i < pGameManager->GetEntities().GetSize(); i++)
		{
			Engine::Entity* pEntity = pGameManager->GetEntities()[i];
			Engine::Entity* pLocal = pGameManager->GetLocalPlayer();

			if (pEntity == nullptr)
				return;

			INT8 Health = pEntity->GetHealth();

			if (Health > 0)
			{
				float distance = pCamera->GetViewTranslation().Distance(pEntity->GetPosition());

				if (1.7f < distance)
				{
					Engine::Vector3 Position = pCamera->WorldToScreen(pEntity->GetPosition());
					Engine::Vector3 HeadPosition = pCamera->WorldToScreen(pEntity->GetHead());
					float BoxHeight1 = Position.y - HeadPosition.y;
					float BoxHeight2 = std::abs(Position.x - HeadPosition.x);
					float BoxHeight = (BoxHeight1 > BoxHeight2) ? BoxHeight1 : BoxHeight2;
					float BoxWidth = BoxHeight / 1.5f;

					if (Position.z >= 1.0f)
					{
						if (pEntity->GetTeam() != pLocal->GetTeam()) {
							if (Settings::GetInstance()->bESPBox)
							{
								// Position of rect.
								XMFLOAT4 rect{ HeadPosition.x - (BoxWidth / 2) - 1,             // x1
									HeadPosition.y + (BoxHeight / 500) - (BoxHeight / 5) - 1,   // y1
									BoxWidth + 2,											    // x2
									BoxHeight + 17 };                                           // y2

																								// Rendering of rect, with color.
								Renderer::GetInstance()->drawRect(rect, 1.0f, { 0.764705896f, 0.176470593f, 0.019607844f, 1.f });
							}

							if (Settings::GetInstance()->bESPHead)
							{
								// Position of circle.
								XMFLOAT2 circ{ HeadPosition.x,                                  // x1
											   HeadPosition.y };                                 // y1

																					  // Rendering of circle, with color.
								Renderer::GetInstance()->drawCircle(circ, BoxHeight / 15, Color{ 0.764705896f, 0.176470593f, 0.019607844f, 1.f });
							}

							if (Settings::GetInstance()->bESPHealth)
							{
								// Values for drawing health correctly.
								float CurrentHealth = pEntity->GetHealth() / 80.0f;
								if (CurrentHealth < 0 || CurrentHealth >= 1) CurrentHealth = 1;

								// Position of background rect.
								XMFLOAT4 rectBackground{ HeadPosition.x - (BoxWidth / 2) - 6 - 1,                                                     // x1
													HeadPosition.y + (BoxHeight / 500) - (BoxHeight / 5) - 1,                                         // y1
													5,                                                                                                // x2
													BoxHeight + 17 + 2 };                                                                             // y2

								// Position of health rect.
								XMFLOAT4 rectHealth{ HeadPosition.x - (BoxWidth / 2) - 6,                                                             // x1
													HeadPosition.y + (BoxHeight / 500) - (BoxHeight / 5),                                             // y1
													3,                                                                                                // x2
													(BoxHeight + 15) * CurrentHealth };                                                               // y2

								// Rendering of both rect's, with color.
								Renderer::GetInstance()->drawFilledRect(rectBackground, Color{ 0.254901975f, 0.254901975f, 0.254901975f, 1.f });      // Background to contrast the health and show how much the person has lost of total health.
								Renderer::GetInstance()->drawFilledRect(rectHealth, Color{ 0.062745102f, 0.486274511f, 0.062745102f, 1.f });          // Actual display of health in color green on top of the background which is dim black / grey.
							}

							if (Settings::GetInstance()->bESPSnapline)
							{
								// Position of x1 & y1. AKA From.
								XMFLOAT2 lineFrom(horizontal / 2,                                  // x1
									vertical);                                      // y1

																					// Position of x2 & y2. AKA To.
								XMFLOAT2 lineTo{ HeadPosition.x,                                    // x2
									HeadPosition.y };                                  // y2

								// Rendering of line, from bottom mid screen to players heads, with color.
								Renderer::GetInstance()->drawLine(lineFrom, lineTo, Color{ 0.764705896f, 0.176470593f, 0.019607844f, 1.f });
							}

							if (Settings::GetInstance()->bESPName)
							{
								// Position of players name.
								XMFLOAT2 namePosition{ HeadPosition.x - (BoxWidth / 2) - 1,         // x1
									(HeadPosition.y - (BoxHeight / 5)) + BoxHeight + 10 - 1 };      // y1

								// Rendering of players name, with color, font size, font type, flags etc...
								// Currently trying to fix this ------> Renderer::GetInstance()->drawText(namePosition, pEntity->GetName(), Color{ 0.764705896f, 0.176470593f, 0.019607844f, 1.f }, FW1_NOWORDWRAP, 1.0f, Renderer::GetInstance()->defaultFontFamily);
								
								// ^^^ Causing Access Violation
							}
						}
					}
				}
			}
		}
	}
}

void Visuals::DrawVisuals()
{
	RECT desktop;
	GetWindowRect(hDesktop, &desktop);
	int horizontal = desktop.right;
	int vertical = desktop.bottom;

	auto pGameManager = Engine::GameManager::GetInstance();

	if (Settings::GetInstance()->bVisualsCrosshair)
	{
		if (pGameManager->IsInGame())
		{
			// Position of x1 & y1. AKA From.
			XMFLOAT2 line1From(horizontal / 2 - 15,                                  // x1
							   vertical / 2);                                        // y1

			// Position of x2 & y2. AKA To.
			XMFLOAT2 line1To(horizontal / 2 - 5,                                     // x2
				             vertical / 2);                                          // y2

			// Rendering of line1, with color.
			Renderer::GetInstance()->drawLine(line1From, line1To, Color{ 0.254901975f, 0.254901975f, 0.254901975f, 1.f });

			// Position of x1 & y1. AKA From.
			XMFLOAT2 line2From(horizontal / 2 + 16,                                  // x1
				               vertical / 2);                                        // y1

			// Position of x2 & y2. AKA To.
			XMFLOAT2 line2To(horizontal / 2 + 6,                                     // x2
				             vertical / 2);                                          // y2

			// Rendering of line2, with color.
			Renderer::GetInstance()->drawLine(line2From, line2To, Color{ 0.254901975f, 0.254901975f, 0.254901975f, 1.f });

			// Position of x1 & y1. AKA From.
			XMFLOAT2 line3From(horizontal / 2,                                       // x1
				               vertical / 2 - 15);								     // y1

			// Position of x2 & y2. AKA To.
			XMFLOAT2 line3To(horizontal / 2,                                         // x2
				             vertical / 2 - 5);                                     // y2

			// Rendering of line3, with color.
			Renderer::GetInstance()->drawLine(line3From, line3To, Color{ 0.254901975f, 0.254901975f, 0.254901975f, 1.f });

			// Position of x1 & y1. AKA From.
			XMFLOAT2 line4From(horizontal / 2,                                       // x1
				               vertical / 2 + 16);							         // y1

			// Position of x2 & y2. AKA To.
			XMFLOAT2 line4To(horizontal / 2,                                         // x2
				             vertical / 2 + 6);                                      // y2

			// Rendering of line4, with color.
			Renderer::GetInstance()->drawLine(line4From, line4To, Color{ 0.254901975f, 0.254901975f, 0.254901975f, 1.f });
		}
	}
}

Visuals* Visuals::GetInstance()
{
	if (!m_pInstance)
		m_pInstance = new Visuals();

	return m_pInstance;
}
