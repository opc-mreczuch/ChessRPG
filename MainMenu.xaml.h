// MainMenu.xaml.h

#pragma once

#include "MainMenu.g.h"

namespace ChessRPG
{
	[Windows::Foundation::Metadata::WebHostHidden]
		public ref class MainMenu sealed
	{
	public:
		MainMenu();
		void Singleplayer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Multiplayer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Ustawienia_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Exit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

	};
}
