#pragma once

#include <string>

// Klasa bazowa reprezentuj¹ca figurê szachow¹
class ChessPiece
{
public:
    ChessPiece(std::wstring symbol, std::wstring color, int index) : symbol_(symbol), color_(color), index_(index) {}

    virtual std::wstring GetSymbol() const
    {
        return color_ + symbol_ + std::to_wstring(index_);
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
};

// Klasa reprezentuj¹ca skoczka
class Knight : public ChessPiece
{
public:
    Knight(std::wstring color, int index) : ChessPiece(color == L"white" ? L"N" : L"n", color, index) {}
};

// Klasa reprezentuj¹ca goñca
class Bishop : public ChessPiece
{
public:
    Bishop(std::wstring color, int index) : ChessPiece(color == L"white" ? L"B" : L"b", color, index) {}
};

// Klasa reprezentuj¹ca wie¿ê
class Rook : public ChessPiece
{
public:
    Rook(std::wstring color, int index) : ChessPiece(color == L"white" ? L"R" : L"r", color, index) {}
};

// Klasa reprezentuj¹ca królow¹
class Queen : public ChessPiece
{
public:
    Queen(std::wstring color, int index) : ChessPiece(color == L"white" ? L"Q" : L"q", color, index) {}
};

// Klasa reprezentuj¹ca króla
class King : public ChessPiece
{
public:
    King(std::wstring color, int index) : ChessPiece(color == L"white" ? L"K" : L"k", color, index) {}
};
