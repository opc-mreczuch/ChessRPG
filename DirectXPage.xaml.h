//
// DirectXPage.xaml.h
// Deklaracja klasy DirectXPage.
//

#pragma once

#include "DirectXPage.g.h"

#include "Common\DeviceResources.h"
#include "ChessRPGMain.h"

namespace ChessRPG
{
	/// <summary>
	/// Strona, która hostuje element SwapChainPanel oprogramowania DirectX.
	/// </summary>
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		virtual ~DirectXPage();

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

	private:
		// Niskopoziomowa procedura obsługi zdarzeń renderowania XAML.
		void OnRendering(Platform::Object^ sender, Platform::Object^ args);

		// Procedury obsługi zdarzeń okna.
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

		// Procedury obsługi zdarzeń wyświetlania informacji.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// Inne procedury obsługi zdarzeń.
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		// Śledź niezależne sygnały wejścia w wątku roboczym pracującym w tle.
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

		// Funkcje niezależnej obsługi danych wejściowych.
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

		// Zasoby użyte do renderowania zawartości DirectX w tle strony XAML.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<ChessRPGMain> m_main; 
		bool m_windowVisible;
	};
}

