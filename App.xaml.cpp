//
// App.xaml.cpp
// Implementacja klasy aplikacji.
//

#include "DirectXPage.xaml.h"
#include "pch.h"
#include "App.xaml.h"
#include "MainMenu.xaml.h"

using namespace ChessRPG;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
/// <summary>
/// Inicjuje pojedynczy obiekt aplikacji. Jest to pierwszy wiersz napisanego kodu
/// wykonywanego i jest logicznym odpowiednikiem metod main() lub WinMain().
/// </summary>
App::App()
{
	InitializeComponent();

	Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
	Resuming += ref new EventHandler<Object^>(this, &App::OnResuming);
}

/// <summary>
/// Wywoływane, gdy aplikacja jest uruchamiana normalnie przez użytkownika końcowego. Inne punkty wejścia
/// będą używane, kiedy aplikacja zostanie uruchomiona, aby otworzyć określony plik, wyświetlić
/// wyniki wyszukiwania itd.
/// </summary>
/// <param name="e">Szczegóły dotyczące żądania uruchomienia i procesu.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
#if _DEBUG
	if (IsDebuggerPresent())
	{
		DebugSettings->EnableFrameRateCounter = true;
	}
#endif

	auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

	// Nie powtarzaj inicjowania aplikacji, gdy w oknie znajduje się już zawartość,
	// upewnij się tylko, że okno jest aktywne
	if (rootFrame == nullptr)
	{
		// Utwórz ramkę, która będzie pełnić funkcję kontekstu nawigacji, i skojarz ją z
		// kluczem SuspensionManager
		rootFrame = ref new Frame();

		rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

		// Umieść ramkę w bieżącym oknie
		Window::Current->Content = rootFrame;
	}

	if (rootFrame->Content == nullptr)
	{
		// Kiedy stos nawigacji nie jest przywrócony, przejdź do pierwszej strony,
		// konfigurując nową stronę przez przekazanie wymaganych informacji jako
		// parametr
		rootFrame->Navigate(TypeName(MainMenu::typeid), e->Arguments);
	}

	if (m_directXPage == nullptr)
	{
		m_directXPage = dynamic_cast<DirectXPage^>(rootFrame->Content);
	}

	if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
	{
		m_directXPage->LoadInternalState(ApplicationData::Current->LocalSettings->Values);
	}
	
	// Upewnij się, ze bieżące okno jest aktywne
	Window::Current->Activate();
}
/// <summary>
/// Wywoływane, gdy wykonanie aplikacji jest wstrzymywane. Stan aplikacji jest zapisywany
/// bez wiedzy o tym, czy aplikacja zostanie zakończona, czy wznowiona z niezmienioną zawartością
/// pamięci.
/// </summary>
/// <param name="sender">Źródło żądania wstrzymania.</param>
/// <param name="e">Szczegóły żądania wstrzymania.</param>

void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
	(void) sender;	// Nieużywany parametr
	(void) e;	// Nieużywany parametr

	m_directXPage->SaveInternalState(ApplicationData::Current->LocalSettings->Values);
}


/// <summary>
/// Wywoływane, gdy wykonanie aplikacji jest wznawiane.
/// </summary>
/// <param name="sender">Źródło żądania wznowienia.</param>
/// <param name="args">Szczegóły żądania wznowienia. </param>
void App::OnResuming(Object ^sender, Object ^args)
{
	(void) sender; // Nieużywany parametr
	(void) args; // Nieużywany parametr

	m_directXPage->LoadInternalState(ApplicationData::Current->LocalSettings->Values);
}

/// <summary>
/// Wywoływane, gdy nawigacja do konkretnej strony nie powiedzie się
/// </summary>
/// <param name="sender">Ramka, do której nawigacja nie powiodła się</param>
/// <param name="e">Szczegóły dotyczące niepowodzenia nawigacji</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
	throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}

void App::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
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