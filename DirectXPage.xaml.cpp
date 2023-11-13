//
// DirectXPage.xaml.cpp
// Implementacja klasy DirectXPage.
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace ChessRPG;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();
	Window::Current->CoreWindow->KeyDown += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::KeyEventArgs^>(this, &DirectXPage::OnKeyDown);

	// Zarejestruj procedury obsługi zdarzeń cyklu życia strony.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// W tym momencie mamy dostęp do urządzenia. 
	// Można utworzyć zasoby zależne od urządzenia.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// Zarejestruj nasz element SwapChainPanel, aby uzyskać niezależne zdarzenia wejściowe wskaźnika
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// Klasa CoreIndependentInputSource spowoduje wywołanie zdarzeń wskaźnika dla wskazanych typów urządzeń niezależnie od wątku, na którym została utworzona.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// Zarejestruj się w celu odbierania zdarzeń wskaźnika, które będą wywoływane w wątku tła.
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// Rozpocznij przetwarzanie komunikatów wejściowych, gdy zaczną się pojawiać.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Uruchom zadanie na dedykowanym wątku w tle o wysokim priorytecie.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<ChessRPGMain>(new ChessRPGMain(m_deviceResources));
	m_main->StartRenderLoop();

}

DirectXPage::~DirectXPage()
{
	// Zatrzymaj zdarzenia renderowania i przetwarzania w chwili zniszczenia.
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// Zapisuje bieżący stan aplikacji dla zdarzeń wstrzymania i przerwania.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Zatrzymaj renderowanie, gdy praca aplikacji jest wstrzymana.
	m_main->StopRenderLoop();

	// W tym miejscu umieść kod zapisywania stanu aplikacji.
}

// Ładuje bieżący stan aplikacji dla zdarzenia wznowienia.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// W tym miejscu umieść kod ładowania stanu aplikacji.

	// Rozpocznij renderowanie, gdy działanie aplikacji zostanie wznowione.
	m_main->StartRenderLoop();
}

// Procedury obsługi zdarzeń okna.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// Procedury obsługi zdarzeń wyświetlania informacji.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// Uwaga: Wartość LogicalDpi pobrana w tym miejscu może nie być zgodna z wartością DPI obowiązującą dla aplikacji,
	// jeśli wykonywane jest skalowanie dla urządzeń obsługujących wysoką rozdzielczość. Po ustawieniu wartości DPI w elemencie DeviceResources
	// należy zawsze pobierać tę wartość za pomocą metody GetDpi.
	// Więcej szczegółów zawiera plik DeviceResources.cpp.
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// Wywoływane po kliknięciu przycisku paska aplikacji.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Użyj paska aplikacji, jeśli jest to odpowiednie dla aplikacji. Zaprojektuj pasek aplikacji, 
	// następnie wypełnij procedury obsługi zdarzeń (jak ta).
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// Gdy wskaźnik zostanie naciśnięty, rozpocznij śledzenie jego ruchu.
	m_main->StartTracking();
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	// Zaktualizuj kod śledzenia wskaźnika.
	if (m_main->IsTracking())
	{
		m_main->TrackingUpdate(e->CurrentPoint->Position.X);
	}
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// Po zwolnieniu wskaźnika przerwij śledzenie jego ruchu.
	m_main->StopTracking();
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
	if (args->VirtualKey == Windows::System::VirtualKey::Escape)
	{
		// For debugging, check if this point is reached
		OutputDebugString(L"Escape key pressed\n");

		auto dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

		// Run the closing operation on the UI thread
		dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler(
			[this]()
			{
				OutputDebugString(L"Closing application\n");
				Windows::UI::Xaml::Window::Current->Close();
			}
		));
	}
}
