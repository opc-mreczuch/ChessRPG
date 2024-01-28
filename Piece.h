#pragma once

#include <string>

// Klasa bazowa reprezentuj�ca figur� szachow�
class ChessPiece
{
public:
    ChessPiece(std::wstring symbol, std::wstring color, int index)
        : symbol_(symbol), color_(color), index_(index) {}

    virtual std::wstring GetSymbol() const
    {
        return color_ + symbol_ + std::to_wstring(index_);
    }

    // Wirtualna funkcja do walidacji ruch�w
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const
    {
        // Domy�lna implementacja zezwala na dowolny ruch
        return true;
    }

    virtual ~ChessPiece() {}

private:
    std::wstring symbol_;
    std::wstring color_;
    int index_;
};

// Klasa reprezentuj�ca piona
class Pawn : public ChessPiece
{
public:
    Pawn(std::wstring color, int index) : ChessPiece(color == L"white" ? L"P" : L"p", color, index) {}

    // Przes�oni�cie funkcji IsValidMove dla piona
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logik� specyficzn� dla ruchu piona
        // Na przyk�ad, zezwalaj na przesuni�cie piona do przodu o jedno pole
        return (toRow == fromRow + 1) && (toCol == fromCol);
    }
};

// Klasa reprezentuj�ca skoczka
class Knight : public ChessPiece
{
public:
    Knight(std::wstring color, int index) : ChessPiece(color == L"white" ? L"N" : L"n", color, index) {}

    // Przes�oni�cie funkcji IsValidMove dla skoczka
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logik� specyficzn� dla ruchu skoczka
        // Na przyk�ad, zezwalaj na ruch w kszta�cie litery "L"
        return ((std::abs(toRow - fromRow) == 2 && std::abs(toCol - fromCol) == 1) ||
            (std::abs(toRow - fromRow) == 1 && std::abs(toCol - fromCol) == 2));
    }
};

// Klasa reprezentuj�ca go�ca
class Bishop : public ChessPiece
{
public:
    Bishop(std::wstring color, int index) : ChessPiece(color == L"white" ? L"B" : L"b", color, index) {}

    // Przes�oni�cie funkcji IsValidMove dla go�ca
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logik� specyficzn� dla ruchu go�ca
        // Na przyk�ad, zezwalaj na ruch po przek�tnej
        return (std::abs(toRow - fromRow) == std::abs(toCol - fromCol));
    }
};

// Klasa reprezentuj�ca wie��
class Rook : public ChessPiece
{
public:
    Rook(std::wstring color, int index) : ChessPiece(color == L"white" ? L"R" : L"r", color, index) {}

    // Przes�oni�cie funkcji IsValidMove dla wie�y
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logik� specyficzn� dla ruchu wie�y
        // Na przyk�ad, zezwalaj na ruch w pionie lub poziomie
        return (fromRow == toRow || fromCol == toCol);
    }
};

// Klasa reprezentuj�ca kr�low�
class Queen : public ChessPiece
{
public:
    Queen(std::wstring color, int index) : ChessPiece(color == L"white" ? L"Q" : L"q", color, index) {}

    // Przes�oni�cie funkcji IsValidMove dla kr�lowej
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logik� specyficzn� dla ruchu kr�lowej
        // Na przyk�ad, zezwalaj na ruch w pionie, poziomie lub po przek�tnej
        return ((fromRow == toRow || fromCol == toCol) || (std::abs(toRow - fromRow) == std::abs(toCol - fromCol)));
    }
};

// Klasa reprezentuj�ca kr�la
class King : public ChessPiece
{
public:
    King(std::wstring color, int index) : ChessPiece(color == L"white" ? L"K" : L"k", color, index) {}

    // Przes�oni�cie funkcji IsValidMove dla kr�la
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logik� specyficzn� dla ruchu kr�la
        // Na przyk�ad, zezwalaj na ruch o jedno pole w dowolnym kierunku
        return (std::abs(toRow - fromRow) <= 1 && std::abs(toCol - fromCol) <= 1);
    }
};
