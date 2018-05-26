#ifndef MICROHCL_H_
#define MICROHCL_H_

#include <cassert>
#include <cctype>
#include <fstream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>
#ifdef MICROHCL_USE_MAP
#include <map>
#else
#include <unordered_map>
#endif
#include <vector>

namespace hcl {

class Value;
typedef std::vector<Value> Array;

#ifdef MICROHCP_USE_MAP
typedef std::map<std::string, Value> Object;
#else
typedef std::unordered_map<std::string, Value> Object;
#endif

namespace internal {
template<typename T> struct call_traits_value {
    typedef T return_type;
};
template<typename T> struct call_traits_ref {
    typedef const T& return_type;
};
} // namespace internal

template<typename T> struct call_traits;
template<> struct call_traits<bool> : public internal::call_traits_value<bool> {};
template<> struct call_traits<int> : public internal::call_traits_value<int> {};
template<> struct call_traits<int64_t> : public internal::call_traits_value<int64_t> {};
template<> struct call_traits<double> : public internal::call_traits_value<double> {};
template<> struct call_traits<std::string> : public internal::call_traits_ref<std::string> {};
template<> struct call_traits<Array> : public internal::call_traits_ref<Array> {};
template<> struct call_traits<Object> : public internal::call_traits_ref<Object> {};

// A value is returned for std::vector<T>. Not reference.
// This is because a fresh vector is made.
template<typename T> struct call_traits<std::vector<T>> : public internal::call_traits_value<std::vector<T>> {};

class Value {
public:
    enum Type {
        NULL_TYPE,
        BOOL_TYPE,
        INT_TYPE,
        DOUBLE_TYPE,
        STRING_TYPE,
        ARRAY_TYPE,
        OBJECT_TYPE,
    };

    Value() : type_(NULL_TYPE), null_(nullptr) {}
    Value(bool v) : type_(BOOL_TYPE), bool_(v) {}
    Value(int v) : type_(INT_TYPE), int_(v) {}
    Value(int64_t v) : type_(INT_TYPE), int_(v) {}
    Value(double v) : type_(DOUBLE_TYPE), double_(v) {}
    Value(const std::string& v) : type_(STRING_TYPE), string_(new std::string(v)) {}
    Value(const char* v) : type_(STRING_TYPE), string_(new std::string(v)) {}
    Value(const Array& v) : type_(ARRAY_TYPE), array_(new Array(v)) {}
    Value(const Object& v) : type_(OBJECT_TYPE), object_(new Object(v)) {}
    Value(std::string&& v) : type_(STRING_TYPE), string_(new std::string(std::move(v))) {}
    Value(Array&& v) : type_(ARRAY_TYPE), array_(new Array(std::move(v))) {}
    Value(Object&& v) : type_(OBJECT_TYPE), object_(new Object(std::move(v))) {}

    Value(const Value& v);
    Value(Value&& v) noexcept;
    Value& operator=(const Value& v);
    Value& operator=(Value&& v) noexcept;

    // Guards from unexpected Value construction.
    // Someone might use a value like this:
    //   hcl::Value v = x->find("foo");
    // But this is wrong. Without this constructor,
    // value will be unexpectedly initialized with bool.
    Value(const void* v) = delete;
    ~Value();

    // Retruns Value size.
    // 0 for invalid value.
    // The number of inner elements for array or object.
    // 1 for other types.
    size_t size() const;
    bool empty() const;
    Type type() const { return type_; }

    bool valid() const { return type_ != NULL_TYPE; }
    template<typename T> bool is() const;
    template<typename T> typename call_traits<T>::return_type as() const;

    friend bool operator==(const Value& lhs, const Value& rhs);
    friend bool operator!=(const Value& lhs, const Value& rhs) { return !(lhs == rhs); }

    // ----------------------------------------------------------------------
    // For integer/floating value

    // Returns true if the value is int or double.
    bool isNumber() const;
    double asNumber() const;

    // ----------------------------------------------------------------------
    // For Object value

    template<typename T> typename call_traits<T>::return_type get(const std::string&) const;
    Value* set(const std::string& key, const Value& v);
    // Finds a Value with |key|. |key| can contain '.'
    // Note: if you would like to find a child value only, you need to use findChild.
    const Value* find(const std::string& key) const;
    Value* find(const std::string& key);
    bool has(const std::string& key) const { return find(key) != nullptr; }
    bool erase(const std::string& key);

    Value& operator[](const std::string& key);

    // Merge object. Returns true if succeeded. Otherwise, |this| might be corrupted.
    // When the same key exists, it will be overwritten.
    bool merge(const Value&);

    // Finds a value with |key|. It searches only children.
    Value* findChild(const std::string& key);
    const Value* findChild(const std::string& key) const;
    // Sets a value, and returns the pointer to the created value.
    // When the value having the same key exists, it will be overwritten.
    Value* setChild(const std::string& key, const Value& v);
    Value* setChild(const std::string& key, Value&& v);
    bool eraseChild(const std::string& key);

    // ----------------------------------------------------------------------
    // For Array value

    template<typename T> typename call_traits<T>::return_type get(size_t index) const;
    const Value* find(size_t index) const;
    Value* find(size_t index);
    Value* push(const Value& v);
    Value* push(Value&& v);

private:
    static const char* typeToString(Type);

    template<typename T> void assureType() const;
    Value* ensureValue(const std::string& key);

    template<typename T> struct ValueConverter;

    Type type_;
    union {
        void* null_; // Can never be legitimately set, indicates parse error
        bool bool_;
        int64_t int_;
        double double_;
        std::string* string_;
        Array* array_;
        Object* object_;
    };

    template<typename T> friend struct ValueConverter;
};

// parse() returns ParseResult.
struct ParseResult {
    ParseResult(hcl::Value v, std::string er) :
        value(std::move(v)),
        errorReason(std::move(er)) {}

    bool valid() const { return value.valid(); }

    hcl::Value value;
    std::string errorReason;
};

// Parses from std::istream.
ParseResult parse(std::istream&);
// Parses a file.
ParseResult parseFile(const std::string& filename);

namespace internal {

enum class TokenType {
    // Special tokens
    ILLEGAL,
    END_OF_FILE,
    COMMENT,


    IDENT, // literals

    NUMBER,  // 12345
    FLOAT,   // 123.45
    BOOL,    // true,false
    STRING,  // "abc"
    HEREDOC, // <<FOO\nbar\nFOO

    LBRACK, // [
    LBRACE, // {
    COMMA,  // ,
    PERIOD, // .

    RBRACK, // ]
    RBRACE, // }

    ASSIGN, // =
    ADD,    // +
    SUB,    // -
};

class Token {
public:
    explicit Token(TokenType type) : type_(type) {}
    Token(TokenType type, const std::string& v) : type_(type), str_value_(v) {}
    Token(TokenType type, bool v) : type_(type), int_value_(v) {}
    Token(TokenType type, std::int64_t v) : type_(type), int_value_(v) {}
    Token(TokenType type, double v) : type_(type), double_value_(v) {}

    TokenType type() const { return type_; }
    const std::string& strValue() const { return str_value_; }
    bool boolValue() const { return int_value_ != 0; }
    std::int64_t intValue() const { return int_value_; }
    double doubleValue() const { return double_value_; }

private:
    TokenType type_;
    std::string str_value_;
    std::int64_t int_value_;
    double double_value_;
};

class Lexer {
public:
    explicit Lexer(std::istream& is) : is_(is), lineNo_(1), peek_(Token(TokenType::ILLEGAL)) {}

    Token nextToken();
    Token peekToken() {
        peek_ = nextToken();
        return peek_;
    }

    int lineNo() const { return lineNo_; }

    // Skips if UTF8BOM is found.
    // Returns true if success. Returns false if intermediate state is left.
    bool skipUTF8BOM();

private:
    bool current(char* c);
    void next();
    bool consume(char c);

    Token nextValueToken();

    void skipUntilNewLine();

    Token nextStringDoubleQuote();
    Token nextStringSingleQuote();
    Token nextHereDoc();

    Token peek_;
    std::istream& is_;
    int lineNo_;
};

class Parser {
public:
    explicit Parser(std::istream& is) : lexer_(is), token_(TokenType::ILLEGAL)
    {
        if (!lexer_.skipUTF8BOM()) {
            token_ = Token(TokenType::ILLEGAL, std::string("Invalid UTF8 BOM"));
        } else {
            nextToken();
        }
    }

    // Parses. If failed, value should be invalid value.
    // You can get the error by calling errorReason().
    Value parse();
    const std::string& errorReason();

private:
    const Token& token() const { return token_; }
    void nextToken() { token_ = lexer_.nextToken(); }

    void skipForKey();
    void skipForValue();

    bool consumeForKey(TokenType);
    bool consumeForValue(TokenType);
    bool consumeEOLorEOFForKey();

    Value parseObjectList(bool);
    bool parseKeys(std::vector<std::string>&);
    bool parseObjectItem(Value&);
    bool parseObject(Value&);
    bool parseObjectType(Value&);
    bool parseListType(Value&);
    bool parseLiteralType(Value&);

    Token peekToken() { return lexer_.peekToken(); };

    void addError(const std::string& reason);

    Lexer lexer_;
    Token token_;
    std::string errorReason_;
};

} // namespace internal

// ----------------------------------------------------------------------
// Implementations

inline ParseResult parse(std::istream& is)
{
    if (!is) {
        return ParseResult(hcl::Value(), "stream is in bad state. file does not exist?");
    }

    internal::Parser parser(is);
    hcl::Value v = parser.parse();

    if (v.valid())
        return ParseResult(std::move(v), std::string());

    return ParseResult(std::move(v), std::move(parser.errorReason()));
}

inline ParseResult parseFile(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs) {
        return ParseResult(hcl::Value(),
                           std::string("could not open file: ") + filename);
    }

    return parse(ifs);
}

inline std::string format(std::stringstream& ss)
{
    return ss.str();
}

template<typename T, typename... Args>
std::string format(std::stringstream& ss, T&& t, Args&&... args)
{
    ss << std::forward<T>(t);
    return format(ss, std::forward<Args>(args)...);
}

template<typename... Args>
#if defined(_MSC_VER)
__declspec(noreturn)
#else
[[noreturn]]
#endif
void failwith(Args&&... args)
{
    std::stringstream ss;
    throw std::runtime_error(format(ss, std::forward<Args>(args)...));
}

namespace internal {

inline std::string removeDelimiter(const std::string& s)
{
    std::string r;
    for (char c : s) {
        if (c == '_')
            continue;
        r += c;
    }
    return r;
}

inline std::string unescape(const std::string& codepoint)
{
    std::uint32_t x;
    std::uint8_t buf[8];
    std::stringstream ss(codepoint);

    ss >> std::hex >> x;

    if (x <= 0x7FUL) {
        // 0xxxxxxx
        buf[0] = 0x00 | ((x >> 0) & 0x7F);
        buf[1] = '\0';
    } else if (x <= 0x7FFUL) {
        // 110yyyyx 10xxxxxx
        buf[0] = 0xC0 | ((x >> 6) & 0xDF);
        buf[1] = 0x80 | ((x >> 0) & 0xBF);
        buf[2] = '\0';
    } else if (x <= 0xFFFFUL) {
        // 1110yyyy 10yxxxxx 10xxxxxx
        buf[0] = 0xE0 | ((x >> 12) & 0xEF);
        buf[1] = 0x80 | ((x >> 6) & 0xBF);
        buf[2] = 0x80 | ((x >> 0) & 0xBF);
        buf[3] = '\0';
    } else if (x <= 0x10FFFFUL) {
        // 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx
        buf[0] = 0xF0 | ((x >> 18) & 0xF7);
        buf[1] = 0x80 | ((x >> 12) & 0xBF);
        buf[2] = 0x80 | ((x >> 6) & 0xBF);
        buf[3] = 0x80 | ((x >> 0) & 0xBF);
        buf[4] = '\0';
    } else {
        buf[0] = '\0';
    }

    return reinterpret_cast<char*>(buf);
}

// Returns true if |s| is integer.
// [+-]?\d+(_\d+)*
inline bool isInteger(const std::string& s)
{
    if (s.empty())
        return false;

    std::string::size_type p = 0;
    if (s[p] == '+' || s[p] == '-')
        ++p;

    while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
        ++p;
        if (p < s.size() && s[p] == '_') {
            ++p;
            if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                return false;
        }
    }

    return p == s.size();
}

// Returns true if |s| is double.
// [+-]? (\d+(_\d+)*)? (\.\d+(_\d+)*)? ([eE] [+-]? \d+(_\d+)*)?
//       1-----------  2-------------  3----------------------
// 2 or (1 and 3) should exist.
inline bool isDouble(const std::string& s)
{
    if (s.empty())
        return false;

    std::string::size_type p = 0;
    if (s[p] == '+' || s[p] == '-')
        ++p;

    bool ok = false;
    while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
        ++p;
        ok = true;

        if (p < s.size() && s[p] == '_') {
            ++p;
            if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                return false;
        }
    }

    if (p < s.size() && s[p] == '.')
        ++p;

    while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
        ++p;
        ok = true;

        if (p < s.size() && s[p] == '_') {
            ++p;
            if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                return false;
        }
    }

    if (!ok)
        return false;

    ok = false;
    if (p < s.size() && (s[p] == 'e' || s[p] == 'E')) {
        ++p;
        if (p < s.size() && (s[p] == '+' || s[p] == '-'))
            ++p;
        while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
            ++p;
            ok = true;

            if (p < s.size() && s[p] == '_') {
                ++p;
                if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                    return false;
            }
        }
        if (!ok)
            return false;
    }

    return p == s.size();
}

// static
inline std::string escapeString(const std::string& s)
{
    std::stringstream ss;
    for (size_t i = 0; i < s.size(); ++i) {
        switch (s[i]) {
        case '\n': ss << "\\n"; break;
        case '\r': ss << "\\r"; break;
        case '\t': ss << "\\t"; break;
        case '\"': ss << "\\\""; break;
        case '\'': ss << "\\\'"; break;
        case '\\': ss << "\\\\"; break;
        default: ss << s[i]; break;
        }
    }

    return ss.str();
}

} // namespace internal

// ----------------------------------------------------------------------
// Lexer

namespace internal {

inline bool Lexer::skipUTF8BOM()
{
    // Check [EF, BB, BF]

    int x1 = is_.peek();
    if (x1 != 0xEF) {
        // When the first byte is not 0xEF, it's not UTF8 BOM.
        // Just return true.
        return true;
    }

    is_.get();
    int x2 = is_.get();
    if (x2 != 0xBB) {
        return false;
    }

    int x3 = is_.get();
    if (x3 != 0xBF) {
        return false;
    }

    return true;
}

inline bool Lexer::current(char* c)
{
    int x = is_.peek();
    if (x == EOF)
        return false;
    *c = static_cast<char>(x);
    return true;
}

inline void Lexer::next()
{
    int x = is_.get();
    if (x == '\n')
        ++lineNo_;
}

inline bool Lexer::consume(char c)
{
    char x;
    if (!current(&x))
        return false;
    if (x != c)
        return false;
    next();
    return true;
}

inline void Lexer::skipUntilNewLine()
{
    char c;
    while (current(&c)) {
        if (c == '\n')
            return;
        next();
    }
}

inline Token Lexer::nextStringDoubleQuote()
{
    if (!consume('"'))
        return Token(TokenType::ILLEGAL, std::string("string didn't start with '\"'"));

    std::string s;
    char c;

    if (current(&c) && c == '"') {
        next();
        if (!current(&c) || c != '"') {
            // OK. It's empty string.
            return Token(TokenType::STRING, std::string());
        }
        return Token(TokenType::ILLEGAL, std::string("string didn't end"));
    }

    while (current(&c)) {
        next();
        if (c == '\\') {
            if (!current(&c))
                return Token(TokenType::ILLEGAL, std::string("string has unknown escape sequence"));
            next();
            switch (c) {
            case 't': c = '\t'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 'u':
            case 'U': {
                int size = c == 'u' ? 4 : 8;
                std::string codepoint;
                for (int i = 0; i < size; ++i) {
                  if (current(&c) && (('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'))) {
                    codepoint += c;
                    next();
                  } else {
                    return Token(TokenType::ILLEGAL, std::string("string has unknown escape sequence"));
                  }
                }
                s += unescape(codepoint);
                continue;
            }
            case '"': c = '"'; break;
            case '\'': c = '\''; break;
            case '\\': c = '\\'; break;
            case '\n':
                while (current(&c) && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                    next();
                }
                continue;
            default:
                return Token(TokenType::ILLEGAL, std::string("string has unknown escape sequence"));
            }
        } else if (c == '"') {
            return Token(TokenType::STRING, s);
        }

        s += c;
    }

    return Token(TokenType::ILLEGAL, std::string("string didn't end"));
}

inline Token Lexer::nextStringSingleQuote()
{
    if (!consume('\''))
        return Token(TokenType::ILLEGAL, std::string("string didn't start with '\''?"));

    std::string s;
    char c;

    if (current(&c) && c == '\'') {
        next();
        if (!current(&c) || c != '\'') {
            // OK. It's empty string.
            return Token(TokenType::STRING, std::string());
        }
        return Token(TokenType::ILLEGAL, std::string("string didn't end with \'' ?"));
    }

    while (current(&c)) {
        next();
        if (c == '\'') {
            return Token(TokenType::STRING, s);
        }

        s += c;
    }

    return Token(TokenType::ILLEGAL, std::string("string didn't end with '\''?"));
}

inline Token Lexer::nextHereDoc()
{
    if (!consume('<'))
        return Token(TokenType::ILLEGAL, std::string("heredoc didn't start with '<<'?"));
    if (!consume('<'))
        return Token(TokenType::ILLEGAL, std::string("heredoc didn't start with '<<'?"));

    std::string s;
    char c;

    next();
    // Indented heredoc syntax
    if(c == '-') {
        next();
    }

    while(current(&c) && (isalpha(c) || ('0' <= c && c <= '9'))) {
        s += c;
        next();
    }

    if(!current(&c)) {
        return Token(TokenType::ILLEGAL, std::string("end of file reached"));
    }

    if(current(&c) && c == '\r') {
        skipUntilNewLine();
    }

    if(!current(&c) || (current(&c) && c != '\n')) {
        return Token(TokenType::ILLEGAL, std::string("invalid characters in heredoc anchor"));
    }

    if(s.size() == 0) {
        return Token(TokenType::ILLEGAL, std::string("zero-length heredoc anchor"));
    }

    std::string buffer;
    std::string line;

    while(current(&c)) {
        next();
        if(current(&c) && c == '\n') {
            line.clear();
        }
        if(line.compare(s) == 0) {
            break;
        }
        buffer += c;
    }

    if(current(&c))
        return Token(TokenType::HEREDOC, buffer);
    else
        return Token(TokenType::ILLEGAL, std::string("heredoc not terminated"));
}

inline Token Lexer::nextValueToken()
{
    std::string s;
    char c;

    if (current(&c) && (isalpha(c) || c == '_')) {
        s += c;
        next();

        while (current(&c) && (isalpha(c) || c == '_' || c == '.')) {
            s += c;
            next();
        }

        if (s == "true")
            return Token(TokenType::BOOL, true);
        if (s == "false")
            return Token(TokenType::BOOL, false);
        return Token(TokenType::IDENT, s);
    }

    while (current(&c) && (('0' <= c && c <= '9') || c == '.' || c == 'e' || c == 'E' ||
                           c == 'T' || c == 'Z' || c == '_' || c == ':' || c == '-' || c == '+')) {
        next();
        s += c;
    }

    if (isInteger(s)) {
        std::stringstream ss(removeDelimiter(s));
        std::int64_t x;
        ss >> x;
        return Token(TokenType::NUMBER, x);
    }

    if (isDouble(s)) {
        std::stringstream ss(removeDelimiter(s));
        double d;
        ss >> d;
        return Token(TokenType::FLOAT, d);
    }

    return Token(TokenType::ILLEGAL, std::string("Invalid token"));
}

inline bool isWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline Token Lexer::nextToken()
{
    if (peek_.type() != TokenType::ILLEGAL) {
        Token tok = peek_;
        peek_ = Token(TokenType::ILLEGAL);
        return tok;
    }

    char c;
    while (current(&c)) {
        if (isWhitespace(c)) {
            next();
            continue;
        }

        if (c == '#') {
            skipUntilNewLine();
            continue;
        }

        switch (c) {
        case '=':
            next();
            return Token(TokenType::ASSIGN);
        case '+':
            next();
            return Token(TokenType::ADD);
        case '-':
            next();
            return Token(TokenType::SUB);
        case '{':
            next();
            return Token(TokenType::LBRACE);
        case '}':
            next();
            return Token(TokenType::RBRACE);
        case '[':
            next();
            return Token(TokenType::LBRACK);
        case ']':
            next();
            return Token(TokenType::RBRACK);
        case ',':
            next();
            return Token(TokenType::COMMA);
        case '.':
            next();
            return Token(TokenType::PERIOD);
        case '\"':
            return nextStringDoubleQuote();
        case '\'':
            return nextStringSingleQuote();
        case '<':
            return nextHereDoc();
        default:
            return nextValueToken();
        }
    }

    return Token(TokenType::END_OF_FILE);
}

} // namespace internal

// static
inline const char* Value::typeToString(Value::Type type)
{
    switch (type) {
    case NULL_TYPE:   return "null";
    case BOOL_TYPE:   return "bool";
    case INT_TYPE:    return "int";
    case DOUBLE_TYPE: return "double";
    case STRING_TYPE: return "string";
    case ARRAY_TYPE:  return "array";
    case OBJECT_TYPE:  return "object";
    default:          return "unknown";
    }
}

inline Value::Value(const Value& v) :
    type_(v.type_)
{
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = new std::string(*v.string_); break;
    case ARRAY_TYPE: array_ = new Array(*v.array_); break;
    case OBJECT_TYPE: object_ = new Object(*v.object_); break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }
}

inline Value::Value(Value&& v) noexcept :
    type_(v.type_)
{
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = v.string_; break;
    case ARRAY_TYPE: array_ = v.array_; break;
    case OBJECT_TYPE: object_ = v.object_; break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }

    v.type_ = NULL_TYPE;
    v.null_ = nullptr;
}

inline Value& Value::operator=(const Value& v)
{
    if (this == &v)
        return *this;

    this->~Value();

    type_ = v.type_;
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = new std::string(*v.string_); break;
    case ARRAY_TYPE: array_ = new Array(*v.array_); break;
    case OBJECT_TYPE: object_ = new Object(*v.object_); break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }

    return *this;
}

inline Value& Value::operator=(Value&& v) noexcept
{
    if (this == &v)
        return *this;

    this->~Value();

    type_ = v.type_;
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = v.string_; break;
    case ARRAY_TYPE: array_ = v.array_; break;
    case OBJECT_TYPE: object_ = v.object_; break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }

    v.type_ = NULL_TYPE;
    v.null_ = nullptr;
    return *this;
}

inline Value::~Value()
{
    switch (type_) {
    case STRING_TYPE:
        delete string_;
        break;
    case ARRAY_TYPE:
        delete array_;
        break;
    case OBJECT_TYPE:
        delete object_;
        break;
    default:
        break;
    }
}

inline size_t Value::size() const
{
    switch (type_) {
    case NULL_TYPE:
        return 0;
    case ARRAY_TYPE:
        return array_->size();
    case OBJECT_TYPE:
        return object_->size();
    default:
        return 1;
    }
}

inline bool Value::empty() const
{
    return size() == 0;
}

template<> struct Value::ValueConverter<bool>
{
    bool is(const Value& v) { return v.type() == Value::BOOL_TYPE; }
    bool to(const Value& v) { v.assureType<bool>(); return v.bool_; }

};
template<> struct Value::ValueConverter<int64_t>
{
    bool is(const Value& v) { return v.type() == Value::INT_TYPE; }
    int64_t to(const Value& v) { v.assureType<int64_t>(); return v.int_; }
};
template<> struct Value::ValueConverter<int>
{
    bool is(const Value& v) { return v.type() == Value::INT_TYPE; }
    int to(const Value& v) { v.assureType<int>(); return static_cast<int>(v.int_); }
};
template<> struct Value::ValueConverter<double>
{
    bool is(const Value& v) { return v.type() == Value::DOUBLE_TYPE; }
    double to(const Value& v) { v.assureType<double>(); return v.double_; }
};
template<> struct Value::ValueConverter<std::string>
{
    bool is(const Value& v) { return v.type() == Value::STRING_TYPE; }
    const std::string& to(const Value& v) { v.assureType<std::string>(); return *v.string_; }
};
template<> struct Value::ValueConverter<Array>
{
    bool is(const Value& v) { return v.type() == Value::ARRAY_TYPE; }
    const Array& to(const Value& v) { v.assureType<Array>(); return *v.array_; }
};
template<> struct Value::ValueConverter<Object>
{
    bool is(const Value& v) { return v.type() == Value::OBJECT_TYPE; }
    const Object& to(const Value& v) { v.assureType<Object>(); return *v.object_; }
};

template<typename T>
struct Value::ValueConverter<std::vector<T>>
{
    bool is(const Value& v)
    {
        if (v.type() != Value::ARRAY_TYPE)
            return false;
        const Array& array = v.as<Array>();
        if (array.empty())
            return true;
        return array.front().is<T>();
    }

    std::vector<T> to(const Value& v)
    {
        const Array& array = v.as<Array>();
        if (array.empty())
            return std::vector<T>();
        array.front().assureType<T>();

        std::vector<T> result;
        for (const auto& element : array) {
            result.push_back(element.as<T>());
        }

        return result;
    }
};

namespace internal {
template<typename T> inline const char* type_name();
template<> inline const char* type_name<bool>() { return "bool"; }
template<> inline const char* type_name<int>() { return "int"; }
template<> inline const char* type_name<int64_t>() { return "int64_t"; }
template<> inline const char* type_name<double>() { return "double"; }
template<> inline const char* type_name<std::string>() { return "string"; }
template<> inline const char* type_name<hcl::Array>() { return "array"; }
template<> inline const char* type_name<hcl::Object>() { return "object"; }
} // namespace internal

template<typename T>
inline void Value::assureType() const
{
    if (!is<T>())
        failwith("type error: this value is ", typeToString(type_), " but ", internal::type_name<T>(), " was requested");
}

template<typename T>
inline bool Value::is() const
{
    return ValueConverter<T>().is(*this);
}

template<typename T>
inline typename call_traits<T>::return_type Value::as() const
{
    return ValueConverter<T>().to(*this);
}

inline bool Value::isNumber() const
{
    return is<int>() || is<double>();
}

inline double Value::asNumber() const
{
    if (is<int>())
        return as<int>();
    if (is<double>())
        return as<double>();

    failwith("type error: this value is ", typeToString(type_), " but number is requested");
}

// static
inline bool operator==(const Value& lhs, const Value& rhs)
{
    if (lhs.type() != rhs.type())
        return false;

    switch (lhs.type()) {
    case Value::Type::NULL_TYPE:
        return true;
    case Value::Type::BOOL_TYPE:
        return lhs.bool_ == rhs.bool_;
    case Value::Type::INT_TYPE:
        return lhs.int_ == rhs.int_;
    case Value::Type::DOUBLE_TYPE:
        return lhs.double_ == rhs.double_;
    case Value::Type::STRING_TYPE:
        return *lhs.string_ == *rhs.string_;
    case Value::Type::ARRAY_TYPE:
        return *lhs.array_ == *rhs.array_;
    case Value::Type::OBJECT_TYPE:
        return *lhs.object_ == *rhs.object_;
    default:
        failwith("unknown type");
    }
}

template<typename T>
inline typename call_traits<T>::return_type Value::get(const std::string& key) const
{
    if (!is<Object>())
        failwith("type must be object to do get(key).");

    const Value* obj = find(key);
    if (!obj)
        failwith("key ", key, " was not found.");

    return obj->as<T>();
}

inline const Value* Value::find(const std::string& key) const
{
    if (!is<Object>())
        return nullptr;

    std::istringstream ss(key);
    internal::Lexer lexer(ss);

    const Value* current = this;
    while (true) {
        internal::Token t = lexer.nextToken();
        if (!(t.type() == internal::TokenType::IDENT || t.type() == internal::TokenType::STRING))
            return nullptr;

        std::string part = t.strValue();
        t = lexer.nextToken();
        if (t.type() == internal::TokenType::PERIOD) {
            current = current->findChild(part);
            if (!current || !current->is<Object>())
                return nullptr;
        } else if (t.type() == internal::TokenType::END_OF_FILE) {
            return current->findChild(part);
        } else {
            return nullptr;
        }
    }
}

inline Value* Value::find(const std::string& key)
{
    return const_cast<Value*>(const_cast<const Value*>(this)->find(key));
}

inline bool Value::merge(const hcl::Value& v)
{
    if (this == &v)
        return true;
    if (!is<Object>() || !v.is<Object>())
        return false;

    for (const auto& kv : *v.object_) {
        if (Value* tmp = find(kv.first)) {
            // If both are object, we merge them.
            if (tmp->is<Object>() && kv.second.is<Object>()) {
                if (!tmp->merge(kv.second))
                    return false;
            } else {
                setChild(kv.first, kv.second);
            }
        } else {
            setChild(kv.first, kv.second);
        }
    }

    return true;
}

inline Value* Value::set(const std::string& key, const Value& v)
{
    Value* result = ensureValue(key);
    *result = v;
    return result;
}

inline Value* Value::setChild(const std::string& key, const Value& v)
{
    if (!valid())
        *this = Value((Object()));

    if (!is<Object>())
        failwith("type must be object to do set(key, v).");

    (*object_)[key] = v;
    return &(*object_)[key];
}

inline Value* Value::setChild(const std::string& key, Value&& v)
{
    if (!valid())
        *this = Value((Object()));

    if (!is<Object>())
        failwith("type must be object to do set(key, v).");

    (*object_)[key] = std::move(v);
    return &(*object_)[key];
}

inline bool Value::erase(const std::string& key)
{
    if (!is<Object>())
        return false;

    std::istringstream ss(key);
    internal::Lexer lexer(ss);

    Value* current = this;
    while (true) {
        internal::Token t = lexer.nextToken();
        if (!(t.type() == internal::TokenType::IDENT || t.type() == internal::TokenType::STRING))
            return false;

        std::string part = t.strValue();
        t = lexer.nextToken();
        if (t.type() == internal::TokenType::PERIOD) {
            current = current->findChild(part);
            if (!current || !current->is<Object>())
                return false;
        } else if (t.type() == internal::TokenType::END_OF_FILE) {
            return current->eraseChild(part);
        } else {
            return false;
        }
    }
}

inline bool Value::eraseChild(const std::string& key)
{
    if (!is<Object>())
        failwith("type must be object to do erase(key).");

    return object_->erase(key) > 0;
}

inline Value& Value::operator[](const std::string& key)
{
    if (!valid())
        *this = Value((Object()));

    if (Value* v = findChild(key))
        return *v;

    return *setChild(key, Value());
}

template<typename T>
inline typename call_traits<T>::return_type Value::get(size_t index) const
{
    if (!is<Array>())
        failwith("type must be array to do get(index).");

    if (array_->size() <= index)
        failwith("index out of bound");

    return (*array_)[index].as<T>();
}

inline const Value* Value::find(size_t index) const
{
    if (!is<Array>())
        return nullptr;
    if (index < array_->size())
        return &(*array_)[index];
    return nullptr;
}

inline Value* Value::find(size_t index)
{
    return const_cast<Value*>(const_cast<const Value*>(this)->find(index));
}

inline Value* Value::push(const Value& v)
{
    if (!valid())
        *this = Value((Array()));
    else if (!is<Array>())
        failwith("type must be array to do push(Value).");

    array_->push_back(v);
    return &array_->back();
}

inline Value* Value::push(Value&& v)
{
    if (!valid())
        *this = Value((Array()));
    else if (!is<Array>())
        failwith("type must be array to do push(Value).");

    array_->push_back(std::move(v));
    return &array_->back();
}

inline Value* Value::ensureValue(const std::string& key)
{
    if (!valid())
        *this = Value((Object()));
    if (!is<Object>()) {
        failwith("encountered non object value");
    }

    std::istringstream ss(key);
    internal::Lexer lexer(ss);

    Value* current = this;
    while (true) {
        internal::Token t = lexer.nextToken();
        if (!(t.type() == internal::TokenType::IDENT || t.type() == internal::TokenType::STRING)) {
            failwith("invalid key");
        }

        std::string part = t.strValue();
        t = lexer.nextToken();
        if (t.type() == internal::TokenType::PERIOD) {
            if (Value* candidate = current->findChild(part)) {
                if (!candidate->is<Object>())
                    failwith("encountered non object value");

                current = candidate;
            } else {
                current = current->setChild(part, Object());
            }
        } else if (t.type() == internal::TokenType::END_OF_FILE) {
            if (Value* v = current->findChild(part))
                return v;
            return current->setChild(part, Value());
        } else {
            failwith("invalid key");
        }
    }
}

inline Value* Value::findChild(const std::string& key)
{
    assert(is<Object>());

    auto it = object_->find(key);
    if (it == object_->end())
        return nullptr;

    return &it->second;
}

inline const Value* Value::findChild(const std::string& key) const
{
    assert(is<Object>());

    auto it = object_->find(key);
    if (it == object_->end())
        return nullptr;

    return &it->second;
}

// ----------------------------------------------------------------------
// Parser

namespace internal {

inline void Parser::addError(const std::string& reason)
{
    if (!errorReason_.empty())
        return;

    std::stringstream ss;
    ss << "Error: line " << lexer_.lineNo() << ": " << reason;
    errorReason_ = ss.str();
}

inline const std::string& Parser::errorReason()
{
    return errorReason_;
}

inline Value Parser::parse()
{
    return parseObjectList(false);
}

inline Value Parser::parseObjectList(bool isNested)
{
    Value node((Object()));

    while (true) {
        if (peekToken().type() == TokenType::END_OF_FILE) {
            break;
        }
        if (isNested) {
            // We're inside a nested object list, so end at an RBRACE.
            if (peekToken().type() == TokenType::RBRACE) {
                break;
            }
        }

        std::vector<std::string> keys;
        if (!parseKeys(keys)) {
            break;
        }

        Value v;
        if (!parseObjectItem(v)) {
            addError("parse object item failure");
            break;
        }

        Value* parent = &node;
        Value* array = nullptr;
        for (const auto& key : keys) {
            array = parent->find(key);
            if(array && !array->is<Array>()) {
                addError("tried setting nested object that wasn't an array");
                array = nullptr;
                break;
            }
            if(!array) {
                parent->setChild(key, Array());
            }
            parent = array;
        }

        if (array) {
            array->setChild(keys.back(), std::move(v));
        } else {
            addError("no keys were found");
            break;
        }
    }

    return node;
}

inline bool Parser::parseKeys(std::vector<std::string>& keys)
{
    int keyCount = 0;
    keys.clear();

    while (true) {
        nextToken();

        switch (token().type()) {
        case TokenType::END_OF_FILE:
            addError("end of file reached");
            return false;
        case TokenType::ASSIGN:
            if (keyCount > 1) {
                addError("nested object expected: LBRACE got: " + token().strValue());
                return false;
            }
            if (keyCount == 0) {
                addError("no object keys found");
                return false;
            }

            return true;
        case TokenType::LBRACE:
            if (keys.size() == 0) {
                addError("expected IDENT | STRING got: " + token().strValue());
                return false;
            }

            return true;
        case TokenType::IDENT:
        case TokenType::STRING:
            keyCount++;
            keys.push_back(token().strValue());
            break;
        case TokenType::ILLEGAL:
            addError("illegal character");
            return false;
        default:
            addError("expected IDENT | STRING | ASSIGN | LBRACE got: " + token().strValue());
            return false;
        }
    }

    return false;
}

inline bool Parser::parseObjectItem(Value& currentValue)
{
    switch (token().type()) {
    case TokenType::ASSIGN:
        if (!parseObject(currentValue))
            return false;
        break;
    case TokenType::LBRACE:
        if (!parseObjectType(currentValue))
            return false;
        break;
    default:
        addError("Expected start of object ('{') or assignment ('=')");
        return false;
    }
    return true;
}

inline bool Parser::parseObject(Value& currentValue)
{
    nextToken();

    switch (token().type()) {
    case TokenType::NUMBER:
    case TokenType::FLOAT:
    case TokenType::BOOL:
    case TokenType::STRING:
    case TokenType::HEREDOC:
        return parseLiteralType(currentValue);
    case TokenType::LBRACE:
        return parseObjectType(currentValue);
    case TokenType::LBRACK:
        return parseListType(currentValue);
    case TokenType::COMMENT:
        break;
    case TokenType::END_OF_FILE:
        addError("Reached end of file");
        return false;
    }

    addError("Unknown token");
    return false;
}

inline bool Parser::parseObjectType(Value& currentValue)
{
    Value result = parseObjectList(true);

    if(!errorReason().empty() && token().type() != TokenType::RBRACE) {
        addError("failed parsing object list");
        return false;
    }

    nextToken();

    if(token().type() != TokenType::RBRACE) {
        addError("object expected closing RBRACE got: " + token().strValue());
        return false;
    }

    currentValue = result;
    return true;
}

inline bool Parser::parseListType(Value& currentValue)
{
    Array a;
    bool needComma = false;

    while (true) {
        nextToken();

        if (needComma) {
            switch (token().type()) {
            case TokenType::COMMA:
            case TokenType::RBRACK:
                break;
            default:
                addError("error parsing list, expected comma or list end, got: " + token().strValue());
                return false;
            }
        }

        switch (token().type()) {
        case TokenType::BOOL:
        case TokenType::NUMBER:
        case TokenType::FLOAT:
        case TokenType::STRING:
        case TokenType::HEREDOC:
        {
            Value literal;
            if (!parseLiteralType(literal)) {
                addError("error parsing literal type");
                return false;
            }

            a.push_back(std::move(literal));
            needComma = true;
            break;
        }
        case TokenType::COMMA:
            needComma = false;
            continue;
        case TokenType::LBRACE:
        {
            Value object;
            if (!parseObjectType(object)) {
                addError("error parsing object within list");
                return false;
            }
            a.push_back(std::move(object));
            needComma = true;
            break;
        }
        case TokenType::LBRACK:
        {
            Value list;
            if (!parseListType(list)) {
                addError("error parsing list within list");
                return false;
            }
            a.push_back(std::move(list));
            break;
        }
        case TokenType::RBRACK:
        {
            currentValue = a;
            return true;
        }
        default:
            addError("unexpected token while parsing list: " + token().strValue());
            return false;
        }
    }
    return false;
}

bool Parser::parseLiteralType(Value& currentValue)
{
    switch (token().type()) {
    case TokenType::STRING:
    case TokenType::HEREDOC:
        currentValue = token().strValue();
        return true;
    case TokenType::BOOL:
        currentValue = token().boolValue();
        return true;
    case TokenType::NUMBER:
        currentValue = token().intValue();
        return true;
    case TokenType::FLOAT:
        currentValue = token().doubleValue();
        return true;
    case TokenType::ILLEGAL:
        addError(token().strValue());
        return false;
    default:
        addError("unexpected token");
        return false;
    }
}


} // namespace internal

} // namespace hcl

#endif // MICROHCL_H_
