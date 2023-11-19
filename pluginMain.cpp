#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

bool isSpace(char& c) {
    return (c == ' ');
}

bool isEndOfLine(char& c) {
    return (c == '\n') || (c == '\r') || (c == '\r\n');
}

bool isNewEndOfLine(char& c) {
    return (c == ';') || (c =='{') || (c == '}');
}

//один таб - 4 пробела
const string oneTab = "    ";

//not used
//ОТДЕЛЕНИЕ КОММЕНТАРИЕВ В КОДЕ:
void markComments(const int& countOfSymbols, vector<char>& text, vector<bool>& inCommentary){
    for (int currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition) {
        if (text[currentPosition] == '/' && currentPosition + 1 < countOfSymbols && text[currentPosition + 1] == '*') {
            currentPosition += 2;
            while (currentPosition < countOfSymbols && !(currentPosition == '*' && currentPosition + 1 < countOfSymbols && text[currentPosition + 1] == '/')) {
                inCommentary[currentPosition] = true;
            }
        }
    }
}

//УДАЛЕНИЕ ЛИШНИХ ПРОБЕЛОВ:
void removeExtraSpaceSymbols(const int& countOfSymbols, vector<char>& text, vector<bool>& ignore) {
    for (int currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition) {
        if (ignore[currentPosition]) continue;
        if (currentPosition == 0 || isNewEndOfLine(text[currentPosition])) {    
            if (currentPosition == 0 && isSpace(text[currentPosition])) {
                ignore[currentPosition] = true;
            }
            while (currentPosition + 1 < countOfSymbols && isSpace(text[currentPosition + 1])) {
                ++currentPosition;
                ignore[currentPosition] = true;
            }
        }
        if (isSpace(text[currentPosition])) {
            while (currentPosition + 1 < countOfSymbols && isSpace(text[currentPosition + 1])) {
                ++currentPosition;
                ignore[currentPosition] = true;
            }
        }
        if (isEndOfLine(text[currentPosition])) {
            while (currentPosition + 1 < countOfSymbols && isSpace(text[currentPosition + 1])) {
                ++currentPosition;
                ignore[currentPosition] = true;
            }
        }
        if (currentPosition + 1 < countOfSymbols && (text[currentPosition] == '/' && text[currentPosition + 1] == '/')) {
            while (currentPosition - 1 >= 0 && isSpace(text[currentPosition - 1]) && !ignore[currentPosition - 1]) {
                --currentPosition;
                ignore[currentPosition] = true;
            }
        }
    }
    return;
}

//УДАЛЕНИЕ ИЗ ТЕКСТА ИГНОРИРУЕМЫХ СИМВОЛОВ:
void refactorText(int& countOfSymbols, vector<char>& text, vector<bool>& ignore) {
    vector<char> newText;
    for (int currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition) {
        if (!ignore[currentPosition]) newText.push_back(text[currentPosition]);
    }
    ignore.clear();
    countOfSymbols = newText.size();
    text = newText;
    return;
}

//not used
//УДАЛЕНИЕ ЛИШНИХ ПЕРЕВОДОВ СТРОК:
void removeExtraEndLines(const int& countOfSymbols, vector<char>& text, vector<bool>& ignore, vector<bool>& neverignore) {
    //TODO
}

//РАЗБИЕНИЕ ТЕКСТА НА СТРОКИ
void partitionTextToLines(const int& countOfSymbols, vector<char>& text, vector<string>& stringsPartition) {
    int currentStringNumber = 0;
    for (int currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition) {
        if (currentStringNumber >= stringsPartition.size()) {
            stringsPartition.push_back(string(""));
        }
        if (isNewEndOfLine(text[currentPosition])) {
            if (currentPosition + 1 < countOfSymbols && text[currentPosition + 1] == '/') {
                stringsPartition[currentStringNumber].push_back(text[currentPosition]);
                ++currentPosition;
                //добавить 3 таба перед комментарием
                for (int i = 0; i < 3; ++i) {
                    stringsPartition[currentStringNumber].append(oneTab);
                }
                while (currentPosition < countOfSymbols && !isEndOfLine(text[currentPosition])) {
                    stringsPartition[currentStringNumber].push_back(text[currentPosition]);
                    ++currentPosition;
                }
                stringsPartition[currentStringNumber].push_back('\n');
                ++currentStringNumber;
            }
            else {
                stringsPartition[currentStringNumber].push_back(text[currentPosition]);
                stringsPartition[currentStringNumber].push_back('\n');
                if (currentPosition + 1 < countOfSymbols && isEndOfLine(text[currentPosition + 1])) {
                    ++currentPosition;
                }
                ++currentStringNumber;
            }
        }
        else if (isEndOfLine(text[currentPosition])) {
            stringsPartition[currentStringNumber].push_back('\n');
            ++currentStringNumber;
        }
        else {
            stringsPartition[currentStringNumber].push_back(text[currentPosition]);
        }
    }
}

//ОПРЕДЕЛЕНИЕ УРОВНЯ ВЛОЖЕННОСТИ:
void setLevelOfNesting(const int& countOfStrings, vector<string>& stringsPartition, vector<int>& levelOfNesting) {
    int currentLevel = 0;
    for (int currentStringNumber = 0; currentStringNumber < countOfStrings; ++currentStringNumber) {
        levelOfNesting[currentStringNumber] = currentLevel;
        int localBalance = 0;
        for (char& Symbol : stringsPartition[currentStringNumber]) {
            if (Symbol == '{') {
                ++currentLevel;
                ++localBalance;
            }
            if (Symbol == '}') {
                --localBalance;
                --currentLevel;
                if (localBalance < 0) {
                    --levelOfNesting[currentStringNumber];
                }
            }
        }
    }
}

int main()
{
    ifstream fin("in.txt");
    ofstream fout("out.txt");
    vector<char> text;
    char t;
    while (fin.get(t)) {
        text.push_back(t);
    }
    int countOfSymbols = text.size();               //изначальное количество символов в алфавите
    vector<bool> ignore(countOfSymbols);            //указание, какие символы требуется игнорировать
    removeExtraSpaceSymbols(countOfSymbols, text, ignore);
    refactorText(countOfSymbols, text, ignore);

    vector<bool> neverignore(countOfSymbols);               //not used

    vector<string> stringsPartition;
    partitionTextToLines(countOfSymbols, text, stringsPartition);

    int countOfStrings = stringsPartition.size();
    vector<int> levelOfNesting(countOfStrings);
    setLevelOfNesting(countOfStrings, stringsPartition, levelOfNesting);

    for (int currentStringNumber = 0; currentStringNumber < countOfStrings; ++currentStringNumber) {
        for (int i = 0; i < levelOfNesting[currentStringNumber]; ++i) {
            fout << oneTab;
        }
        fout << stringsPartition[currentStringNumber];
    }
    
    fin.close();
    fout.close();
    return 0;
}

