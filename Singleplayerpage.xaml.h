//
// Singleplayerpage.xaml.h
// Deklaracja klasy Singleplayerpage
//

#pragma once

#include "Singleplayerpage.g.h"

namespace ChessRPG
{
	/// <summary>
	/// Pusta strona, która może być używana samodzielnie lub do której można nawigować wewnątrz ramki.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class Singleplayerpage sealed
	{
	public:
		Singleplayerpage();
		void Newgame_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Loadgame_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Menu_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
