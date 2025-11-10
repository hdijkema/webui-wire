#include "json.h"

using std::map;
using std::deque;
using std::string;
using std::enable_if;
using std::initializer_list;
using std::is_same;
using std::is_convertible;
using std::is_integral;
using std::is_floating_point;

std::string json_escape(const string &str) {
    string output;
    for( unsigned i = 0; i < str.length(); ++i )
        switch( str[i] ) {
        case '\"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\b': output += "\\b";  break;
        case '\f': output += "\\f";  break;
        case '\n': output += "\\n";  break;
        case '\r': output += "\\r";  break;
        case '\t': output += "\\t";  break;
        default  : output += str[i]; break;
        }
    return std::move( output );
}

JSON::JSON() : Internal(), Type( Class::Null ){}

JSON::JSON(std::initializer_list<JSON> list)
    : JSON()
{
    SetType( Class::Object );
    for( auto i = list.begin(), e = list.end(); i != e; ++i, ++i )
        operator[]( i->toString() ) = *std::next( i );
}

JSON::JSON(JSON &&other)
    : Internal( other.Internal )
    , Type( other.Type )
{ other.Type = Class::Null; other.Internal.Map = nullptr; }

JSON::JSON(const JSON &other) {
    switch( other.Type ) {
    case Class::Object:
        Internal.Map =
            new map<string,JSON>( other.Internal.Map->begin(),
                                  other.Internal.Map->end() );
        break;
    case Class::Array:
        Internal.List =
            new deque<JSON>( other.Internal.List->begin(),
                            other.Internal.List->end() );
        break;
    case Class::String:
        Internal.String =
            new string( *other.Internal.String );
        break;
    default:
        Internal = other.Internal;
    }
    Type = other.Type;
}

JSON JSON::Make(Class type) {
    JSON ret; ret.SetType( type );
    return ret;
}

JSON &JSON::operator[](unsigned int index) {
    SetType( Class::Array );
    if( index >= Internal.List->size() ) Internal.List->resize( index + 1 );
    return Internal.List->operator[]( index );
}

JSON &JSON::at(const std::string &key) {
    return operator[]( key );
}

const JSON &JSON::at(const std::string &key) const {
    return Internal.Map->at( key );
}

JSON &JSON::at(unsigned int index) {
    return operator[]( index );
}

const JSON &JSON::at(unsigned int index) const {
    return Internal.List->at( index );
}

int JSON::length() const {
    if( Type == Class::Array )
        return Internal.List->size();
    else
        return -1;
}

bool JSON::hasKey(const std::string &key) const {
    if( Type == Class::Object )
        return Internal.Map->find( key ) != Internal.Map->end();
    return false;
}

int JSON::size() const {
    if( Type == Class::Object )
        return Internal.Map->size();
    else if( Type == Class::Array )
        return Internal.List->size();
    else
        return -1;
}

JSON::Class JSON::JSONType() const { return Type; }

bool JSON::IsNull() const { return Type == Class::Null; }

string JSON::toString() const { bool b; return std::move( toString( b ) ); }

string JSON::toString(bool &ok) const {
    ok = (Type == Class::String);
    return ok ? std::move( json_escape( *Internal.String ) ): string("");
}

double JSON::toFloat() const { bool b; return toFloat( b ); }

double JSON::toFloat(bool &ok) const {
    ok = (Type == Class::Floating);
    return ok ? Internal.Float : 0.0;
}

long JSON::toInt() const { bool b; return toInt( b ); }

long JSON::toInt(bool &ok) const {
    ok = (Type == Class::Integral);
    return ok ? Internal.Int : 0;
}

bool JSON::toBool() const { bool b; return toBool( b ); }

bool JSON::toBool(bool &ok) const {
    ok = (Type == Class::Boolean);
    return ok ? Internal.Bool : false;
}

JSON::JSONWrapper<std::map<string, JSON> > JSON::ObjectRange() {
    if( Type == Class::Object )
        return JSONWrapper<std::map<std::string,JSON>>( Internal.Map );
    return JSONWrapper<std::map<std::string,JSON>>( nullptr );
}

JSON::JSONWrapper<std::deque<JSON> > JSON::ArrayRange() {
    if( Type == Class::Array )
        return JSONWrapper<std::deque<JSON>>( Internal.List );
    return JSONWrapper<std::deque<JSON>>( nullptr );
}

JSON::JSONConstWrapper<std::map<string, JSON> > JSON::ObjectRange() const {
    if( Type == Class::Object )
        return JSONConstWrapper<std::map<std::string,JSON>>( Internal.Map );
    return JSONConstWrapper<std::map<std::string,JSON>>( nullptr );
}

JSON::JSONConstWrapper<std::deque<JSON> > JSON::ArrayRange() const {
    if( Type == Class::Array )
        return JSONConstWrapper<std::deque<JSON>>( Internal.List );
    return JSONConstWrapper<std::deque<JSON>>( nullptr );
}

string JSON::dump(int depth, const std::string &tab) const {
    std::string pad = "";
    for( int i = 0; i < depth; ++i, pad += tab );

    switch( Type ) {
    case Class::Null:
        return "null";
    case Class::Object: {
        std::string s = "{ ";
        bool skip = true;
        for( auto &p : *Internal.Map ) {
            if( !skip ) s += ", ";
            s += ( pad + "\"" + p.first + "\" : " + p.second.dump( depth + 1, tab ) );
            skip = false;
        }
        s += ( " " + pad.erase( 0, 2 ) + "}" ) ;
        return s;
    }
    case Class::Array: {
        std::string s = "[";
        bool skip = true;
        for( auto &p : *Internal.List ) {
            if( !skip ) s += ", ";
            s += p.dump( depth + 1, tab );
            skip = false;
        }
        s += "]";
        return s;
    }
    case Class::String:
        return "\"" + json_escape( *Internal.String ) + "\"";
    case Class::Floating:
        return std::to_string( Internal.Float );
    case Class::Integral:
        return std::to_string( Internal.Int );
    case Class::Boolean:
        return Internal.Bool ? "true" : "false";
    default:
        return "";
    }
    return "";
}

void JSON::SetType(Class type) {
    if( type == Type )
        return;

    ClearInternal();

    switch( type ) {
    case Class::Null:      Internal.Map    = nullptr;                           break;
    case Class::Object:    Internal.Map    = new std::map<std::string,JSON>();  break;
    case Class::Array:     Internal.List   = new std::deque<JSON>();            break;
    case Class::String:    Internal.String = new std::string();                 break;
    case Class::Floating:  Internal.Float  = 0.0;                               break;
    case Class::Integral:  Internal.Int    = 0;                                 break;
    case Class::Boolean:   Internal.Bool   = false;                             break;
    }

    Type = type;
}

void JSON::ClearInternal() {
    switch( Type ) {
    case Class::Object: delete Internal.Map;    break;
    case Class::Array:  delete Internal.List;   break;
    case Class::String: delete Internal.String; break;
    default:;
    }
}

JSON &JSON::operator[](const std::string &key) {
    SetType( Class::Object ); return Internal.Map->operator[]( key );
}

JSON &JSON::operator=(const JSON &other) {

    if (&other == this) { return *this; }

    ClearInternal();
    switch( other.Type ) {
    case Class::Object:
        Internal.Map =
            new map<string,JSON>( other.Internal.Map->begin(),
                                  other.Internal.Map->end() );
        break;
    case Class::Array:
        Internal.List =
            new deque<JSON>( other.Internal.List->begin(),
                            other.Internal.List->end() );
        break;
    case Class::String:
        Internal.String =
            new string( *other.Internal.String );
        break;
    default:
        Internal = other.Internal;
    }
    Type = other.Type;
    return *this;
}

JSON &JSON::operator=(JSON &&other) {
    ClearInternal();
    Internal = other.Internal;
    Type = other.Type;
    other.Internal.Map = nullptr;
    other.Type = Class::Null;
    return *this;
}

JSON Array() {
    return std::move( JSON::Make( JSON::Class::Array ) );
}

//JSON Object() { return std::move(JSON::Make(JSON::Class::Object)); }

JSON Object() {
    return std::move( JSON::Make( JSON::Class::Object ) );
}

std::ostream &operator<<(std::ostream &os, const JSON &json) {
    os << json.dump();
    return os;
}

// Private functions.

static JSON parse_next( const std::string &, size_t &, std::function<void(const std::string &err)> );


static void consume_ws( const std::string &str, size_t &offset ) {
    while( isspace( str[offset] ) ) ++offset;
}

static JSON parse_object( const string &str, size_t &offset, std::function<void(const std::string &err)> on_error ) {
    JSON Object = JSON::Make( JSON::Class::Object );

    ++offset;
    consume_ws( str, offset );
    if( str[offset] == '}' ) {
        ++offset; return std::move( Object );
    }

    while( true ) {
        JSON Key = parse_next( str, offset, on_error );
        consume_ws( str, offset );
        if( str[offset] != ':' ) {
            on_error(std::string("Error: Object: Expected colon, found '") + str[offset] + "'");
            break;
        }
        consume_ws( str, ++offset );
        JSON Value = parse_next( str, offset, on_error );
        Object[Key.toString()] = Value;

        consume_ws( str, offset );
        if( str[offset] == ',' ) {
            ++offset; continue;
        }
        else if( str[offset] == '}' ) {
            ++offset; break;
        }
        else {
            on_error(std::string("ERROR: Object: Expected comma, found '") + str[offset] + "'");
            break;
        }
    }

    return std::move( Object );
}

static JSON parse_array( const string &str, size_t &offset, std::function<void(const std::string &err)> on_error ) {
    JSON Array = JSON::Make( JSON::Class::Array );
    unsigned index = 0;

    ++offset;
    consume_ws( str, offset );
    if( str[offset] == ']' ) {
        ++offset; return std::move( Array );
    }

    while( true ) {
        Array[index++] = parse_next( str, offset, on_error );
        consume_ws( str, offset );

        if( str[offset] == ',' ) {
            ++offset; continue;
        }
        else if( str[offset] == ']' ) {
            ++offset; break;
        }
        else {
            on_error(std::string("ERROR: Array: Expected ',' or ']', found '") + str[offset] + "'");
            return std::move( JSON::Make( JSON::Class::Array ) );
        }
    }

    return std::move( Array );
}

static JSON parse_string( const string &str, size_t &offset, std::function<void(const std::string &err)> on_error ) {
    JSON String;
    string val;
    for( char c = str[++offset]; c != '\"' ; c = str[++offset] ) {
        if( c == '\\' ) {
            switch( str[ ++offset ] ) {
            case '\"': val += '\"'; break;
            case '\\': val += '\\'; break;
            case '/' : val += '/' ; break;
            case 'b' : val += '\b'; break;
            case 'f' : val += '\f'; break;
            case 'n' : val += '\n'; break;
            case 'r' : val += '\r'; break;
            case 't' : val += '\t'; break;
            case 'u' : {
                val += "\\u" ;
                for( unsigned i = 1; i <= 4; ++i ) {
                    c = str[offset+i];
                    if( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') )
                        val += c;
                    else {
                        on_error(std::string("ERROR: String: Expected hex character in unicode escape, found '") + c + "'");
                        return std::move( JSON::Make( JSON::Class::String ) );
                    }
                }
                offset += 4;
            } break;
            default  : val += '\\'; break;
            }
        }
        else
            val += c;
    }
    ++offset;
    String = val;
    return std::move( String );
}

static JSON parse_number( const string &str, size_t &offset, std::function<void(const std::string &err)> on_error ) {
    JSON Number;
    string val, exp_str;
    char c;
    bool isDouble = false;
    long exp = 0;
    while( true ) {
        c = str[offset++];
        if( (c == '-') || (c >= '0' && c <= '9') )
            val += c;
        else if( c == '.' ) {
            val += c;
            isDouble = true;
        }
        else
            break;
    }
    if( c == 'E' || c == 'e' ) {
        c = str[ offset++ ];
        if( c == '-' ){ ++offset; exp_str += '-';}
        while( true ) {
            c = str[ offset++ ];
            if( c >= '0' && c <= '9' )
                exp_str += c;
            else if( !isspace( c ) && c != ',' && c != ']' && c != '}' ) {
                on_error(std::string("ERROR: Number: Expected a number for exponent, found '") + c + "'");
                return std::move( JSON::Make( JSON::Class::Null ) );
            }
            else
                break;
        }
        exp = std::stol( exp_str );
    }
    else if( !isspace( c ) && c != ',' && c != ']' && c != '}' ) {
        on_error(std::string("ERROR: Number: unexpected character '") + c + "'");
        return std::move( JSON::Make( JSON::Class::Null ) );
    }
    --offset;

    if( isDouble )
        Number = std::stod( val ) * std::pow( 10, exp );
    else {
        if( !exp_str.empty() )
            Number = std::stol( val ) * std::pow( 10, exp );
        else
            Number = std::stol( val );
    }
    return std::move( Number );
}

static JSON parse_bool( const string &str, size_t &offset, std::function<void(const std::string &err)> on_error ) {
    JSON Bool;
    if( str.substr( offset, 4 ) == "true" )
        Bool = true;
    else if( str.substr( offset, 5 ) == "false" )
        Bool = false;
    else {
        on_error(std::string("ERROR: Bool: Expected 'true' or 'false', found '") + str.substr( offset, 5 ) + "'");
        return std::move( JSON::Make( JSON::Class::Null ) );
    }
    offset += (Bool.toBool() ? 4 : 5);
    return std::move( Bool );
}

static JSON parse_null( const string &str, size_t &offset, std::function<void(const std::string &err)> on_error ) {
    JSON Null;
    if( str.substr( offset, 4 ) != "null" ) {
        on_error(std::string("ERROR: Null: Expected 'null', found '") + str.substr( offset, 4 ) + "'" );
        return std::move( JSON::Make( JSON::Class::Null ) );
    }
    offset += 4;
    return std::move( Null );
}

static JSON parse_next( const string &str, size_t &offset, std::function<void(const std::string &err)> on_error ) {
    char value;
    consume_ws( str, offset );
    value = str[offset];
    switch( value ) {
    case '[' : return std::move( parse_array( str, offset, on_error ) );
    case '{' : return std::move( parse_object( str, offset, on_error ) );
    case '\"': return std::move( parse_string( str, offset, on_error ) );
    case 't' :
    case 'f' : return std::move( parse_bool( str, offset, on_error ) );
    case 'n' : return std::move( parse_null( str, offset, on_error ) );
    default  : if( ( value <= '9' && value >= '0' ) || value == '-' )
            return std::move( parse_number( str, offset, on_error ) );
    }
    on_error(std::string("ERROR: Parse: Unknown starting character '") + value + "'");
    return JSON();
}

JSON JSON::Load( const string &str, std::function<void(const std::string &err)> on_error) {
    size_t offset = 0;
    return std::move( parse_next( str, offset, on_error ) );
}

