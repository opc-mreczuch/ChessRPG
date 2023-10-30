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
		void NewGame_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void LoadGame_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
