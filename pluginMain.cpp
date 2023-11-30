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
void refactorText(int& countOfSymbols, vector<char>& text, vector<bool>& ignore, vector<bool>& immutableSymbols) {
    vector<char> newText;
    for (int currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition) {
        if (!ignore[currentPosition] || immutableSymbols[currentPosition]) newText.push_back(text[currentPosition]);
    }
    countOfSymbols = newText.size();
    ignore = vector<bool>(countOfSymbols);
    immutableSymbols = vector<bool>(countOfSymbols);
    text = newText;
    return;
}

//not used
//УДАЛЕНИЕ ЛИШНИХ ПЕРЕВОДОВ СТРОК:
void removeExtraEndLines(const int& countOfSymbols, vector<char>& text, vector<bool>& ignore) {
    //TODO
}



void setImmutableSymbols(const int& countOfSymbols, vector<char>& text, vector<bool>& immutableSymbols) {
    for (int currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition) {
        // int('"') == 34
        if (int(text[currentPosition]) == 34) {
            while (currentPosition + 1 < countOfSymbols && int(text[currentPosition + 1]) != 34) {
                immutableSymbols[currentPosition] = true;
                ++currentPosition;
            }
            ++currentPosition;
            if (currentPosition < countOfSymbols) {
                immutableSymbols[currentPosition] = true;
            }
        }
        if (currentPosition + 1 < countOfSymbols && text[currentPosition] == '/') {
            if (text[currentPosition + 1] == '/') {
                while (currentPosition + 1 < countOfSymbols && !isEndOfLine(text[currentPosition + 1])) {
                    immutableSymbols[currentPosition] = true;
                    ++currentPosition;
                }
            }
            else if (text[currentPosition + 1] == '*') {
                while (currentPosition + 2 < countOfSymbols && !(text[currentPosition + 1] == '*' && text[currentPosition + 2] == '/')) {
                    immutableSymbols[currentPosition] = true;
                    ++currentPosition;
                }
            }
        }

    }
}

//РАЗБИЕНИЕ ТЕКСТА НА СТРОКИ
void partitionTextToLines(const int& countOfSymbols, vector<char>& text, vector<bool>& immutableSymbols, vector<string>& stringsPartition) {
    int currentStringNumber = 0;
    for (int currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition) {
        if (currentStringNumber >= stringsPartition.size()) {
            stringsPartition.push_back(string(""));
        }
        if (immutableSymbols[currentPosition]) {
            stringsPartition[currentStringNumber].push_back(text[currentPosition]);
        }
        else if (isNewEndOfLine(text[currentPosition])) {
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
    vector<bool> immutableSymbols(countOfSymbols);
    removeExtraSpaceSymbols(countOfSymbols, text, ignore);
    setImmutableSymbols(countOfSymbols, text, immutableSymbols);
    refactorText(countOfSymbols, text, ignore, immutableSymbols);


    vector<string> stringsPartition;
    setImmutableSymbols(countOfSymbols, text, immutableSymbols);
    partitionTextToLines(countOfSymbols, text, immutableSymbols, stringsPartition);

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

