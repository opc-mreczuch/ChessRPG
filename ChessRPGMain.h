//ChessRPGMain.h
#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"

// Renderuje na ekranie zawartość Direct2D i 3D.
namespace ChessRPG
{
	class ChessRPGMain : public DX::IDeviceNotify
	{
	public:
		ChessRPGMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~ChessRPGMain();
		void CreateWindowSizeDependentResources();
		void StartTracking() { m_sceneRenderer->StartTracking(); }
		void TrackingUpdate(float positionX) { m_pointerLocationX = positionX; }
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// Wskaźnik do zasobów urządzenia przechowywany w pamięci podręcznej.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Zastąp własnymi modułami renderowania zawartości.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Renderowanie czasomierza pętli.
		DX::StepTimer m_timer;

		// Śledź bieżącą pozycję wskaźnika wejścia.
		float m_pointerLocationX;
	};
}