//
// Singleplayerpage.xaml.cpp
// Implementacja klasy Singleplayerpage
//

#include "pch.h"
#include "Singleplayerpage.xaml.h"
#include "MainMenu.xaml.h"
#include "Newgame.xaml.h"
#include "LoadGame.xaml.h"
#include "Stage.xaml.h"

using namespace ChessRPG;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

//Szablon elementu Pusta strona jest udokumentowany na stronie https://go.microsoft.com/fwlink/?LinkId=234238

Singleplayerpage::Singleplayerpage()
{
	InitializeComponent();
}

void Singleplayerpage::Newgame_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(Stage::typeid);
}
void Singleplayerpage::Loadgame_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(LoadGame::typeid);
}
void Singleplayerpage::Menu_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(MainMenu::typeid);
}