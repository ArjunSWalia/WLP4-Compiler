#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
using namespace std;

vector<string> CONSTIDS{
    "INT", "RETURN", "WAIN", "IF", "ELSE", "WHILE", "PRINTLN", "NEW", "DELETE", "NULL"};

class Scanner
{
public:
    string str;
    int index;
    Scanner(string str)
    {
        this->str = str;
        this->index = 0;
    }
    char grab() { return this->str[this->index]; }
    void incr() { this->index++; }
    bool completed() { return this->index >= this->str.length(); }
};

class Token
{
public:
    pair<string, string> kindlexeme;
    Token()
    {
        this->kindlexeme.first = "KIND";
        this->kindlexeme.second = "LEXEME";
    }
    Token(string kind, string lexeme)
    {
        this->kindlexeme.first = kind;
        this->kindlexeme.second = lexeme;
    }
    void printToken()
    {
        if (this->kindlexeme.first == "INVALID")
        {
            cerr << "ERROR "<< "\n";
        }
        else
        {
            cout << this->kindlexeme.first << " " << this->kindlexeme.second << "\n";
        }
    }
};

Token DFA_ID(Scanner &scanner)
{
    Token t;
    string str;
    string Upper;
    str += scanner.grab();
    scanner.incr();
    // we check isalnum as the first char must be alpha, and then the rest can be 0-9 as well, ex, ID arjun132 is valid, because I can have int a123
    while (!scanner.completed() && isalnum(scanner.grab()))
    {
        str += scanner.grab();
        scanner.incr();
    }
    for (int i = 0; i < str.length(); i++)
    {
        Upper += toupper(str.at(i));
    }
    if (find(CONSTIDS.begin(), CONSTIDS.end(), Upper) != CONSTIDS.end())
    {
        t.kindlexeme.first = Upper;
        t.kindlexeme.second = str;
    }
    else
    {
        t.kindlexeme.first = "ID";
        t.kindlexeme.second = str;
    }
    return t;
}

Token DFA_NUM(Scanner &scanner)
{
    Token t;
    string str;
    str += scanner.grab();
    scanner.incr();
    while (!scanner.completed() && isdigit(scanner.grab()))
    {
        str += scanner.grab();
        scanner.incr();
    }
    // leading zero
    if (str.length() > 1 && str.at(0) == '0')
    {
        t.kindlexeme.first = "INVALID";
        t.kindlexeme.second = str;
        return t;
    }
    try
    {
        unsigned long long val = stoull(str);
        if (val > 2147483647)
        {
            t.kindlexeme.first = "INVALID";
            t.kindlexeme.second = str;
            return t;
        }
    }
    catch (out_of_range)
    {
        t.kindlexeme.first = "INVALID";
        t.kindlexeme.second = str;
    }
    t.kindlexeme.first = "NUM";
    t.kindlexeme.second = str;
    return t;
}

Token DFA_EQUALS(Scanner &scanner)
{
    string str;
    Token t;
    scanner.incr();
    if (scanner.grab() == '=')
    {
        t.kindlexeme.first = "EQ";
        t.kindlexeme.second = "==";
        if (!scanner.completed())
        {
            scanner.incr();
        }
    }
    else
    {
        // this assumes the next value is a space
        t.kindlexeme.first = "BECOMES";
        t.kindlexeme.second = "=";
    }
    return t;
}

Token DFA_GREAT(Scanner &scanner)
{
    string str;
    Token t;
    scanner.index++;
    if (scanner.grab() == '=')
    {
        t.kindlexeme.first = "GE";
        t.kindlexeme.second = ">=";
        if (!scanner.completed())
        {
            scanner.incr();
        }
    }
    else
    {
        t.kindlexeme.first = "GT";
        t.kindlexeme.second = ">";
    }
    return t;
}

Token DFA_LESS(Scanner &scanner)
{
    string str;
    Token t;
    scanner.index++;
    if (scanner.grab() == '=')
    {
        t.kindlexeme.first = "LE";
        t.kindlexeme.second = "<=";
        if (!scanner.completed())
        {
            scanner.incr();
        }
    }
    else
    {
        t.kindlexeme.first = "LT";
        t.kindlexeme.second = "<";
    }
    return t;
}

int main()
{
    char c;
    string total;
    while (true)
    {
        c = getchar();
        if (c == EOF)
        {
            break;
        }
        total += c;
    }
    Scanner WLP4Scan(total);
    vector<Token> tokens;

    while (!WLP4Scan.completed())
    {
        char c = WLP4Scan.grab();
        Token t;
        if (isalpha(c))
        {
            t = DFA_ID(WLP4Scan);
        }
        else if (isdigit(c))
        {
            t = DFA_NUM(WLP4Scan);
        }
        else if (c == '=')
        {
            t = DFA_EQUALS(WLP4Scan);
        }
        else if (c == '>')
        {
            t = DFA_GREAT(WLP4Scan);
        }
        else if (c == '<')
        {
            t = DFA_LESS(WLP4Scan);
        }
        else if (c == '(')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "LPAREN";
            t.kindlexeme.second = c;
        }
        else if (c == ')')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "RPAREN";
            t.kindlexeme.second = c;
        }
        else if (c == '{')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "LBRACE";
            t.kindlexeme.second = c;
        }
        else if (c == '}')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "RBRACE";
            t.kindlexeme.second = c;
        }
        else if (c == '[')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "LBRACK";
            t.kindlexeme.second = c;
        }
        else if (c == ']')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "RBRACK";
            t.kindlexeme.second = c;
        }
        else if (c == '[')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "LBRACK";
            t.kindlexeme.second = c;
        }
        else if (c == '+')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "PLUS";
            t.kindlexeme.second = c;
        }
        else if (c == '-')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "MINUS";
            t.kindlexeme.second = c;
        }
        else if (c == '*')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "STAR";
            t.kindlexeme.second = c;
        }
        else if (c == '/')
        {
            WLP4Scan.incr();
            if (WLP4Scan.grab() == '/')
            {
                WLP4Scan.incr();
                while (WLP4Scan.grab() != 0x0a && WLP4Scan.grab() != 0x0d)
                {
                    WLP4Scan.incr();
                }
                WLP4Scan.incr();
            }
            else
            {
                t.kindlexeme.first = "SLASH";
                t.kindlexeme.second = "/";
            }
        }
        else if (c == '%')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "PCT";
            t.kindlexeme.second = c;
        }
        else if (c == '&')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "AMP";
            t.kindlexeme.second = c;
        }
        else if (c == ',')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "COMMA";
            t.kindlexeme.second = c;
        }
        else if (c == ';')
        {
            WLP4Scan.incr();
            t.kindlexeme.first = "SEMI";
            t.kindlexeme.second = c;
        }
        else if (c == '!')
        {
            WLP4Scan.incr();
            if (WLP4Scan.grab() == '=')
            {
                t.kindlexeme.first = "NE";
                t.kindlexeme.second = "!=";
            }
            else
            {
                t.kindlexeme.first = "INVALID";
                t.kindlexeme.second = c + WLP4Scan.grab();
            }
            WLP4Scan.incr();
        }
        else if (c == 0x20 || c == 0x09 || c == 0x0a || c == 0x0d)
        {
            WLP4Scan.incr();
        }
        else
        {
            t.kindlexeme.first = "INVALID";
            t.kindlexeme.second = c;
        }
        if (t.kindlexeme.first == "INVALID")
        {
            cerr << "ERROR"<<"\n";
            return 1;
        }
        if (t.kindlexeme.first != "KIND")
        {
            tokens.push_back(t);
        }
    }
    for (int i = 0; i < tokens.size(); i++)
    {
        tokens.at(i).printToken();
    }
}