//Author: https://github.com/SamFangshan
#include <string>
#include <cctype>
#include "grid.h"
#include "lexicon.h"
#include "set.h"
#include "vector.h"
#include "set.h"
#include "stack.h"
#include "shuffle.h"
#include "random.h"
#include "bogglegui.h"
using namespace std;

/* Define a data structure that will be useful for keeping track of coordinates
 */
struct Coordinate {
    int row;
    int column;

};

bool operator <(const Coordinate& c1, const Coordinate& c2) {
    return c1.row < c2.row || (c1.row == c2.row && c1.column < c2.column);
}

/* generates a random board for the game
 */
Grid<char> generateRandomBoard(int size, const Vector<std::string>& letterCubes) {
    Grid<char> board(size, size);
    Set<Coordinate> filled;
    for (int i = 0; i < letterCubes.size(); i++) {
        //determines the letter to be filled from the cube
        string cube = letterCubes[i];
        string cubeShuffled = shuffle(cube);
        char letter = stringToChar(cubeShuffled.substr(0, 1));
        //tentatively chooses the location to put the letter value
        int r = randomInteger(0, size - 1);
        int c = randomInteger(0, size - 1);
        Coordinate numberComb;
        numberComb.row = r;
        numberComb.column = c;
        //if this location is filled before, keep picking another one until a valid location is found
        while (filled.contains(numberComb)) {
            r = randomInteger(0, size - 1);
            c = randomInteger(0, size - 1);
            numberComb.row = r;
            numberComb.column = c;
        }
        //if the location is not filled, then fill it with the letter
        board[r][c] = letter;
        filled.add(numberComb); //keep a record: I HAVE FILLED THIS LOCATION!!
    }
    return board;
}

void humanWordSearchHelper(const Grid<char>& board, Stack<char>& wordChars, Set<Coordinate>& chosen, bool& found);
bool humanWordSearchHelperNeedHelp(const Grid<char>& board, Stack<char>& wordChars, Set<Coordinate>& chosen, Coordinate& preLoc, bool& found);

/* does the human word search
 * 1. determines the validity of the word designated by the player
 * 2. searches the word designated by the player
 * 3. adds corresponding scores according to the length of the word
 * PS: helper function written to do the recursive job
 */
bool humanWordSearch(const Grid<char>& board, const Lexicon& dictionary, const string& word) {
    BoggleGUI::clearHighlighting();  //clears highlight from the last play

    //eliminates invalid words
    if ((word.length() < BoggleGUI::MIN_WORD_LENGTH) || !dictionary.contains(word)) {
        return false;
    }

    //char-izes the word, to be stored in a stack
    Stack<char> wordChars;
    for (int i = (int)word.length() - 1; i >= 0; i--) {
        char letter = stringToChar(word.substr(i, 1));
        char letterU = toupper(letter);  //converts to uppercase for standardization
        wordChars.push(letterU);
    }

    //real recursive part: for search the designated word in the board
    Set<Coordinate> chosen;
    bool found = false;
    humanWordSearchHelper(board, wordChars, chosen, found);

    //if the word is found in the board, adds score according to the length
    if (found) {
        int length = word.length();
        if (length == 4) {
            BoggleGUI::scorePointsHuman(1);
        } else if (length == 5) {
            BoggleGUI::scorePointsHuman(2);
        } else if (length == 6) {
            BoggleGUI::scorePointsHuman(3);
        } else if (length == 7) {
            BoggleGUI::scorePointsHuman(5);
        } else {
            BoggleGUI::scorePointsHuman(11);
        }
    }

    return found;
}

/* helper funtion to do the real recursive search job
 * 1. traverses the board & highlights relevant elements
 * 2. if an element(letter) equals to the starting letter of the designated word, chooses it as the origin of the search
 * PS: helper function written to continue the pathway
 */
void humanWordSearchHelper(const Grid<char>& board, Stack<char>& wordChars, Set<Coordinate>& chosen, bool& found) {
    bool shouldExit = false;  //a boolean value designed for the convenience of exiting when the wanted word is found

    char startingLetter = wordChars.pop();
    for (int r = 0; r < board.numRows(); r++) {
        for (int c = 0; c < board.numCols(); c++) {
            //recursion: if a valid origin of pathway is found
            if (board[r][c] == startingLetter) {
                //choose
                Coordinate loc {r, c};
                chosen.add(loc);
                BoggleGUI::setHighlighted(r, c, true);

                //search
                shouldExit = humanWordSearchHelperNeedHelp(board, wordChars, chosen, loc, found);
                if (shouldExit) {
                    break;  //exits if found
                }

                //un-choose
                chosen.remove(loc);
                BoggleGUI::setHighlighted(r, c, false);
            //simply highlights to show the track of traversing
            } else {
                BoggleGUI::setHighlighted(r, c, true);
                BoggleGUI::setHighlighted(r, c, false);
            }
        }
        if (shouldExit) {
            break;  //exits if found
        }
    }

}

/* follows the original point to complete the search along a pathway
 * !!NESTED BACKTRACKING PATTERN
 */
bool humanWordSearchHelperNeedHelp(const Grid<char>& board, Stack<char>& wordChars, Set<Coordinate>& chosen, Coordinate& preLoc, bool& found) {
    //word is found
    if (wordChars.isEmpty()) {
        found = true;
        return true;
    //word hasn't been found
    } else {
        bool shouldExit = false;  //boolean value serving the same function as described in humanWordSearchHelper();
        int row = preLoc.row;
        int col = preLoc.column;

        //CHOOSE
        char letterToSearch = wordChars.pop();
        //EXPLORE
        for (int r = row - 1; r <= row + 1; r++) {
            for (int c = col - 1; c <= col + 1; c++) {
                if (board.inBounds(r, c)) {
                    Coordinate loc {r, c};
                    if (!chosen.contains(loc)) {  //to ensure the current location is not chosen
                        //choose
                        chosen.add(loc);
                        BoggleGUI::setHighlighted(loc.row, loc.column, true);

                        //explore
                        if (board[r][c] == letterToSearch) {
                            shouldExit = humanWordSearchHelperNeedHelp(board, wordChars, chosen, loc, found);
                        }
                        if (shouldExit) {
                            return true;  //exits if found
                        }

                        //un-choose
                        chosen.remove(loc);
                        BoggleGUI::setHighlighted(loc.row, loc.column, false);
                    }
                }
            }
        }
        //UN-CHOOSE
        wordChars.push(letterToSearch);
    }
    return false;
}

void computerWordSearchHelper(const Grid<char>& board, const Lexicon& dictionary, const Set<string>& humanWords, Set<string>& foundWords, string& wordStr, Set<Coordinate>& chosen, Coordinate& preLoc);

/* does the computer word searches
 * 1. explores the board to determine possible words(traverses the board for starting points to begin with)
 * 2. adds corresponding scores according to the length of found words
 * PS: helper function written to do the recursive job
 */
Set<string> computerWordSearch(const Grid<char>& board, const Lexicon& dictionary, const Set<string>& humanWords) {
    Set<string> foundWords;
    for (int r = 0; r < board.numCols(); r++) {
        for (int c = 0; c < board.numCols(); c++) {
            string wordStr = charToString(board[r][c]);
            Set<Coordinate> chosen;
            Coordinate loc{r, c};
            //choose
            chosen.add(loc);

            //search
            computerWordSearchHelper(board, dictionary, humanWords, foundWords, wordStr, chosen, loc);

            //un-choose
            chosen.remove(loc);
        }
    }
    return foundWords;
}

void computerWordSearchHelper(const Grid<char>& board, const Lexicon& dictionary, const Set<string>& humanWords, Set<string>& foundWords, string& wordStr, Set<Coordinate>& chosen, Coordinate& preLoc) {
    int row = preLoc.row;
    int col = preLoc.column;
    for (int r = row - 1; r <= row + 1; r++) {
        for (int c = col - 1; c <= col + 1; c++) {
            if (board.inBounds(r, c)) {
                Coordinate loc{r, c};
                if (!chosen.contains(loc)) {
                    //choose
                    chosen.add(loc);
                    string next = charToString(board[r][c]);
                    wordStr += next;
                    //explore
                    if (dictionary.containsPrefix(wordStr)) {  //determines the possibility for continued exploring
                        //if this string is a valid word
                        if (wordStr.length() >= BoggleGUI::MIN_WORD_LENGTH && dictionary.contains(wordStr)) {
                            //if the word is a not yet found word
                            if (!humanWords.contains(wordStr) && !foundWords.contains(wordStr)) {
                                foundWords.add(wordStr);

                                int length1 = wordStr.length();
                                if (length1 == 4) {
                                    BoggleGUI::scorePointsComputer(1);
                                } else if (length1 == 5) {
                                    BoggleGUI::scorePointsComputer(2);
                                } else if (length1 == 6) {
                                    BoggleGUI::scorePointsComputer(3);
                                } else if (length1 == 7) {
                                    BoggleGUI::scorePointsComputer(5);
                                } else {
                                    BoggleGUI::scorePointsComputer(11);
                                }
                                //explore one more time in case the word found is a sub-word of another word
                                computerWordSearchHelper(board, dictionary, humanWords, foundWords, wordStr, chosen, loc);
                            //if the word is found before, then explore in case of sub-word situation
                            } else {
                                computerWordSearchHelper(board, dictionary, humanWords, foundWords, wordStr, chosen, loc);
                            }
                        //if it is not yet a valid word, explore
                        } else {
                            computerWordSearchHelper(board, dictionary, humanWords, foundWords, wordStr, chosen, loc);
                        }
                    }
                    //un-choose
                    chosen.remove(loc);
                    int strLength = wordStr.length();
                    string substr = wordStr.substr(0, strLength - 1);
                    wordStr = substr;
                }
            }
        }
    }
}
