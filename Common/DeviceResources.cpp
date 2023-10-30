#include "pch.h"
#include "DeviceResources.h"
#include "DirectXHelper.h"
#include <windows.ui.xaml.media.dxinterop.h>

using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;

namespace DisplayMetrics
{
	// Ekrany o wysokiej rozdzielczość mogą wymagać bardzo dużo mocy procesora GPU i energii baterii na potrzeby renderowania.
	// Na przykład telefony z ekranami o wysokiej rozdzielczości mogą mieć krótki czas pracy baterii, jeśli
	// gry będą próbować renderować 60 klatek na sekundę przy pełnej rozdzielczości.
	// Decyzję o renderowaniu w pełnej rozdzielczości na wszystkich platformach i dla wszystkich typów urządzeń
	// należy podejmować z rozwagą.
	static const bool SupportHighResolutions = false;

	// Domyślne progi definiujące ekran o „wysokiej rozdzielczości”. Jeśli progi
	// zostaną przekroczone i parametr SupportHighResolutions ma wartość false, wymiary będą skalowane
	// o 50%.
	static const float DpiThreshold = 192.0f;		// 200% standardowego ekranu.
	static const float WidthThreshold = 1920.0f;	// 1080 pikseli szerokości.
	static const float HeightThreshold = 1080.0f;	// 1080 pikseli wysokości.
};

// Stałe używane do obliczania obrotów ekranu.
namespace ScreenRotation
{
	// Obrót wokół osi Z o 0 stopni
	static const XMFLOAT4X4 Rotation0( 
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// Obrót wokół osi Z o 90 stopni
	static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// Obrót wokół osi Z o 180 stopni
	static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// Obrót wokół osi Z o 270 stopni
	static const XMFLOAT4X4 Rotation270( 
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
};

// Konstruktor zasobów urządzenia.
DX::DeviceResources::DeviceResources() : 
	m_screenViewport(),
	m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1),
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_effectiveDpi(-1.0f),
	m_compositionScaleX(1.0f),
	m_compositionScaleY(1.0f),
	m_deviceNotify(nullptr)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();
}

// Konfiguruje zasoby, które nie zależą od urządzenia Direct3D.
void DX::DeviceResources::CreateDeviceIndependentResources()
{
	// Inicjuj zasoby Direct2D.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// Jeśli projekt należy do kompilacji debugowanej, włącz debugowanie programu Direct2D za pośrednictwem warstw zestawu SDK.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	// Inicjuj fabrykę Direct2D.
	DX::ThrowIfFailed(
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory3),
			&options,
			&m_d2dFactory
			)
		);

	// Inicjuj fabrykę DirectWrite.
	DX::ThrowIfFailed(
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory3),
			&m_dwriteFactory
			)
		);

	// Inicjuj fabrykę składnika Windows Imaging Component (WIC).
	DX::ThrowIfFailed(
		CoCreateInstance(
			CLSID_WICImagingFactory2,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_wicFactory)
			)
		);
}

// Konfiguruje urządzenie Direct3D i zapisuje w nim dojścia oraz kontekst urządzenia.
void DX::DeviceResources::CreateDeviceResources() 
{
	// Ta flaga dodaje obsługę powierzchni z uporządkowaniem kanału kolorów
	// innym niż domyślne uporządkowanie interfejsu API. Jest to wymagane w celu zachowania zgodności z programem Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	if (DX::SdkLayersAvailable())
	{
		// Jeśli projekt należy do kompilacji debugowanej, włącz debugowanie za pośrednictwem warstw zestawu SDK z tą flagą.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	// Ta tablica definiuje zestaw poziomów funkcji sprzętowych DirectX, które ta aplikacja będzie obsługiwać.
	// Kolejność powinna zostać zachowana.
	// Należy pamiętać, aby zadeklarować minimalny wymagany poziom funkcji aplikacji w opisie
	// aplikacji. Uznaje się, że wszystkie aplikacje obsługują poziom 9.1, chyba że określono inaczej.
	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Utwórz obiekt urządzenia interfejsu API programu Direct3D 11 i odpowiadający mu kontekst.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,					// Określ wskaźnik nullptr w celu używania karty domyślnej.
		D3D_DRIVER_TYPE_HARDWARE,	// Utwórz urządzenie, używając sterownika sprzętowej karty graficznej.
		0,							// Powinna być ustawiona wartość 0, chyba że sterownik to D3D_DRIVER_TYPE_SOFTWARE.
		creationFlags,				// Ustaw flagi debugowania i zgodności z programem Direct2D.
		featureLevels,				// Lista poziomów funkcji, które może obsługiwać ta aplikacja.
		ARRAYSIZE(featureLevels),	// Rozmiar powyższej listy.
		D3D11_SDK_VERSION,			// Dla aplikacji przeznaczonych do sklepu Microsoft Store zawsze ustawiaj ten parametr na wartość D3D11_SDK_VERSION.
		&device,					// Zwraca utworzone urządzenie Direct3D.
		&m_d3dFeatureLevel,			// Zwraca poziom funkcji utworzonego urządzenia.
		&context					// Zwraca bezpośredni kontekst urządzenia.
		);

	if (FAILED(hr))
	{
		// Jeśli inicjowanie się nie powiedzie, wróć do urządzenia WARP.
		// Aby uzyskać więcej informacji na temat platformy WARP, zobacz: 
		// https://go.microsoft.com/fwlink/?LinkId=286690
		DX::ThrowIfFailed(
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP, // Utwórz urządzenie WARP zamiast urządzenia sprzętowego.
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&device,
				&m_d3dFeatureLevel,
				&context
				)
			);
	}

	// Zapisz wskaźniki do urządzenia interfejsu API biblioteki Direct3D 11.3 i bezpośredniego kontekstu.
	DX::ThrowIfFailed(
		device.As(&m_d3dDevice)
		);

	DX::ThrowIfFailed(
		context.As(&m_d3dContext)
		);

	// Utwórz obiekt urządzenia programu Direct2D i odpowiadający mu kontekst.
	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice)
		);

	DX::ThrowIfFailed(
		m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
		);

	DX::ThrowIfFailed(
		m_d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_d2dContext
			)
		);
}

// Te zasoby muszą być odtwarzane przy każdej zmianie rozmiaru okna.
void DX::DeviceResources::CreateWindowSizeDependentResources() 
{
	// Wyczyść poprzedni kontekst rozmiaru okna.
	ID3D11RenderTargetView* nullViews[] = {nullptr};
	m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_d3dRenderTargetView = nullptr;
	m_d2dContext->SetTarget(nullptr);
	m_d2dTargetBitmap = nullptr;
	m_d3dDepthStencilView = nullptr;
	m_d3dContext->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);

	UpdateRenderTargetSize();

	// Szerokość i wysokość łańcucha wymiany musi być oparta na
	// natywna szerokość i wysokość. Jeśli okno nie jest w natywnym
	// orientacji, konieczne jest odwrócenie wymiarów.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	if (m_swapChain != nullptr)
	{
		// Jeśli łańcuch wymiany już istnieje, zmień jego rozmiar.
		HRESULT hr = m_swapChain->ResizeBuffers(
			2, // Łańcuch wymiany z podwójnym buforem.
			lround(m_d3dRenderTargetSize.Width),
			lround(m_d3dRenderTargetSize.Height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
			);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// Jeśli urządzenie zostało usunięte, należy utworzyć nowe urządzenie i nowy łańcuch wymiany.
			HandleDeviceLost();

			// Wszystko zostało skonfigurowane. Nie kontynuuj wykonywania tej metody. Zostanie ona ponownie wprowadzona przez element HandleDeviceLost 
			// i poprawnie skonfiguruj nowe urządzenie.
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		// W przeciwnym razie utwórz nowy element za pomocą tej samej karty, która jest używana przez istniejące urządzenie Direct3D.
		DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};

		swapChainDesc.Width = lround(m_d3dRenderTargetSize.Width);		// Dopasuj rozmiar okna.
		swapChainDesc.Height = lround(m_d3dRenderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// To jest najczęściej używany format łańcucha wymiany.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;								// Nie używaj próbkowania wielokrotnego.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;									// Użyj podwójnego buforowania, aby zminimalizować opóźnienie.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// Wszystkie aplikacje ze sklepu Microsoft Store muszą używać elementu _FLIP_ SwapEffects.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// Ta sekwencja uzyskuje fabrykę DXGI, za pomocą której utworzono powyższe urządzenie Direct3D.
		ComPtr<IDXGIDevice3> dxgiDevice;
		DX::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		ComPtr<IDXGIFactory4> dxgiFactory;
		DX::ThrowIfFailed(
			dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
			);

		// Łańcuch wymiany musi zostać utworzony dla kompozycji podczas korzystania z międzyoperacyjnego kodu XAML.
		ComPtr<IDXGISwapChain1> swapChain;
		DX::ThrowIfFailed(
			dxgiFactory->CreateSwapChainForComposition(
				m_d3dDevice.Get(),
				&swapChainDesc,
				nullptr,
				&swapChain
				)
			);

		DX::ThrowIfFailed(
			swapChain.As(&m_swapChain)
			);

		// Skojarz łańcuch wymiany z elementem SwapChainPanel
		// Zmiany interfejsu użytkownika trzeba będzie wysłać z powrotem do wątku interfejsu użytkownika
		m_swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
		{
			// Pobierz zapasowy interfejs natywny dla elementu SwapChainPanel
			ComPtr<ISwapChainPanelNative> panelNative;
			DX::ThrowIfFailed(
				reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative))
				);

			DX::ThrowIfFailed(
				panelNative->SetSwapChain(m_swapChain.Get())
				);
		}, CallbackContext::Any));

		// Upewnij się, że infrastruktura DXGI nie umieszcza w kolejce więcej niże jednej klatki jednocześnie. Obniża to opóźnienie i
		// powoduje, że aplikacja będzie renderować obraz tylko po każdej synchronizacji w pionie, w celu zminimalizowania zużycia energii.
		DX::ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);
	}

	// Ustaw odpowiednią orientację łańcucha wymiany i wygeneruj obraz 2W oraz
	// Przekształcenia macierzy 3W na potrzeby renderowania w obróconym łańcuchu wymiany.
	// Kąty obrotu dla przekształceń 2W i 3W różnią się od siebie.
	// Przyczyną jest różnica przestrzeni współrzędnych. Ponadto
	// jawnie określono, aby macierz 3W unikała błędów zaokrąglania.

	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform2D = Matrix3x2F::Identity();
		m_orientationTransform3D = ScreenRotation::Rotation0;
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform2D = 
			Matrix3x2F::Rotation(90.0f) *
			Matrix3x2F::Translation(m_logicalSize.Height, 0.0f);
		m_orientationTransform3D = ScreenRotation::Rotation270;
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform2D = 
			Matrix3x2F::Rotation(180.0f) *
			Matrix3x2F::Translation(m_logicalSize.Width, m_logicalSize.Height);
		m_orientationTransform3D = ScreenRotation::Rotation180;
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform2D = 
			Matrix3x2F::Rotation(270.0f) *
			Matrix3x2F::Translation(0.0f, m_logicalSize.Width);
		m_orientationTransform3D = ScreenRotation::Rotation90;
		break;

	default:
		throw ref new FailureException();
	}

	DX::ThrowIfFailed(
		m_swapChain->SetRotation(displayRotation)
		);

	// Skonfiguruj skalę odwrotną w łańcuchu wymiany
	DXGI_MATRIX_3X2_F inverseScale = { 0 };
	inverseScale._11 = 1.0f / m_effectiveCompositionScaleX;
	inverseScale._22 = 1.0f / m_effectiveCompositionScaleY;
	ComPtr<IDXGISwapChain2> spSwapChain2;
	DX::ThrowIfFailed(
		m_swapChain.As<IDXGISwapChain2>(&spSwapChain2)
		);

	DX::ThrowIfFailed(
		spSwapChain2->SetMatrixTransform(&inverseScale)
		);

	// Utwórz widok elementu docelowego renderowania dla buforu zapasowego łańcucha wymiany.
	ComPtr<ID3D11Texture2D1> backBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
		);

	DX::ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView1(
			backBuffer.Get(),
			nullptr,
			&m_d3dRenderTargetView
			)
		);

	// Utwórz widok wzornika głębi, który będzie używany podczas renderowania 3W, jeśli to konieczne.
	CD3D11_TEXTURE2D_DESC1 depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT, 
		lround(m_d3dRenderTargetSize.Width),
		lround(m_d3dRenderTargetSize.Height),
		1, // Ten widok wzornika głębi ma tylko jedną teksturę.
		1, // Użyj jednego poziomu mipmapowania.
		D3D11_BIND_DEPTH_STENCIL
		);

	ComPtr<ID3D11Texture2D1> depthStencil;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture2D1(
			&depthStencilDesc,
			nullptr,
			&depthStencil
			)
		);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
			depthStencil.Get(),
			&depthStencilViewDesc,
			&m_d3dDepthStencilView
			)
		);

	// Ustaw całe okno jako element docelowy okienka ekranu renderowania 3W.
	m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		m_d3dRenderTargetSize.Width,
		m_d3dRenderTargetSize.Height
		);

	m_d3dContext->RSSetViewports(1, &m_screenViewport);

	// Utwórz docelową mapę bitową programu Direct2D skojarzoną
	// bufor zapasowy łańcucha wymiany i ustaw go jako bieżący element docelowy.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = 
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			m_dpi,
			m_dpi
			);

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
		);

	DX::ThrowIfFailed(
		m_d2dContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			&m_d2dTargetBitmap
			)
		);

	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());
	m_d2dContext->SetDpi(m_effectiveDpi, m_effectiveDpi);

	// Antyaliasing tekstów wyszarzonych jest zalecany we wszystkich aplikacjach sklepu Microsoft Store.
	m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

// Określ wymiary obiektu docelowego renderowania i czy będzie on skalowany w dół.
void DX::DeviceResources::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;
	m_effectiveCompositionScaleX = m_compositionScaleX;
	m_effectiveCompositionScaleY = m_compositionScaleY;

	// Aby wydłużyć czas pracy baterii w przypadku urządzeń z ekranami o wysokiej rozdzielczości, renderuj z użyciem mniejszego obiektu docelowego renderowania
	// i zezwól procesorowi GPU na skalowanie obrazu wyjściowego przy wyświetlaniu.
	if (!DisplayMetrics::SupportHighResolutions && m_dpi > DisplayMetrics::DpiThreshold)
	{
		float width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_dpi);
		float height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_dpi);

		// Gdy urządzenie jest w orientacji pionowej, wysokość jest większa od szerokości. Porównaj
		// większy wymiar z progiem szerokości i mniejszy wymiar
		// z progiem wysokości.
		if (max(width, height) > DisplayMetrics::WidthThreshold && min(width, height) > DisplayMetrics::HeightThreshold)
		{
			// Aby skalować aplikację, zmieniamy obowiązującą wartość DPI. Rozmiar logiczny nie ulega zmianie.
			m_effectiveDpi /= 2.0f;
			m_effectiveCompositionScaleX /= 2.0f;
			m_effectiveCompositionScaleY /= 2.0f;
		}
	}

	// Oblicz wymagany rozmiar obiektu docelowego renderowania w pikselach.
	m_outputSize.Width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

	// Uniemożliwiaj tworzenie zawartości DirectX o zerowym rozmiarze.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);
}

// Ta metoda jest wywoływana przy tworzeniu (lub odtwarzaniu) kontrolki XAML.
void DX::DeviceResources::SetSwapChainPanel(SwapChainPanel^ panel)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_swapChainPanel = panel;
	m_logicalSize = Windows::Foundation::Size(static_cast<float>(panel->ActualWidth), static_cast<float>(panel->ActualHeight));
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_compositionScaleX = panel->CompositionScaleX;
	m_compositionScaleY = panel->CompositionScaleY;
	m_dpi = currentDisplayInformation->LogicalDpi;
	m_d2dContext->SetDpi(m_dpi, m_dpi);

	CreateWindowSizeDependentResources();
}

// Ta metoda jest wywoływana w procedurze obsługi zdarzeń dla zdarzenia SizeChanged.
void DX::DeviceResources::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// Ta metoda jest wywoływana w procedurze obsługi zdarzeń dla zdarzenia DpiChanged.
void DX::DeviceResources::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;
		m_d2dContext->SetDpi(m_dpi, m_dpi);
		CreateWindowSizeDependentResources();
	}
}

// Ta metoda jest wywoływana w procedurze obsługi zdarzeń dla zdarzenia OrientationChanged.
void DX::DeviceResources::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

// Ta metoda jest wywoływana w procedurze obsługi zdarzeń dla zdarzenia CompositionScaleChanged.
void DX::DeviceResources::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
	if (m_compositionScaleX != compositionScaleX ||
		m_compositionScaleY != compositionScaleY)
	{
		m_compositionScaleX = compositionScaleX;
		m_compositionScaleY = compositionScaleY;
		CreateWindowSizeDependentResources();
	}
}

// Ta metoda jest wywoływana w procedurze obsługi zdarzeń dla zdarzenia DisplayContentsInvalidated.
void DX::DeviceResources::ValidateDevice()
{
	// Urządzenie D3D nie jest już prawidłowe, jeśli domyślna karta zmieniła się od czasu, gdy urządzenie
	// zostało utworzone lub jeśli urządzenie zostało usunięte.

	// Najpierw pobierz informacje dla domyślnej karty z czasu, gdy urządzenie zostało utworzone.

	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

	ComPtr<IDXGIAdapter> deviceAdapter;
	DX::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

	ComPtr<IDXGIFactory2> deviceFactory;
	DX::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

	ComPtr<IDXGIAdapter1> previousDefaultAdapter;
	DX::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

	DXGI_ADAPTER_DESC1 previousDesc;
	DX::ThrowIfFailed(previousDefaultAdapter->GetDesc1(&previousDesc));

	// Następnie pobierz informacje dla bieżącej domyślnej karty.

	ComPtr<IDXGIFactory4> currentFactory;
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

	ComPtr<IDXGIAdapter1> currentDefaultAdapter;
	DX::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

	DXGI_ADAPTER_DESC1 currentDesc;
	DX::ThrowIfFailed(currentDefaultAdapter->GetDesc1(&currentDesc));

	// Jeśli identyfikatory LUID karty nie pasują lub jeśli urządzenie zgłasza, że zostało usunięte,
	// musi zostać utworzone nowe urządzenie D3D.

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		// Zwolnij odwołania do zasobów związanych ze starym urządzeniem.
		dxgiDevice = nullptr;
		deviceAdapter = nullptr;
		deviceFactory = nullptr;
		previousDefaultAdapter = nullptr;

		// Utwórz nowe urządzenie i łańcuch wymiany.
		HandleDeviceLost();
	}
}

// Odtwórz wszystkie zasoby urządzenia i ustaw dla nich ponownie bieżący stan.
void DX::DeviceResources::HandleDeviceLost()
{
	m_swapChain = nullptr;

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceLost();
	}

	CreateDeviceResources();
	m_d2dContext->SetDpi(m_dpi, m_dpi);
	CreateWindowSizeDependentResources();

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceRestored();
	}
}

// Zarejestruj nasz element DeviceNotify, aby uzyskiwać informacje o utracie i utworzeniu urządzenia.
void DX::DeviceResources::RegisterDeviceNotify(DX::IDeviceNotify* deviceNotify)
{
	m_deviceNotify = deviceNotify;
}

// Wywołaj tę metodę w przypadku wstrzymania aplikacji. Stanowi wskazówkę dla sterownika, że aplikacja 
// przechodzi do stanu bezczynności i że tymczasowe bufory można odzyskać do użytku przez inne aplikacje.
void DX::DeviceResources::Trim()
{
	ComPtr<IDXGIDevice3> dxgiDevice;
	m_d3dDevice.As(&dxgiDevice);

	dxgiDevice->Trim();
}

// Pokaż zawartość łańcucha wymiany na ekranie.
void DX::DeviceResources::Present() 
{
	// Pierwszy argument nakazuje zablokowanie infrastruktury DXGI do czasu włączenia synchronizacji w pionie, umieszczając aplikację
	// uśpienie do następnej synchronizacji w pionie. Dzięki temu cykle nie są używane do renderowania
	// klatek, które nigdy nie będą wyświetlane na ekranie.
	DXGI_PRESENT_PARAMETERS parameters = { 0 };
	HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

	// Odrzuć zawartość elementu docelowego renderowania.
	// Ta operacja jest prawidłowa tylko wtedy, gdy istniejąca zawartość zostanie całkowicie
	// przesłonięcie. W przypadku używania prostokątów zanieczyszczonych lub z przewijaniem należy zmodyfikować to wywołanie.
	m_d3dContext->DiscardView1(m_d3dRenderTargetView.Get(), nullptr, 0);

	// Odrzuć zawartość wzornika głębi.
	m_d3dContext->DiscardView1(m_d3dDepthStencilView.Get(), nullptr, 0);

	// Jeśli urządzenie zostało usunięte w wyniku zakończenia połączenia lub uaktualnienia sterownika, 
	// musi odtworzyć wszystkie zasoby urządzenia.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		HandleDeviceLost();
	}
	else
	{
		DX::ThrowIfFailed(hr);
	}
}

// Ta metoda określa obrót między orientacją natywną urządzenia wyświetlającego i
// bieżąca orientacja wyświetlania.
DXGI_MODE_ROTATION DX::DeviceResources::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// Uwaga: orientacja natywna może być tylko pozioma lub pionowa, mimo że
	// wyliczenie DisplayOrientations ma inne wartości.
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}