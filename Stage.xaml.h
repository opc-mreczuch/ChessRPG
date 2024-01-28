// Stage.xaml.h

#pragma once

#include "Stage.g.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "MainMenu.xaml.h"

namespace ChessRPG
{
    public ref class Stage sealed
    {
    public:
        Stage();

    private:

        void ChessButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void UpdateChessboardUI();
        void SetChessPieceImage(Windows::UI::Xaml::Controls::Button^ button, const std::wstring& piece);
        void UndoButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void SaveButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        Platform::String^ Stage::GetChessboardState();
        void Stage::LoadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void Stage::Updateblack(const std::wstring& fileContent);
        void Stage::Updatewhite(const std::wstring& fileContent);
        void Stage::AddPointsForPiece(const std::wstring& piece);
        void Stage::changeShield();
        void SprawdzPlansze();
        void ExitButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);


        bool IsValidMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol);
        bool IsValidPawnMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol);
        bool IsValidRookMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol);
        bool IsValidKnightMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol);
        bool IsValidBishopMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol);
        bool IsValidQueenMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol);
        bool IsValidKingMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol);
        bool isWhiteTurn;

        static std::pair<int, int> selectedButton;
        static std::vector<std::vector<std::wstring>> chessboard;
        static std::vector<std::vector<std::wstring>> chessboard2;
        static std::vector<std::vector<Windows::UI::Xaml::Controls::Button^>> chessButtons;
    };
}
