//
// Multipage.xaml.cpp
// Implementacja klasy Multipage
//

#include "pch.h"
#include "Multipage.xaml.h"
#include "MainMenu.xaml.h"
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


Multipage::Multipage()
{
	InitializeComponent();
}

void Multipage::Online_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// Gra online
}
void Multipage::Local_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(Stage::typeid);
}
void Multipage::Menu_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(MainMenu::typeid);
}