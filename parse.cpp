#include <vector>
#include <iostream>

enum TokenType {eNumber = 0, eLeftBracket, eRightBracket, eComma, eString, eWhitespace, eUnexpected};

const char *tokentypestr[] =
{
    "number",
    "leftbracket",
    "rightbracket",
    "comma",
    "string",
    "whitespace",
    "unexpected"
};

typedef char Char;

struct Token
{
    const Char* from;
    const Char* to;
    TokenType type;
    Token(const Char* f, const Char* t, TokenType ty) : from(f), to(t), type(ty) {}
};

void printtoken(const Token &token)
{
    for (const Char *p = token.from; p < token.to; ++p)
        std::cout << *p;
}

bool isdigit(Char ch)
{
    return ch >= '0' && ch <= '9';
}

bool iswhitespace(Char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n';
}

const Char *eatnumber(const Char *start, const Char *end)
{
    const Char *pos = start;
    bool dot = false;
    int digits = 0;
    
    if (*pos == '+' || *pos == '-')
    {
        ++pos;
    }
    
    for (; pos < end; ++pos)
    {
        if (isdigit(*pos)) ++digits;
        else if (*pos == '.' && !dot) dot = true;
        else break;
    }
    
    if (digits > 0)
        return pos;

    return 0;
}

const Char *eatstring(const Char *start, const Char *end)
{
    const Char *pos = start;

    if (*pos == '"') ++pos;
    else return 0;

    for (;;++pos)
    {
        if (pos == end) return 0;
        if (*pos == '"') return pos + 1;
    }
}

TokenType peek(Char ch)
{
    if (iswhitespace(ch)) return eWhitespace;
    else if (ch == '"') return eString;
    else if (isdigit(ch) || ch == '+' || ch == '-' || ch == '.') return eNumber;
    else if (ch == '[') return eLeftBracket;
    else if (ch == ']') return eRightBracket;
    else if (ch == ',') return eComma;
    else return eUnexpected;
}

const Char *eatwhitespace(const Char *start, const Char *end)
{
    const Char *pos = start;
    while (pos < end && iswhitespace(*pos))
        ++pos;
    return pos;
}

bool tokenize(const Char *start, const Char *end, std::vector<Token> &tokens)
{
    const Char *pos = start;
    while (pos < end)
    {
        const Char *newpos = pos;
        TokenType type = peek(*pos);
        switch (type)
        {
        case eWhitespace:
            newpos = eatwhitespace(pos, end);
            break;
        case eNumber:
            newpos = eatnumber(pos, end);
            break;
        case eLeftBracket:
        case eRightBracket:
        case eComma:
            newpos = pos + 1;
            break;
        case eString:
            newpos = eatstring(pos, end);
            break;
        default:
            break;
        }
        
        if (type == eUnexpected)
        {
            std::cout << "Unexpected characters at column " << pos - start << std::endl;
            break;
        }
        
        if (newpos)
        {
            if (type != eWhitespace)
                tokens.push_back(Token(pos, newpos, type));
        }
        else
        {
            std::cout << "Unable to parse " << tokentypestr[type] << " at column " << pos - start << std::endl;
            break;
        }
        pos = newpos;
    }

    return pos == end;
}

struct Tree
{
    const Token *from;
    const Token *to;
    Tree(const Token *f, const Token *t) : from(f), to(t) {}
};

void printtree(const Tree &tree)
{
    if (tree.from == tree.to)
        std::cout << "empty";
    if (tree.from > tree.to)
        std::cout << "error";
    if (!tree.from || !tree.to)
        std::cout << "error null";

    for (const Token *t = tree.from; t < tree.to; ++t)
    {
        printtoken(*t);
    }
}

// return next token after first element in array
const Token *eatfirstelement(const Token *from, const Token *to, const Token* &firstto)
{
    const Token *result = 0;
    firstto = from;
    
    if (firstto == to)
    {
        // empty array
        result = firstto;
    }
    
    else if (firstto->type == eComma)
    {
        // first element is empty
        result = firstto + 1;
    }

    else if (firstto->type == eNumber || firstto->type == eString)
    {
        ++firstto;

        if (firstto == to)
            result = to; // last element, no comma

        if (firstto->type == eComma) result = firstto + 1;
    }

    else if (firstto->type == eLeftBracket)
    {
        int level = 1;
        ++firstto; // skip [
        // Search for closing bracket pair
        for (;; ++firstto)
        {
            if (firstto == to) break;
            if (firstto->type == eRightBracket) --level;
            if (firstto->type == eLeftBracket) ++level;
            if (level == 0)
            {
                ++firstto;
                if (firstto == to)
                {
                    result = to; // last element, no comma
                    break;
                }
                if (firstto->type == eComma)
                {
                    result = firstto + 1;
                    break;
                }
            }
        }
    }

    return result;
}

// tokens are [from, to) path is [pfrom, pto)

Tree getsubtree(const Token *from, const Token *to, const size_t *pfrom, const size_t *pto)
{
    if (from > to)
    {
        std::cout << "Error: cound not find such element" << std::endl;
        return Tree(from, to); // empty range
    }

    if (pfrom == pto)
        return Tree(from, to); // no more steps in path, return what we have so far

    // take next step from path
    size_t index = *pfrom;

    if (from->type == eLeftBracket)
    {
        const Token *currentto = 0;
        const Token *actualto = eatfirstelement(from, to, currentto);

        if (actualto == 0)
        {
            std::cout << "Error: unable to index elements in ";
            printtree(Tree(from, to));
            std::cout << std::endl;
            return Tree(from, to);
        }

        if (actualto != to)
            std::cout << "Warning: given range does not represent a valid array in brackets" << std::endl;

        const Token *current;
        const Token *next = from + 1; // skip [
        --actualto; // omit ]

        for (size_t steps = 0; steps <= index; ++steps)
        {
            current = next;
            next = eatfirstelement(current, actualto, currentto);

            if (next == 0)
            {
                std::cout << "Error: unable to take first element in ";
                printtree(Tree(current, actualto));
                std::cout << std::endl;
                return Tree(current, actualto);
            }
        }

        return getsubtree(current, currentto, pfrom + 1, pto);
    }
    else
    {
        std::cout << "Error: attempt to index a non-array" << std::endl;
        return Tree(from, to);
    }
}

Tree resolvepath(const std::vector<Token>& tokens, const size_t *path, size_t pathsize)
{
    return getsubtree(tokens.data(), tokens.data() + tokens.size(),
                      path, path + pathsize);
}

// validate??
// "hello""test"
 
int main()
{
    const Char *test_string = "[[[\"a\",3,-2.4,1],,,3,\"hello\",[[-1,.0,0.,[+2]]]]]";
    std::vector<Token> tokens;
    bool ok = tokenize(test_string, test_string + strlen(test_string), tokens);

    if (!ok)
    {
        std::cout << "failed to tokenize" << std::endl;
        return 0;
    }

    size_t path[][10] =
    {
        {0},
        {0, 0},
        {0, 0, 0},       // --> "a"
        {0, 0, 1},       // --> 2
        {0, 0, 2},       // --> -2.4
        {0, 2},          // --> empty
        {0, 4},          // --> "hello"
        {0, 5, 0, 3, 0}, // --> +2
        {0, 5, 0, 3}     // --> [ +2 ]
    };

    size_t steps[] = {1, 2, 3, 3, 3, 2, 2, 5, 4};

    for (size_t p = 0; p < sizeof(steps)/sizeof(steps[0]); ++p)
    {
        std::cout << "result: ";
        printtree(resolvepath(tokens, path[p], steps[p]));
        std::cout << std::endl;
    }

    return 0;
}
