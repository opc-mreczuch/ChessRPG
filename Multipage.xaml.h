//
// Multipage.xaml.h
// Deklaracja klasy Multipage
//

#pragma once

#include "Multipage.g.h"

namespace ChessRPG
{
	/// <summary>
	/// Pusta strona, która może być używana samodzielnie lub do której można nawigować wewnątrz ramki.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class Multipage sealed
	{
	public:
		Multipage();
		void Online_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Local_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Menu_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
