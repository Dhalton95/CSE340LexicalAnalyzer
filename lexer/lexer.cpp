/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <stdlib.h>

#include "lexer.h"
#include "inputbuf.h"


using namespace std;

string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR", "REALNUM", "BASE08NUM",
    "BASE16NUM"
};

#define KEYWORDS_COUNT 5
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
    << reserved[(int) this->token_type] << " , "
    << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;
    
    input.GetChar(c);
    line_no += (c == '\n');
    
    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }
    
    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    
    bool canBeBase8 = true;
    
    char c;
    
    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                
                if (c >= '8')
                {
                    canBeBase8 = false;
                }
                
                input.GetChar(c);
            }
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
        
        // If the first digit we see is a zero
        if (tmp.lexeme == "0")
        {
            tmp.token_type = NUM;
            // Read the very next character
            input.GetChar(c);
            // If it is a digit, we know the lexeme should stay "0" and the token type is NUM
            if (isdigit(c))
            {
                tmp.token_type = NUM;
                input.UngetChar(c);
            }
            // If it is a dot
            else if (c == '.')
            {
                // Create a temporary Lexeme string and add the dot
                string tempLex = "0.";
                // Create a boolean to determine if we have seen a nonzero and a count for all zeros that follow the dot
                int zeroCount = 0;
                bool seenNonZero = false;
                bool hasDigits = false;
                
                // Get the next character
                input.GetChar(c);
                
                // While it is not the end of the file and the most recently read character is a digit
                while (!input.EndOfInput() && isdigit(c))
                {
                    hasDigits = true;
                    // If any character is not zero
                    if (c != '0')
                    {
                        // Set seenNonZero to true
                        seenNonZero = true;
                    }
                    
                    // If we have not seen any non-zeros after the dot yet
                    if (seenNonZero == false)
                    {
                        // Keep track of how many zeros we do see after the dot
                        zeroCount++;
                    }
                    // Concatenate each digit (zero or not) ontp the temporary lexeme string
                    tempLex += c;
                    
                    // Get a new character on each loop iteration
                    input.GetChar(c);
                }
                
                // If it is not the end of the input
                if (!input.EndOfInput())
                {
                    // Unget the most recent character
                    input.UngetChar(c);
                }
                
                // If we never saw a non-zero after the dot
                if (seenNonZero == false)
                {
                    
                    // Loop and unget all of the zeros we read after the dot
                    for (int i = 0; i < zeroCount; i++)
                    {
                        input.UngetChar('0');
                        
                    }
                    // Unget the dot
                    input.UngetChar('.');
                    
                    // Set lexeme, token_type, and line_no
                    tmp.lexeme = "0";
                    tmp.token_type = NUM;
                    tmp.line_no = line_no;
                }
                else
                    // If we DID see a non-zero after the dot
                {
                    // Set the lexeme, token_type, and line_no
                    tmp.lexeme = tempLex;
                    tmp.token_type = REALNUM;
                    tmp.line_no = line_no;
                    
                }
                
                
            }
            // If the next character after the zero is a newline character
            else if (c == '\n')
            {
                // Unget the newline character
                input.UngetChar('\n');
            }
            // If the next character after the zero is an x
            else if (c == 'x')
            {
                // Get the next char
                input.GetChar(c);
                
                // If it is a zero
                if (c == '0')
                {
                    // Get the next character
                    input.GetChar(c);
                    
                    // If it is a zero
                    if (c == '8')
                    {
                        // Update the lexeme, set token type to BASE08NUM, and set line number to line_no
                        tmp.lexeme += "x08";
                        tmp.token_type = BASE08NUM;
                        tmp.line_no = line_no;
                    }
                    // If it is not a zero
                    else
                    {
                        // Unget the character and the zero and the x
                        input.UngetChar(c);
                        input.UngetChar('0');
                        input.UngetChar('x');
                    }
                }
                // If it is a one
                else if (c == '1')
                {
                    // Get the next character
                    input.GetChar(c);
                    // If it is a 6
                    if (c == '6')
                    {
                        // Update the lexeme, set token type to BASE16NUM, and set line number to line_no
                        tmp.lexeme += "x16";
                        tmp.token_type = BASE16NUM;
                        tmp.line_no = line_no;
                        
                    }
                    // If it is anything but a 6
                    else
                    {
                        // Unget the character and the one and the x
                        input.UngetChar(c);
                        input.UngetChar('1');
                        input.UngetChar('x');
                        
                    }
                    
                }
                // If it is not a zero or a one
                else
                {
                    input.UngetChar(c);
                    input.UngetChar('x');
                    tmp.token_type = NUM;
                    tmp.line_no = line_no;
                }
            }
            else if (c == ',')
            {
                input.UngetChar(c);
            }
            else if (c == ';')
            {
                input.UngetChar(c);
            }
        }
        // If the first digit we saw was not zero
        else
        {
            // Set initial token type to NUM
            tmp.token_type = NUM;
            // Get the next character from input
            input.GetChar(c);
            
            // If the next character is a dot
            if (c == '.')
            {
                // Create a temporary lexeme string and add the dot
                string tempLex = tmp.lexeme;
                tempLex += ".";
                
                // boolean to determine whther or not there are digits after the dot
                bool hasDigits = false;
                
                // Get the character following the dot
                input.GetChar(c);
                
                while (!input.EndOfInput() && isdigit(c))
                {
                    // Concatenate all of the digits following the dot
                    tempLex += c;
                    // Set hasDigits to true if we ever enter this loop (because it means there are digits immediately
                    // following the dot)
                    hasDigits = true;
                    
                    // Get a new character from input after each loop iteration
                    input.GetChar(c);
                }
                // If it isn't the end of the input, unget the most recently gotten char
                if (!input.EndOfInput())
                {
                    input.UngetChar(c);
                }
                
                // If there are digits immediately following the dot
                if (hasDigits)
                {
                    // Set the lexeme to tempLex, the token type to REALNUM, and set the line number
                    tmp.lexeme = tempLex;
                    tmp.token_type = REALNUM;
                    tmp.line_no = line_no;
                }
                // If the next character after the dot is not a digit
                else
                {
                    // Set the token type to NUM, set the line number, and Unget the dot
                    tmp.token_type = NUM;
                    tmp.line_no = line_no;
                    input.UngetChar('.');
                }
            }
            // If the character following the string of digits is the newline character
            else if (c == '\n')
            {
                // Unget the newline character
                input.UngetChar(c);
            }
            // If the next character after the string of digits is an alphabetic character
            else if (isalpha(c))
            {
                // Create a temporary lexeme string
                string tempLex = tmp.lexeme;
                // If c is x and the number can be base 8 (none of its digits exceed 7)
                if (c == 'x' && canBeBase8)
                {
                    // Add the x to the temporaey lexeme
                    tempLex += c;
                    // Get the next character after the x
                    input.GetChar(c);
                    // If the next character after the x is a digit
                    if (isdigit(c))
                    {
                        // If the digit is a zero
                        if (c == '0')
                        {
                            // Add it to the temporary lexeme string and get the next char
                            tempLex += c;
                            input.GetChar(c);
                            // If the next char is an 8
                            if (c == '8')
                            {
                                // Add it to the lexeme and set the lexeme, token type, and line number
                                tempLex += c;
                                tmp.lexeme = tempLex;
                                tmp.token_type = BASE08NUM;
                                tmp.line_no = line_no;
                            }
                            // If the char after the 0 was not an 8
                            else
                            {
                                // Unget the character, then the zero, then the x
                                input.UngetChar(c);
                                input.UngetChar('0');
                                input.UngetChar('x');
                            }
                        }
                        // If the digit is a 1
                        else if (c == '1')
                        {
                            // Add it to the temporary lexeme and get the next char
                            tempLex += c;
                            input.GetChar(c);
                            // If the next char is a 6
                            if (c == '6')
                            {
                                // Add it to the temporary lexeme and set the lexeme, token type, and line number
                                tempLex += c;
                                tmp.lexeme = tempLex;
                                tmp.token_type = BASE16NUM;
                                tmp.line_no = line_no;
                            }
                            // If the next char was not a 6
                            else
                            {
                                // Unget the character, then the one, then the x
                                input.UngetChar(c);
                                input.UngetChar('1');
                                input.UngetChar('x');
                            }
                        }
                        // If the digit was not zero or one
                        else
                        {
                            // Unget the character and then the x
                            input.UngetChar(c);
                            input.UngetChar('x');
                        }
                    }
                    // If the next character after the x is not a digit
                    else
                    {
                        // Unget the character and unget the x
                        input.UngetChar(c);
                        input.UngetChar('x');
                    }
                }
                // If c is any other letter of the alphabet
                else
                {
                    // Boolean to determine if the number can be a base 16 number or not
                    bool canBeBase16 = true;
                    // Char array to store alphabetical characters that could be part of a hex number
                    char *possibleHexValues = new char[100];
                    
                    // If the first letter after the number is upper case and it does not come after 'F'
                    if (isupper(c) && c <= 'F')
                    {
                        // Add the letter to the possible hex values array
                        possibleHexValues[0] = c;
                        // Integer used in the following while loop
                        int i = 1;
                        
                        // Get a character from input
                        input.GetChar(c);
                        // While there is still input and the character is alphabetic, uppercase, and does not come after 'F'
                        while (!input.EndOfInput() && isalnum(c) && c <= 'F')
                        {
                            if (isalpha(c) && isupper(c))
                            {
                            // Add the character to the array of possible hex values
                            possibleHexValues[i] = c;
                            // Increment the index variable
                            i++;
                            // Get the next character from input
                            input.GetChar(c);
                            }
                            else if (isdigit(c))
                            {
                                possibleHexValues[i] = c;
                                
                                i++;
                                
                                input.GetChar(c);
                                
                            }
                            else if (isalpha(c))
                            {
                                break;
                            }
                        }
                        
                        // If we stopped reading uppercase letters because we saw an x
                        if (c == 'x')
                        {
                            // Get the next char from input
                            input.GetChar(c);
                            
                            // If it is a 1
                            if (c == '1')
                            {
                                // Get the next char after the 1
                                input.GetChar(c);
                                
                                // If the next char is a 6
                                if (c == '6')
                                {
                                    // Add the possible hex values onto the lexeme
                                    for (int a = 0; a < i; a++)
                                    {
                                        tmp.lexeme += possibleHexValues[a];
                                    }
                                    // Add the 'x16' to the lexeme
                                    tmp.lexeme += 'x';
                                    tmp.lexeme += '1';
                                    tmp.lexeme += '6';
                                    // Set the token type to BASE16NUM and the line number to line_no
                                    tmp.token_type = BASE16NUM;
                                    tmp.line_no = line_no;
                                }
                                // If the next char after the 1 was not a 6
                                else
                                {
                                    // Unget the character, then the one, then the x
                                    input.UngetChar(c);
                                    input.UngetChar('1');
                                    input.UngetChar('x');
                                    
                                    // Decrement the index variable so it's at the last read possible hex value
                                    i--;
                                    
                                    // Loop through the possibleHexValues array and unget all of those characters
                                    for (; i >= 0; i--)
                                    {
                                        input.UngetChar(possibleHexValues[i]);
                                    }
                                    // Set the token type to NUM and line number to line_no
                                    tmp.token_type = NUM;
                                    tmp.line_no = line_no;
                                }
                                
                            }
                            // If the character after the x is anything but a 1
                            else
                            {
                                // Unget the character, and the x
                                input.UngetChar(c);
                                input.UngetChar('x');
                                
                                // Decrement i so that it is the index of the last recorded possible hex value in the
                                // char array
                                i--;
                                // Traverse possibleHexValues backwards, ungetting each char
                                for (; i >= 0; i--)
                                {
                                    input.UngetChar(possibleHexValues[i]);
                                }
                                // Set the token type to NUM and the line number to line_no
                                tmp.token_type = NUM;
                                tmp.line_no = line_no;
                                
                            }
                        }
                        // If we stopped reading uppercase letters because we saw a digit, a dot, a lower case letter,
                        // a newline character, or a space
                        else if (isdigit(c) || c == '.' || islower(c) || c == '\n' || c == ' ' || c > 'F')
                        {
                            // Unget the character
                            input.UngetChar(c);
                            
                            // Decrement i so that it is indexed at the last entry of possibleHexValues
                            i--;
                            // Traverse possibleHexValues backwards to unget all of the characters
                            for (; i >= 0; i--)
                            {
                                input.UngetChar(possibleHexValues[i]);
                            }
                            // Set token type to NUM and line number to line_no
                            tmp.token_type = NUM;
                            tmp.line_no = line_no;
                        }
                        
                        
                        
                        
                    }
                    // If it is lower case, or the letter comes after 'E', we know it cannot be a base 16 number, so we can unget
                    else
                    {
                        input.UngetChar(c);
                    }
                }
                
            }
            else if (c == ',')
            {
                input.UngetChar(c);
            }
            else if (c == ';')
            {
                input.UngetChar(c);
            }
        }
        
        
        //tmp.token_type = NUM;
        //tmp.line_no = line_no;
        return tmp;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);
    
    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;
    
    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }
    
    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;
            
            return tmp;
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;
    
    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}

