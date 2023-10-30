#pragma once

#include <ppltasks.h>	// Dla create_task

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Ustaw punkt przerwania w tym wierszu w celu przechwytywania błędów interfejsu API systemu Win32.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Funkcja odczytująca asynchronicznie zawartość pliku binarnego.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([] (StorageFile^ file) 
		{
			return FileIO::ReadBufferAsync(file);
		}).then([] (Streams::IBuffer^ fileBuffer) -> std::vector<byte> 
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	// Konwertuje długość wyrażoną w pikselach niezależnych od urządzenia (DIP) na długość wyrażoną w pikselach fizycznych.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Zaokrąglij do najbliższej liczby całkowitej.
	}

#if defined(_DEBUG)
	// Sprawdź obsługę warstwy SDK.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // Nie ma potrzeby tworzenia rzeczywistego urządzenia sprzętowego.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Sprawdź warstwę SDK.
			nullptr,                    // Wystarczy dowolny poziom funkcji.
			0,
			D3D11_SDK_VERSION,          // Dla aplikacji przeznaczonych do sklepu Microsoft Store zawsze ustawiaj ten parametr na wartość D3D11_SDK_VERSION.
			nullptr,                    // Nie trzeba utrzymywać odwołania do urządzenia D3D.
			nullptr,                    // Nie trzeba znać poziomu funkcji.
			nullptr                     // Nie trzeba utrzymywać odwołania do kontekstu urządzenia D3D.
			);

		return SUCCEEDED(hr);
	}
#endif
}
