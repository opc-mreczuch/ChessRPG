//
// Settingspage.xaml.cpp
// Implementacja klasy Settingspage
//

#include "pch.h"
#include "Settingspage.xaml.h"
#include "MainMenu.xaml.h"

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

Settingspage::Settingspage()
{
	InitializeComponent();
}

void Settingspage::Menu_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(MainMenu::typeid);
}

void Settingspage::General_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//Ustawienia generalne
}

void Settingspage::Graphics_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//Ustawienia graficzne
}

void Settingspage::Audio_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//Ustawienia audio
}