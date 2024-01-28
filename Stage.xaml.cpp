//Stage.xaml.cpp
#include "MainMenu.xaml.h"
#include "pch.h"

#include <iostream>
#include <cctype>
#include "Stage.xaml.h"
#include <sstream>
#include <wctype.h>
#include <roapi.h>
#include <wrl\client.h>
#include <ppltasks.h>
#include <fstream>
#include <string>
#include <Windows.UI.Popups.h>
#include <random>
#include <Windows.Storage.Pickers.h>
#include <PathCch.h>
#include <limits>
#include <Windows.h>
#include <Shlwapi.h>
#include <wrl\wrappers\corewrappers.h>




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
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::UI::Popups;
using namespace concurrency;


// Definicje planszy i przycisków szachowych
std::vector<std::vector<std::wstring>> Stage::chessboard;  // Plansza szachowa
std::vector<std::vector<std::wstring>> Stage::chessboard2; // Kopia planszy do przechowywania poprzedniego stanu
std::vector<std::vector<std::wstring>> chessboardWithPoints(2, std::vector<std::wstring>(16, L"0")); // Plansza z punktami dla każdego pola

std::vector<std::vector<Windows::UI::Xaml::Controls::Button^>> Stage::chessButtons; // Przyciski szachowe
std::pair<int, int> Stage::selectedButton = std::make_pair(-1, -1); // Para przechowująca aktualnie zaznaczony przycisk

Stage::Stage() : isWhiteTurn(true)
{
    InitializeComponent();

    // Inicjalizacja planszy szachowej na podstawie danych wejściowych
    chessboard = {
        { L"br1", L"bn1", L"bb1", L"bk1", L"bq1", L"bb2", L"bn2", L"br2" },
        { L"bp1", L"bp2", L"bp3", L"bp4", L"bp5", L"bp6", L"bp7", L"bp8" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"", L"", L"", L"", L"", L"", L"", L"" },
        { L"wp1", L"wp2", L"wp3", L"wp4", L"wp5", L"wp6", L"wp7", L"wp8" },
        { L"wr1", L"wn1", L"wb1", L"wk1", L"wq1", L"wb2", L"wn2", L"wr2" }
    };

    // Inicjalizacja planszy z punktami
    chessboardWithPoints = {
        { L"wp1", L"wp2", L"wp3", L"wp4", L"wp5", L"wp6", L"wp7", L"wp8",
          L"wr1", L"wn1", L"wb1", L"wq1", L"wk1", L"wb2", L"wn2", L"wr2",
          L"br1", L"bn1", L"bb1", L"bq1", L"bk1", L"bb2", L"bn2", L"br2",
          L"bp1", L"bp2", L"bp3", L"bp4", L"bp5", L"bp6", L"bp7", L"bp8" },
        { L"0", L"0", L"0", L"0", L"0", L"0", L"0", L"0",
          L"0", L"0", L"0", L"0", L"0", L"0", L"0", L"0",
          L"0", L"0", L"0", L"0", L"0", L"0", L"0", L"0",
          L"0", L"0", L"0", L"0", L"0", L"0", L"0", L"0" }
    };

    // Inicjalizacja przycisków szachowych zgodnie z planszą
    chessButtons = std::vector<std::vector<Windows::UI::Xaml::Controls::Button^>>(8, std::vector<Windows::UI::Xaml::Controls::Button^>(8, nullptr));

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            String^ buttonName = "i" + (row + 1) + "y" + (col + 1);
            Windows::UI::Xaml::Controls::Button^ chessButton = dynamic_cast<Windows::UI::Xaml::Controls::Button^>(this->FindName(buttonName));

            if (chessButton != nullptr)
            {
                // Ustawienie treści przycisku na pusty ciąg, ponieważ będą używane obrazy
                chessButton->Content = "";

                // Dodanie obsługi zdarzenia dla przycisku
                chessButton->Click += ref new RoutedEventHandler(this, &Stage::ChessButton_Click);

                chessButtons[row][col] = chessButton;

                // Ustawienie obrazu na przycisku zgodnie z figurą na planszy
                SetChessPieceImage(chessButton, chessboard[row][col]);
            }
        }
    }
}

// Obsługa zdarzenia kliknięcia przycisku szachowego
void Stage::ChessButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    Windows::UI::Xaml::Controls::Button^ clickedButton = dynamic_cast<Windows::UI::Xaml::Controls::Button^>(sender);

    // Pobranie nazwy przycisku
    Platform::String^ buttonName = clickedButton->Name;

    // Konwersja nazwy przycisku do std::wstring
    std::wstring buttonNameStr(buttonName->Data());

    // Pobranie numeru wiersza i kolumny z nazwy przycisku
    int row = buttonNameStr[1] - L'0';
    int col = buttonNameStr[3] - L'0';

    // Sprawdzenie, czy to pierwsze czy drugie wciśnięcie
    if (selectedButton.first == -1 && selectedButton.second == -1)
    {
        // Pierwsze wciśnięcie - zapisanie współrzędnych
        selectedButton = std::make_pair(row, col);
    }
    else
    {
        // Drugie wciśnięcie - zapisanie poprzedniego stanu planszy
        chessboard2 = chessboard;

        // Pobranie figury na pierwszym polu
        std::wstring& sourcePiece = chessboard[selectedButton.first - 1][selectedButton.second - 1];

        // Sprawdzenie, czy to tura aktualnego gracza
        if ((isWhiteTurn && sourcePiece[0] == L'w') || (!isWhiteTurn && sourcePiece[0] == L'b'))
        {
            // Pobranie figury na docelowym polu
            std::wstring& targetPiece = chessboard[row - 1][col - 1];

            if (targetPiece != L"ws" && targetPiece != L"bs" && sourcePiece[0] != targetPiece[0])
            {
                if ((targetPiece == L"bo" || targetPiece == L"wo") && IsValidMove(sourcePiece, selectedButton.first, selectedButton.second, row, col))
                {
                    sourcePiece = L"";
                    targetPiece = L"";
                    UpdateChessboardUI();
                    isWhiteTurn = !isWhiteTurn;
                    changeShield();
                    SprawdzPlansze();
                }
                else if (IsValidMove(sourcePiece, selectedButton.first, selectedButton.second, row, col))
                {
                    std::swap(sourcePiece, targetPiece);
                    UpdateChessboardUI();
                    isWhiteTurn = !isWhiteTurn;
                    changeShield();
                    SprawdzPlansze();
                }
            }
        }

        // Zresetowanie licznika
        selectedButton = std::make_pair(-1, -1);
    }
}


// Metoda zmieniająca tarczę króla w zależności od aktualnej tury
void Stage::changeShield()
{
    for (int row = 0; row < chessboard.size(); ++row)
    {
        for (int col = 0; col < chessboard[row].size(); ++col)
        {
            if (!isWhiteTurn && chessboard[row][col] == L"bs")
            {
                chessboard[row][col] = L"bk1";
            }
            else if (isWhiteTurn && chessboard[row][col] == L"ws")
            {
                chessboard[row][col] = L"wk1";
            }

            if (!isWhiteTurn && chessboard[row][col] == L"bo")
            {
                chessboard[row][col] = L"bq1";
            }
            else if (isWhiteTurn && chessboard[row][col] == L"wo")
            {
                chessboard[row][col] = L"wq1";
            }
        }
    }
}

// Metoda sprawdzająca aktualny stan planszy po wykonaniu ruchu
void Stage::SprawdzPlansze()
{
    // Flaga informująca o znalezieniu białego króla (wk1) i czarnego króla (bk1)
    bool wk1_found = false;
    bool bk1_found = false;

    // Przeszukiwanie planszy w poszukiwaniu królów
    for (size_t i = 0; i < chessboard.size(); ++i)
    {
        for (size_t j = 0; j < chessboard[i].size(); ++j)
        {
            if (chessboard[i][j] == L"wk1" || chessboard[i][j] == L"ws")
                wk1_found = true;
            else if (chessboard[i][j] == L"bk1" || chessboard[i][j] == L"bs")
                bk1_found = true;
        }
    }

    // Wyświetlanie komunikatów w zależności od wyników przeszukiwania
    if (!wk1_found)
    {
        auto messageDialog = ref new MessageDialog(L"Biały król został pokonany! Zwycięstwo należy do czarnych!");

        // Wywołanie ShowAsync i obsługa zdarzenia Completed
        create_task(messageDialog->ShowAsync())
            .then([=](IUICommand^ command)
                {
                    // Utworzenie nowego obiektu Page dla MainMenu.xaml
                    MainMenu^ mainMenuPage = ref new MainMenu();

                    // Pobranie bieżącej ramki (Frame) z obecnego okna
                    Windows::UI::Xaml::Controls::Frame^ frame = dynamic_cast<Windows::UI::Xaml::Controls::Frame^>(Window::Current->Content);

                    // Sprawdzenie, czy udało się pobrać ramkę
                    if (frame != nullptr)
                    {
                        // Następnie, użyj ramki do nawigacji do nowej strony
                        frame->Navigate(mainMenuPage->GetType());
                    }
                });
    }

    if (!bk1_found)
    {
        auto messageDialog = ref new MessageDialog(L"Czarny król został pokonany! Zwycięstwo należy do białych!");

        // Wywołanie ShowAsync i obsługa zdarzenia Completed
        create_task(messageDialog->ShowAsync())
            .then([=](IUICommand^ command)
                {
                    // Utworzenie nowego obiektu Page dla MainMenu.xaml
                    MainMenu^ mainMenuPage = ref new MainMenu();

                    // Pobranie bieżącej ramki (Frame) z obecnego okna
                    Windows::UI::Xaml::Controls::Frame^ frame = dynamic_cast<Windows::UI::Xaml::Controls::Frame^>(Window::Current->Content);

                    // Sprawdzenie, czy udało się pobrać ramkę
                    if (frame != nullptr)
                    {
                        // Następnie, użyj ramki do nawigacji do nowej strony
                        frame->Navigate(mainMenuPage->GetType());
                    }
                });
    }
}


// Obsługa zdarzenia kliknięcia przycisku wyjścia do menu głównego
void Stage::ExitButton_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
    // Utwórz nowy obiekt Page dla MainMenu.xaml
    MainMenu^ mainMenuPage = ref new MainMenu();

    // Pobierz bieżącą ramkę (Frame) z obecnego okna
    Windows::UI::Xaml::Controls::Frame^ frame = dynamic_cast<Windows::UI::Xaml::Controls::Frame^>(Window::Current->Content);

    // Sprawdź, czy udało się pobrać ramkę
    if (frame != nullptr)
    {
        // Następnie, użyj ramki do nawigacji do nowej strony
        frame->Navigate(mainMenuPage->GetType());
    }
}

// Obsługa zdarzenia kliknięcia przycisku cofnięcia ruchu
void Stage::UndoButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Sprawdź, czy istnieje poprzedni stan planszy
    if (!chessboard2.empty())
    {
        // Przywróć poprzedni stan planszy
        chessboard = chessboard2;

        // Zresetuj zmienną selectedButton, aby uniknąć problemów z ponownym wybieraniem figury po cofnięciu
        selectedButton = std::make_pair(-1, -1);

        // Przywróć tę samą turę, co przed wykonaniem cofnięcia
        isWhiteTurn = !isWhiteTurn;

        // Zresetuj planszę2
        chessboard2.clear();

        // Aktualizuj planszę
        UpdateChessboardUI();
    }
}

// Obsługa zdarzenia kliknięcia przycisku zapisu stanu gry
void Stage::SaveButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Pobierz folder lokalny aplikacji
    auto localFolder = Windows::Storage::ApplicationData::Current->LocalFolder;

    // Utwórz nazwę pliku na podstawie aktualnej daty i godziny
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);

    std::wstringstream ss;
    ss << L"Save_" << systemTime.wYear << L"-" << systemTime.wMonth << L"-" << systemTime.wDay
        << L"_" << systemTime.wHour << L"-" << systemTime.wMinute << L"-" << systemTime.wSecond
        << L".txt";

    std::wstring fileName = ss.str();

    // Utwórz ścieżkę do pliku w folderze lokalnym
    std::wstring filePath;
    wchar_t combinedPath[MAX_PATH];
    PathCchCombine(combinedPath, MAX_PATH, localFolder->Path->Data(), fileName.c_str());
    filePath = combinedPath;

    // Zapisz stan gry do pliku tekstowego
    concurrency::create_task(localFolder->CreateFileAsync(ref new Platform::String(fileName.c_str()), Windows::Storage::CreationCollisionOption::GenerateUniqueName))
        .then([this](Windows::Storage::StorageFile^ file)
            {
                // Utwórz strumień zapisu do pliku
                return Windows::Storage::FileIO::WriteTextAsync(file, GetChessboardState());
            })
        .then([this, filePath](concurrency::task<void> previousTask)
            {
                try
                {
                    // Przechwytywanie ewentualnych wyjątków
                    previousTask.get();
                    // Wyświetlenie powiadomienia o zapisie
                    std::wstring notificationText = L"Gra została zapisana w pliku: " + filePath;
                    Windows::UI::Popups::MessageDialog^ dialog = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(notificationText.c_str()), L"Zapisano");
                    dialog->ShowAsync();
                }
                catch (Platform::Exception^ ex)
                {
                    // Obsługa ewentualnych błędów
                    std::wstring errorText = L"Błąd podczas zapisywania gry: " + std::wstring(ex->Message->Data());

                    Windows::UI::Popups::MessageDialog^ dialog = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(errorText.c_str()), L"Błąd");
                    dialog->ShowAsync();
                }
            });
}


Platform::String^ Stage::GetChessboardState()
{
    // Twórz string reprezentujący stan tablicy chessboard
    Platform::String^ chessboardState = L"";

    // Dodaj informację o aktualnej turze
    chessboardState += (isWhiteTurn ? L"Białe" : L"Czarne") + L"\n";

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            // Dodaj informacje o każdej figurze w tablicy chessboard
            chessboardState += ref new Platform::String(chessboard[row][col].c_str()) + L"\t";
        }
        chessboardState += L"\n";
    }

    return chessboardState;
}

void Stage::LoadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Utwórz okno dialogowe do wyboru pliku
    Windows::Storage::Pickers::FileOpenPicker^ openPicker = ref new Windows::Storage::Pickers::FileOpenPicker();
    openPicker->ViewMode = Windows::Storage::Pickers::PickerViewMode::List;
    openPicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary;
    openPicker->FileTypeFilter->Append(L".txt");

    // Pokaż okno dialogowe
    concurrency::create_task(openPicker->PickSingleFileAsync()).then([this](Windows::Storage::StorageFile^ file)
        {
            if (file)
            {
                // Otwórz plik tekstowy zewnętrzny
                return concurrency::create_task(Windows::Storage::FileIO::ReadTextAsync(file));
            }
            else
            {
                // Użytkownik anulował wybór pliku
                // Tutaj możesz podjąć odpowiednie działania, jeśli potrzebujesz
                return concurrency::create_task([]() { return ref new Platform::String(L""); });
            }
        }).then([this](Platform::String^ fileContent)
            {
                // Wyświetl zawartość pliku, pomijając pierwszą linię
                std::wistringstream iss(fileContent->Data());
                std::wstring firstLine;
                std::getline(iss, firstLine);  // Pomijamy pierwszą linię

                // Wyświetl resztę zawartości pliku
                std::wstring remainingContent, board;
                while (std::getline(iss, remainingContent))
                {
                    board += L"\n" + remainingContent;
                }
                // Utwórz powiadomienie z zawartością remainingContent (cały tekst poza pierwszą linią)
                Windows::UI::Popups::MessageDialog^ messageDialog = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(board.c_str()), L"Zawartość pliku poza pierwszą linią");

                // Wyświetl powiadomienie
                concurrency::create_task(messageDialog->ShowAsync());

                // Teraz możemy przekazać board do funkcji Updateblack lub Updatewhite
                if (firstLine.find(L"Czarne") != std::wstring::npos)
                {
                    // Jeżeli pierwsza linia zawiera "Czarne", zmień turę i wczytaj planszę
                    Updateblack(board);
                }
                else if (firstLine.find(L"Białe") != std::wstring::npos)
                {
                    // Jeżeli pierwsza linia zawiera "Białe", zmień turę i wczytaj planszę
                    Updatewhite(board);
                }
            });
}


void Stage::Updateblack(const std::wstring& fileContent)
{


    // Pętle wczytujące planszę
    std::wistringstream iss(fileContent);
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            // Odczytaj figurę (oddzieloną tabulatorem)
            std::getline(iss, chessboard[row][col], L'\t');

        }
    }

    // Aktualizuj obrazy na przyciskach na podstawie nowej planszy szachowej
    UpdateChessboardUI();
}

void Stage::Updatewhite(const std::wstring& fileContent)
{
    // Pętle wczytujące planszę
    std::wistringstream iss(fileContent);
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            // Odczytaj figurę (oddzieloną tabulatorem)
            std::getline(iss, chessboard[row][col], L'\t');


            
            
        }
    }

    // Aktualizuj obrazy na przyciskach na podstawie nowej planszy szachowej
    UpdateChessboardUI();
}



void Stage::AddPointsForPiece(const std::wstring& piece)
{

        // Iteruj po wierszu pierwszym tablicy i szukaj identyfikatora figury
        for (int i = 0; i < chessboardWithPoints[0].size(); i++)
        {
            // Jeżeli znaleziono identyfikator figury, dodaj 1 punkt do komórki o 1 niżej
            if (chessboardWithPoints[0][i] == piece)
            {
                chessboardWithPoints[1][i] = std::to_wstring(std::stoi(chessboardWithPoints[1][i]) + 1);
                break;
            }
        }

}



void Stage::SetChessPieceImage(Windows::UI::Xaml::Controls::Button^ button, const std::wstring& piece)
{
    // Sprawdź, czy nazwa figury ma co najmniej dwa znaki
    if (piece.size() < 2)
    {
        // Puste pole - nie rysuj obrazu
        button->Content = nullptr;
        return;
    }

    // Pobierz dwie pierwsze litery z nazwy figury
    std::wstring pieceCode = piece.substr(0, 2);

    // Określ kolor figury
    wchar_t pieceColor = pieceCode[0];

    // Określ typ figury
    wchar_t pieceType = pieceCode[1];

    // Zbuduj nazwę pliku obrazu na podstawie koloru i typu figury
    Platform::String^ imagePath = "ms-appx:///Assets/";

    // Dodaj kolor do ścieżki
    imagePath += (pieceColor == L'w') ? "White" : "Black";

    // Dodaj typ figury do ścieżki
    switch (pieceType)
    {
    case L'p':
        imagePath += "Pawn";
        break;
    case L'r':
        imagePath += "Rook";
        break;
    case L'n':
        imagePath += "Knight";
        break;
    case L'b':
        imagePath += "Bishop";
        break;
    case L'q':
        imagePath += "Queen";
        break;
    case L'k':
        imagePath += "King";
        break;
    case L's':
        imagePath += "Shield";
        break;
    case L'o':
        imagePath += "Fire";
        break;
    default:
        // Puste pole - nie rysuj obrazu
        button->Content = nullptr;
        return;
    }

    // Dodaj rozszerzenie pliku obrazu
    imagePath += ".png";

    // Ustaw obraz na przycisku
    auto uri = ref new Windows::Foundation::Uri(imagePath);
    auto bitmap = ref new Windows::UI::Xaml::Media::Imaging::BitmapImage(uri);

    // Użyj auto dla dynamic_cast
    auto imageControl = ref new Windows::UI::Xaml::Controls::Image();
    imageControl->Source = bitmap;
    button->Content = imageControl;
}


void Stage::UpdateChessboardUI()
{
    Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(
        Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this]()
            {
                // Aktualizuj obrazy na przyciskach na podstawie planszy szachowej
                for (int row = 0; row < 8; row++)
                {
                    for (int col = 0; col < 8; col++)
                    {

                        // Ustaw obraz na przycisku zgodnie z planszą szachową
                        SetChessPieceImage(chessButtons[row][col], chessboard[row][col]);
                    }
                }
            }));
}


bool Stage::IsValidMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol)
{
    wchar_t pieceColor = piece[0]; // Kolor figury
    wchar_t pieceType = towupper(piece[1]); // Typ figury (dużej literze)

    // Sprawdź, czy ruch mieści się w granicach szachownicy
    if (fromRow < 1 || fromRow > 8 || fromCol < 1 || fromCol > 8 ||
        toRow < 1 || toRow > 8 || toCol < 1 || toCol > 8) {
        return false;
    }

    // Sprawdź, czy to tura odpowiedniego koloru
    if ((isWhiteTurn && pieceColor != L'w') || (!isWhiteTurn && pieceColor != L'b')) {
        return false;
    }
    else if (pieceType == L'S') // Shield
    {
        return false; // Figura "shield" nie może się poruszać
    }

    // Rozpoznaj typ figury i sprawdź zasady ruchu
    switch (pieceType)
    {
    case L'P': // Pion
        return IsValidPawnMove(piece, fromRow, fromCol, toRow, toCol);

    case L'R': // Wieża
        return IsValidRookMove(piece, fromRow, fromCol, toRow, toCol);

    case L'N': // Skoczek
        return IsValidKnightMove(piece, fromRow, fromCol, toRow, toCol);

    case L'B': // Goniec
        return IsValidBishopMove(piece, fromRow, fromCol, toRow, toCol);

    case L'Q': // Hetman
        return IsValidQueenMove(piece, fromRow, fromCol, toRow, toCol);

    case L'K': // Król
        return IsValidKingMove(piece, fromRow, fromCol, toRow, toCol);


    default:
        // Nieznany typ figury
        return false;
    }
}


bool Stage::IsValidPawnMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol)
{
    wchar_t pieceColor = piece[0]; // Kolor pionka

    // Sprawdź, czy pole docelowe zawiera figurę przeciwnego koloru
    if (chessboard[toRow - 1][toCol - 1] != L"" && chessboard[toRow - 1][toCol - 1][0] != pieceColor)
    {
        // Eliminuj przeciwną figurę tylko jeśli ruch jest po skosie
        if (std::abs(toCol - fromCol) == 1 && std::abs(toRow - fromRow) == 1)
        {
            if (chessboard[toRow - 1][toCol - 1] != L"")
            {
                AddPointsForPiece(piece);

                for (int i = 0; i < chessboardWithPoints[1].size(); i++)
                {
                    if (chessboardWithPoints[1][i] == L"2" && chessboardWithPoints[0][i] == piece)
                    {
                        // Zamiast rand():
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<int> distrib(1, 3);
                        int random = distrib(gen);

                        if (random == 1) {
                            // Zamiana figury na skoczka
                            std::wstring newPiece = (pieceColor == L'w') ? L"wn" : L"bn";
                            chessboard[fromRow - 1][fromCol - 1] = newPiece;
                        }
                        else if (random == 2) {
                            // Zamiana figury na gońca
                            std::wstring newPiece = (pieceColor == L'w') ? L"wb" : L"bb";
                            chessboard[fromRow - 1][fromCol - 1] = newPiece;
                        }
                        else {
                            // Zamiana figury na wieżę
                            std::wstring newPiece = (pieceColor == L'w') ? L"wr" : L"br";
                            chessboard[fromRow - 1][fromCol - 1] = newPiece;
                        }

                    }

                }
            }

            chessboard[toRow - 1][toCol - 1].clear(); // Eliminuj przeciwną 
            return true;
        }
        else
        {
            // Ruch dozwolony tylko po skosie (bicie)
            return false;
        }
    }

    // Piony poruszają się do przodu o jedno pole, ale mogą wykonywać pierwszy ruch o dwa pola
    if (pieceColor == L'w') // Pion biały
    {
        if (fromCol == toCol && fromRow - toRow == 1) // Ruch do przodu o jedno pole
            return true;
        else if (fromCol == toCol && fromRow == 7 && toRow == 5) // Pierwszy ruch o dwa pola
            return true;
    }
    else if (pieceColor == L'b') // Pion czarny
    {
        if (fromCol == toCol && toRow - fromRow == 1) // Ruch do przodu o jedno pole
            return true;
        else if (fromCol == toCol && fromRow == 2 && toRow == 4) // Pierwszy ruch o dwa pola
            return true;
    }

    // Ruch nie jest dozwolony dla piona
    return false;
}



bool Stage::IsValidKnightMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol)
{
    // Sprawdź, czy ruch figury skoczka jest dozwolony
    int rowDiff = std::abs(toRow - fromRow);
    int colDiff = std::abs(toCol - fromCol);

    // Sprawdź, czy skoczek porusza się w "L" i nie wchodzi na pole figury tego samego koloru
    if ((rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2))
    {
        // Sprawdź, czy pole docelowe jest puste lub zajęte przez przeciwną figurę
        if (chessboard[toRow - 1][toCol - 1] == L"" || chessboard[toRow - 1][toCol - 1][0] != piece[0])
        {

            if (chessboard[toRow - 1][toCol - 1] != L"")
            {
                AddPointsForPiece(piece);

                for (int i = 0; i < chessboardWithPoints[1].size(); i++)
                {
                    // Jeżeli znaleziono identyfikator figury, dodaj 1 punkt do komórki o 1 niżej
                    if (chessboardWithPoints[1][i] == L"3" && chessboardWithPoints[0][i] == piece)
                    {
                        std::wstring newPiece = (piece[0] == L'w') ? L"wq" : L"bq";
                        chessboard[fromRow - 1][fromCol - 1] = newPiece;
                    }
                }
            }
            chessboard[toRow - 1][toCol - 1].clear();
            return true;
        }
    }

    return false; // Ruch nie jest dozwolony
}


bool Stage::IsValidBishopMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol)
{
    // Sprawdź, czy ruch figury gońca jest dozwolony
    int rowDiff = toRow - fromRow;
    int colDiff = toCol - fromCol;

    // Sprawdź, czy ruch jest po skosie
    if (std::abs(rowDiff) != std::abs(colDiff))
    {
        return false; // Ruch nie jest po skosie
    }

    // Określ kierunek ruchu (w prawo-górę, w lewo-górę, w lewo-dół, w prawo-dół)
    int rowDirection = (rowDiff > 0) ? 1 : -1;
    int colDirection = (colDiff > 0) ? 1 : -1;

    // Sprawdź, czy na trasie ruchu nie ma przeszkód
    int checkRow = fromRow + rowDirection;
    int checkCol = fromCol + colDirection;

    while (checkRow != toRow && checkCol != toCol)
    {
        // Jeśli na trasie ruchu znajduje się figura, zwróć false
        if (chessboard[checkRow - 1][checkCol - 1] != L"" || (checkCol != toCol && chessboard[toRow - 1][checkCol - 1][0] == piece[0]))
        {
            return false; // Jest przeszkoda na trasie ruchu lub figura tego samego koloru
        }

        // Przesuń się do kolejnego pola na trasie ruchu
        checkRow += rowDirection;
        checkCol += colDirection;
    }

    // Sprawdź, czy pole docelowe jest puste lub zajęte przez przeciwną figurę
    if (chessboard[toRow - 1][toCol - 1] == L"" || chessboard[toRow - 1][toCol - 1][0] != piece[0])
    {
        // Eliminuj przeciwną figurę, jeśli istnieje
        if (chessboard[toRow - 1][toCol - 1] != L"")
        {
            AddPointsForPiece(piece);

            for (int i = 0; i < chessboardWithPoints[1].size(); i++)
            {
                if (chessboardWithPoints[1][i] == L"3" && chessboardWithPoints[0][i] == piece)
                {
                    std::wstring newPiece = (piece[0] == L'w') ? L"wq" : L"bq";
                    chessboard[fromRow - 1][fromCol - 1] = newPiece;
                }
            }
        }
        chessboard[toRow - 1][toCol - 1].clear();
        return true;
    }

    return false; // Ruch nie jest dozwolony
}





bool Stage::IsValidRookMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol)
{
    // Sprawdź, czy ruch odbywa się w jednym wierszu lub jednej kolumnie
    if (fromRow != toRow && fromCol != toCol) {
        return false;
    }

    // Sprawdź, czy na trasie ruchu nie ma żadnych przeszkód
    if (fromRow == toRow) // Ruch w jednym wierszu
    {
        int startCol = (fromCol < toCol) ? fromCol + 1 : toCol + 1;
        int endCol = (fromCol < toCol) ? toCol : fromCol;
        for (int col = startCol; col < endCol; col++) {
            if (chessboard[fromRow - 1][col - 1] != L"" || (col != toCol && chessboard[toRow - 1][col - 1][0] == piece[0])) {
                return false; // Jest przeszkoda na trasie ruchu lub figura tego samego koloru
            }
        }
    }
    else // Ruch w jednej kolumnie
    {
        int startRow = (fromRow < toRow) ? fromRow + 1 : toRow + 1;
        int endRow = (fromRow < toRow) ? toRow : fromRow;
        for (int row = startRow; row < endRow; row++) {
            if (chessboard[row - 1][fromCol - 1] != L"" || (row != toRow && chessboard[row - 1][toCol - 1][0] == piece[0])) {
                return false; // Jest przeszkoda na trasie ruchu lub figura tego samego koloru
            }
        }
    }

    // Sprawdź, czy na docelowym polu znajduje się figura przeciwnego koloru
    if (chessboard[toRow - 1][toCol - 1] != L"" && chessboard[toRow - 1][toCol - 1][0] == piece[0]) {
        return false; 
    }

    // Eliminuj przeciwną figurę, jeśli istnieje
    if (chessboard[toRow - 1][toCol - 1] != L"")
    {
        AddPointsForPiece(piece);

        for (int i = 0; i < chessboardWithPoints[1].size(); i++)
        {
            // Jeżeli znaleziono identyfikator figury, dodaj 1 punkt do komórki o 1 niżej
            if (chessboardWithPoints[1][i] == L"3" && chessboardWithPoints[0][i] == piece)
            {
                std::wstring newPiece = (piece[0] == L'w') ? L"wq" : L"bq";
                chessboard[fromRow - 1][fromCol - 1] = newPiece;
            }
        }
    }
    chessboard[toRow - 1][toCol - 1].clear();

    return true; // Ruch jest dozwolony dla wieży
}


bool Stage::IsValidQueenMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol)
{
    // Sprawdź, czy ruch figury królowej jest dozwolony
    int rowDiff = std::abs(toRow - fromRow);
    int colDiff = std::abs(toCol - fromCol);

    // Ruch królowej jest dozwolony, jeśli spełnione są warunki dla gońca lub wieży
    if ((rowDiff == colDiff) || (fromRow == toRow) || (fromCol == toCol))
    {
        // Określ kierunek ruchu (w prawo-górę, w lewo-górę, w lewo-dół, w prawo-dół lub poziomo/wertykalnie)
        int rowDirection = (rowDiff > 0) ? (toRow > fromRow ? 1 : -1) : 0;
        int colDirection = (colDiff > 0) ? (toCol > fromCol ? 1 : -1) : 0;

        // Sprawdź, czy na trasie ruchu nie ma przeszkód
        int checkRow = fromRow + rowDirection;
        int checkCol = fromCol + colDirection;

        while (checkRow != toRow || checkCol != toCol)
        {
            // Jeśli na trasie ruchu znajduje się figura, zwróć false
            if (chessboard[checkRow - 1][checkCol - 1] != L"" || (checkCol != toCol && chessboard[toRow - 1][checkCol - 1][0] == piece[0]))
            {
                return false; // Jest przeszkoda na trasie ruchu lub figura tego samego koloru
            }

            // Przesuń się do kolejnego pola na trasie ruchu
            checkRow += rowDirection;
            checkCol += colDirection;
        }

        // Sprawdź, czy na docelowym polu znajduje się figura przeciwnego koloru
        if (chessboard[toRow - 1][toCol - 1] != L"" && chessboard[toRow - 1][toCol - 1][0] == piece[0]) {
            return false; // Na docelowym polu znajduje się figura tego samego koloru co poruszająca się królowa
        }

        // Eliminuj przeciwną figurę, jeśli istnieje
        if (chessboard[toRow - 1][toCol - 1] != L"")
        {
            AddPointsForPiece(piece);

            for (int i = 0; i < chessboardWithPoints[1].size(); i++)
            {
                if (std::stoi(chessboardWithPoints[1][i]) % 3 == 0 && chessboardWithPoints[0][i] == piece)
                {
                    std::wstring newPiece = (piece[0] == L'w') ? L"wo" : L"bo";
                    chessboard[fromRow - 1][fromCol - 1] = newPiece;
                }
            }
        }
        chessboard[toRow - 1][toCol - 1].clear();
        return true;
    }

    return false; // Ruch nie jest dozwolony
}




bool Stage::IsValidKingMove(const std::wstring& piece, int fromRow, int fromCol, int toRow, int toCol)
{
    // Sprawdź, czy ruch figury króla jest dozwolony
    int rowDiff = std::abs(toRow - fromRow);
    int colDiff = std::abs(toCol - fromCol);

    // Ruch króla jest dozwolony, jeśli przesunięcie wiersza i kolumny wynosi co najwyżej 1
    if (rowDiff <= 1 && colDiff <= 1)
    {
        // Sprawdź, czy na docelowym polu znajduje się figura przeciwnego koloru
        if (chessboard[toRow - 1][toCol - 1] != L"" && chessboard[toRow - 1][toCol - 1][0] == piece[0]) {
            return false; // Na docelowym polu znajduje się figura tego samego koloru co poruszający się król
        }

        // Eliminuj przeciwną figurę, jeśli istnieje
        if (chessboard[toRow - 1][toCol - 1] != L"")
        {
            AddPointsForPiece(piece);

            for (int i = 0; i < chessboardWithPoints[1].size(); i++)
            {
                if (std::stoi(chessboardWithPoints[1][i]) % 3 == 0 && chessboardWithPoints[0][i] == piece)
                {
                    std::wstring newPiece = (piece[0] == L'w') ? L"ws" : L"bs";
                    chessboard[fromRow - 1][fromCol - 1] = newPiece;
                }
            }
        }
        chessboard[toRow - 1][toCol - 1].clear();
        return true;
    }

    return false; // Ruch nie jest dozwolony
}



