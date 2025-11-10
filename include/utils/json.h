#ifndef JSON_H
#define JSON_H

#include <cstdint>
#include <cmath>
#include <cctype>
#include <string>
#include <deque>
#include <map>
#include <type_traits>
#include <initializer_list>
#include <ostream>
#include <iostream>
#include <functional>

std::string json_escape( const std::string &str );

class JSON
{
    union BackingData {
        BackingData( double d )         : Float( d ){}
        BackingData( long   l )         : Int( l ){}
        BackingData( bool   b )         : Bool( b ){}
        BackingData( std::string s )    : String( new std::string( s ) ){}
        BackingData()                   : Int( 0 ){}

        std::deque<JSON>             *List;
        std::map<std::string,JSON>   *Map;
        std::string                  *String;
        double                        Float;
        long                          Int;
        bool                          Bool;
    } Internal;

public:
    enum class Class {
        Null,
        Object,
        Array,
        String,
        Floating,
        Integral,
        Boolean
    };

    template <typename Container>
    class JSONWrapper {
        Container *object;

    public:
        JSONWrapper( Container *val ) : object( val ) {}
        JSONWrapper( std::nullptr_t )  : object( nullptr ) {}

        typename Container::iterator begin() { return object ? object->begin() : typename Container::iterator(); }
        typename Container::iterator end() { return object ? object->end() : typename Container::iterator(); }
        typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::iterator(); }
        typename Container::const_iterator end() const { return object ? object->end() : typename Container::iterator(); }
    };

    template <typename Container>
    class JSONConstWrapper {
        const Container *object;

    public:
        JSONConstWrapper( const Container *val ) : object( val ) {}
        JSONConstWrapper( std::nullptr_t )  : object( nullptr ) {}

        typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::const_iterator(); }
        typename Container::const_iterator end() const { return object ? object->end() : typename Container::const_iterator(); }
    };

    JSON();
    JSON( std::nullptr_t ) : Internal(), Type( Class::Null ){}
    JSON( std::initializer_list<JSON> list );
    JSON( JSON&& other );
    JSON( const JSON &other );

    JSON& operator=( const JSON &other );
    JSON& operator=( JSON&& other );

    // Template T constructors
    template <typename T>
    JSON( T b, typename std::enable_if<std::is_same<T,bool>::value>::type* = 0 );

    template <typename T>
    JSON( T i, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T,bool>::value>::type* = 0 );

    template <typename T>
    JSON( T f, typename std::enable_if<std::is_floating_point<T>::value>::type* = 0 );

    template <typename T>
    JSON( T s, typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = 0 );

    ~JSON() {
        switch( Type ) {
        case Class::Array:
            delete Internal.List;
            break;
        case Class::Object:
            delete Internal.Map;
            break;
        case Class::String:
            delete Internal.String;
            break;
        default:;
        }
    }

    static JSON Make( Class type );
    static JSON Load( const std::string &, std::function<void(const std::string &err)> );

    // Appending things.

    template <typename T>
    void append( T arg ) {
        SetType( Class::Array ); Internal.List->emplace_back( arg );
    }

    template <typename T, typename... U>
    void append( T arg, U... args ) {
        append( arg ); append( args... );
    }

    // Assignments (template T).

    template <typename T>
    typename std::enable_if<std::is_same<T,bool>::value, JSON&>::type operator=( T b ) {
        SetType( Class::Boolean ); Internal.Bool = b; return *this;
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T,bool>::value, JSON&>::type operator=( T i ) {
        SetType( Class::Integral ); Internal.Int = i; return *this;
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, JSON&>::type operator=( T f ) {
        SetType( Class::Floating ); Internal.Float = f; return *this;
    }

    template <typename T>
    typename std::enable_if<std::is_convertible<T, std::string>::value, JSON&>::type operator=( T s ) {
        SetType( Class::String ); *Internal.String = std::string( s ); return *this;
    }



    // Indexing.
    JSON& operator[]( const std::string &key );
    JSON& operator[]( unsigned index );

    JSON &at( const std::string &key );
    const JSON &at( const std::string &key ) const;
    JSON &at( unsigned index );
    const JSON &at( unsigned index ) const;

    int length() const;
    int size() const;

    bool hasKey( const std::string &key ) const;

    Class JSONType() const;

    /// Functions for getting primitives from the JSON object.
    bool IsNull() const;

    std::string toString() const;
    std::string toString( bool &ok ) const;

    double toFloat() const;
    double toFloat( bool &ok ) const;

    long toInt() const;
    long toInt( bool &ok ) const;

    bool toBool() const;
    bool toBool( bool &ok ) const;

    JSONWrapper<std::map<std::string,JSON>> ObjectRange();
    JSONWrapper<std::deque<JSON>> ArrayRange();
    JSONConstWrapper<std::map<std::string,JSON>> ObjectRange() const;
    JSONConstWrapper<std::deque<JSON>> ArrayRange() const;

    std::string dump( int depth = 1, const std::string &tab = std::string(" ")) const;

    friend std::ostream& operator<<( std::ostream&, const JSON & );

private:
    void SetType( Class type );

private:
    /* beware: only call if YOU know that Internal is allocated. No checks performed here.
         This function should be called in a constructed JSON just before you are going to
        overwrite Internal...
      */
    void ClearInternal();

private:

    Class Type = Class::Null;
};

JSON Array();

template <typename... T>
JSON Array( T... args ) {
    JSON arr = JSON::Make( JSON::Class::Array );
    arr.append( args... );
    return std::move( arr );
}

JSON Object();

std::ostream& operator<<( std::ostream &os, const JSON &json );


#endif // JSON_H
