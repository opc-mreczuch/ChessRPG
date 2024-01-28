#pragma once

#include <string>

// Klasa bazowa reprezentuj¹ca figurê szachow¹
class ChessPiece
{
public:
    ChessPiece(std::wstring symbol, std::wstring color, int index)
        : symbol_(symbol), color_(color), index_(index) {}

    virtual std::wstring GetSymbol() const
    {
        return color_ + symbol_ + std::to_wstring(index_);
    }

    // Wirtualna funkcja do walidacji ruchów
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const
    {
        // Domyœlna implementacja zezwala na dowolny ruch
        return true;
    }

    virtual ~ChessPiece() {}

private:
    std::wstring symbol_;
    std::wstring color_;
    int index_;
};

// Klasa reprezentuj¹ca piona
class Pawn : public ChessPiece
{
public:
    Pawn(std::wstring color, int index) : ChessPiece(color == L"white" ? L"P" : L"p", color, index) {}

    // Przes³oniêcie funkcji IsValidMove dla piona
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logikê specyficzn¹ dla ruchu piona
        // Na przyk³ad, zezwalaj na przesuniêcie piona do przodu o jedno pole
        return (toRow == fromRow + 1) && (toCol == fromCol);
    }
};

// Klasa reprezentuj¹ca skoczka
class Knight : public ChessPiece
{
public:
    Knight(std::wstring color, int index) : ChessPiece(color == L"white" ? L"N" : L"n", color, index) {}

    // Przes³oniêcie funkcji IsValidMove dla skoczka
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logikê specyficzn¹ dla ruchu skoczka
        // Na przyk³ad, zezwalaj na ruch w kszta³cie litery "L"
        return ((std::abs(toRow - fromRow) == 2 && std::abs(toCol - fromCol) == 1) ||
            (std::abs(toRow - fromRow) == 1 && std::abs(toCol - fromCol) == 2));
    }
};

// Klasa reprezentuj¹ca goñca
class Bishop : public ChessPiece
{
public:
    Bishop(std::wstring color, int index) : ChessPiece(color == L"white" ? L"B" : L"b", color, index) {}

    // Przes³oniêcie funkcji IsValidMove dla goñca
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logikê specyficzn¹ dla ruchu goñca
        // Na przyk³ad, zezwalaj na ruch po przek¹tnej
        return (std::abs(toRow - fromRow) == std::abs(toCol - fromCol));
    }
};

// Klasa reprezentuj¹ca wie¿ê
class Rook : public ChessPiece
{
public:
    Rook(std::wstring color, int index) : ChessPiece(color == L"white" ? L"R" : L"r", color, index) {}

    // Przes³oniêcie funkcji IsValidMove dla wie¿y
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logikê specyficzn¹ dla ruchu wie¿y
        // Na przyk³ad, zezwalaj na ruch w pionie lub poziomie
        return (fromRow == toRow || fromCol == toCol);
    }
};

// Klasa reprezentuj¹ca królow¹
class Queen : public ChessPiece
{
public:
    Queen(std::wstring color, int index) : ChessPiece(color == L"white" ? L"Q" : L"q", color, index) {}

    // Przes³oniêcie funkcji IsValidMove dla królowej
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logikê specyficzn¹ dla ruchu królowej
        // Na przyk³ad, zezwalaj na ruch w pionie, poziomie lub po przek¹tnej
        return ((fromRow == toRow || fromCol == toCol) || (std::abs(toRow - fromRow) == std::abs(toCol - fromCol)));
    }
};

// Klasa reprezentuj¹ca króla
class King : public ChessPiece
{
public:
    King(std::wstring color, int index) : ChessPiece(color == L"white" ? L"K" : L"k", color, index) {}

    // Przes³oniêcie funkcji IsValidMove dla króla
    virtual bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) const override
    {
        // Dodaj logikê specyficzn¹ dla ruchu króla
        // Na przyk³ad, zezwalaj na ruch o jedno pole w dowolnym kierunku
        return (std::abs(toRow - fromRow) <= 1 && std::abs(toCol - fromCol) <= 1);
    }
};
