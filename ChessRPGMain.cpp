//ChessRPGMain.cpp
#include "pch.h"
#include "ChessRPGMain.h"
#include "Common\DirectXHelper.h"

using namespace ChessRPG;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Ładuje i inicjuje zasoby aplikacji podczas ładowania aplikacji.
ChessRPGMain::ChessRPGMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), m_pointerLocationX(0.0f)
{
	// Zarejestruj się w celu uzyskiwania powiadomień, że urządzenie zostało utracone lub utworzone ponownie
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Zastąp to inicjacją zawartości aplikacji.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: Zmień ustawienia czasomierza, jeśli chcesz użyć czegoś innego niż domyślny tryb zmiennego kroku czasu.
	// np. dla logiki aktualizacji ze stałym krokiem czasu 60 kl./s użyj wywołania:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

ChessRPGMain::~ChessRPGMain()
{
	// Powiadomienie dotyczące wyrejestrowania urządzenia
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Aktualizuje stan aplikacji, gdy następuje zmiana rozmiaru okna (np. zmienia się orientacja urządzenia)
void ChessRPGMain::CreateWindowSizeDependentResources() 
{
	// TODO: Zastąp to zależnym od rozmiaru inicjowaniem zawartości aplikacji.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void ChessRPGMain::StartRenderLoop()
{
	// Jeśli pętla renderowania animacji jest już uruchomiona, nie uruchamiaj kolejnego wątku.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Utwórz zadanie, które będzie działać w wątku w tle.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Oblicz zaktualizowaną klatkę i wyrenderuj raz na interwał wygaszania pionowego.
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			Update();
			if (Render())
			{
				m_deviceResources->Present();
			}
		}
	});

	// Uruchom zadanie na dedykowanym wątku w tle o wysokim priorytecie.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void ChessRPGMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

// Aktualizuje stan aplikacji raz na klatkę.
void ChessRPGMain::Update() 
{
	ProcessInput();

	// Aktualizuj obiekty sceny.
	m_timer.Tick([&]()
	{
		// TODO: Zastąp to funkcją aktualizującą zawartość aplikacji.
		m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// Przetwórz wszystkie dane wejściowe użytkownika przed zaktualizowaniem stanu gry
void ChessRPGMain::ProcessInput()
{
	// TODO: Dodaj tutaj obsługę wejścia na klatkę.
	m_sceneRenderer->TrackingUpdate(m_pointerLocationX);
}

// Renderuje bieżącą klatkę zgodnie z bieżącym stanem aplikacji.
// Zwraca wartość true, jeśli ramka została wyrenderowana i jest gotowa do wyświetlenia.
bool ChessRPGMain::Render() 
{
	// Nie próbuj renderować czegokolwiek przed pierwszą aktualizacją.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Resetuj okienko ekranu tak, aby wykorzystać cały ekran.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Resetuj elementy docelowe renderowania na ekranie.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Wyczyść bufor zapasowy i widok wzornika głębi.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Renderuj obiekty sceny.
	// TODO: Zastąp to funkcjami renderującymi zawartość aplikacji.
	m_sceneRenderer->Render();
	m_fpsTextRenderer->Render();

	return true;
}

// Powiadamia moduły renderowania, że należy zwolnić zasoby urządzenia.
void ChessRPGMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Powiadamia moduły renderowania, że można teraz odtworzyć zasoby urządzenia.
void ChessRPGMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
