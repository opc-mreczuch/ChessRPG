#pragma once

#include "Stage.g.h"
#include <vector>
#include <string>

namespace ChessRPG
{
    public ref class Stage sealed
    {
    public:
        Stage();

    private:
        void ChessButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void UpdateChessboardUI();

        static std::vector<std::vector<std::wstring>> chessboard;
        static std::vector<std::vector<Windows::UI::Xaml::Controls::Button^>> chessButtons;
        std::pair<int, int> selectedButton;
    };
}
