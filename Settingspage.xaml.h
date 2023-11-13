//
// Settingspage.xaml.h
// Deklaracja klasy Settingspage
//

#pragma once

#include "Settingspage.g.h"

namespace ChessRPG
{
	/// <summary>
	/// Pusta strona, która może być używana samodzielnie lub do której można nawigować wewnątrz ramki.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden] 
	public ref class Settingspage sealed
	{
	public:
		Settingspage();
		void General_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Graphics_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Audio_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Menu_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);

	};
}
