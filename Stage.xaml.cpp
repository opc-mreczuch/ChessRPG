#include "pch.h"
#include "Stage.xaml.h"
#include <sstream>
#include <Windows.UI.Xaml.Controls.Primitives.h>

using namespace ChessRPG;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Controls::Primitives;

std::vector<std::vector<std::wstring>> Stage::chessboard;
std::vector<std::vector<Windows::UI::Xaml::Controls::Button^>> Stage::chessButtons;

Stage::Stage()
{
    InitializeComponent();

    // Utwórz planszę szachową zgodnie z danymi wejściowymi
    chessboard = {
        { L"br1", L"bn1", L"bb1", L"bq1", L"bk1", L"bb2", L"bn2", L"br2" },
        { L"bp1", L"bp2", L"bp3", L"bp4", L"bp5", L"bp6", L"bp7", L"bp8" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"wp1", L"wp2", L"wp3", L"wp4", L"wp5", L"wp6", L"wp7", L"wp8" },
        { L"wr1", L"wn1", L"wb1", L"wq1", L"wk1", L"wb2", L"wn2", L"wr2" }
    };

    // Indeksy guzików odpowiadają indeksom w wektorze planszy
    chessButtons = std::vector<std::vector<Windows::UI::Xaml::Controls::Button^>>(8, std::vector<Windows::UI::Xaml::Controls::Button^>(8, nullptr));

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            String^ buttonName = "i" + (row + 1) + "y" + (col + 1);
            Windows::UI::Xaml::Controls::Button^ chessButton = dynamic_cast<Windows::UI::Xaml::Controls::Button^>(this->FindName(buttonName));

            if (chessButton != nullptr)
            {
                // Ustaw treść przycisku zgodnie z planszą szachową
                chessButton->Content = ref new String(chessboard[row][col].c_str());

                // Dodaj obsługę zdarzenia dla przycisku
                chessButton->Click += ref new RoutedEventHandler(this, &Stage::ChessButton_Click);

                chessButtons[row][col] = chessButton;
            }
        }
    }
}

void Stage::ChessButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    Windows::UI::Xaml::Controls::Button^ clickedButton = dynamic_cast<Windows::UI::Xaml::Controls::Button^>(sender);

    // Pobierz nazwę przycisku
    Platform::String^ buttonName = clickedButton->Name;

    // Konwertuj nazwę przycisku do std::wstring
    std::wstring buttonNameStr(buttonName->Data());

    // Pobierz numer wiersza i kolumny z nazwy przycisku
    int row = buttonNameStr[1] - L'0';
    int col = buttonNameStr[3] - L'0';

    // Sprawdź, czy to pierwsze czy drugie wciśnięcie
    if (selectedButton.first == -1 && selectedButton.second == -1)
    {
        // To pierwsze wciśnięcie - zapisz współrzędne
        selectedButton = std::make_pair(row, col);
    }
    else
    {
        // To drugie wciśnięcie - zamień miejscami i zresetuj licznik

        // Sprawdź, czy indeksy nie wychodzą poza zakres
        if (row > 0 && row <= 8 && col > 0 && col <= 8 &&
            selectedButton.first > 0 && selectedButton.first <= 8 && selectedButton.second > 0 && selectedButton.second <= 8)
        {
            std::swap(chessboard[row - 1][col - 1], chessboard[selectedButton.first - 1][selectedButton.second - 1]);
            UpdateChessboardUI();
        }

        selectedButton = std::make_pair(-1, -1);
    }
}

void Stage::UpdateChessboardUI()
{
    // Aktualizuj treści przycisków na podstawie planszy szachowej
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            // Ustaw treść przycisku zgodnie z planszą szachową
            chessButtons[row][col]->Content = ref new String(chessboard[row][col].c_str());
        }
    }
}
