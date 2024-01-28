// MainMenu.xaml.cpp

#include "pch.h"
#include "MainMenu.xaml.h"
#include "Singleplayerpage.xaml.h"
#include "Multipage.xaml.h"
#include "Settingspage.xaml.h"
#include <Windows.h>
#include <roapi.h>

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
    // Pobierz bieżące okno aplikacji
    auto appView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();

    // Zamknij bieżące okno aplikacji
    appView->TryConsolidateAsync();

    // Zwolnij interfejsy dla tego wątku
    RoUninitialize();
}


