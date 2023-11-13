// MainMenu.xaml.cpp

#include "pch.h"
#include "MainMenu.xaml.h"
#include "Singleplayerpage.xaml.h"
#include "Multipage.xaml.h"
#include "Settingspage.xaml.h"

using namespace ChessRPG;

MainMenu::MainMenu()
{
	InitializeComponent();
}


void MainMenu::Singleplayer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    Frame->Navigate(Singleplayerpage::typeid);
}

void MainMenu::Multiplayer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(Multipage::typeid);
}

void MainMenu::Ustawienia_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(Settingspage::typeid);
}
void MainMenu::Exit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}

