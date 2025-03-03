//
//  INIReader.cpp
//  ModuleTest
//
//  Created by pinky on 2025-03-02.
//

#include "INIReader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>


void Section::addKeyValue( const string& strKey, const string& strValue )
{
    m_mapValues[ strKey ] = strValue;
    
}

int Section::getInt( const string& strKey, int defValue )
{
    auto it = m_mapValues.find( strKey );
    if( it == m_mapValues.end() )
    {
        return defValue;
    }
    else{
        try {
            int num = std::stoi( it->second );
            return num;
        } catch (...) {
            return defValue;
        }
    }
}
string Section::getString( const string& strKey, string defValue )
{
    auto it = m_mapValues.find( strKey );
    if( it == m_mapValues.end() )
    {
        return defValue;
    }
    else{
        return it->second;
    }
    
    
}

string Section::getLogString()
{
    stringstream ss;
    if( m_secName.length() != 0 )
    {
        ss<<"["<<m_secName<<"]\n";
    }
    
    for( auto it : m_mapValues )
    {
        ss<< it.first << "=" << it.second <<"\n";
    }
    
    return ss.str();
}

INIReader::INIReader()
{
    
}



// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

string INIReader::getLogString()
{
    stringstream ss;
    ss<< m_mapSections[""].getLogString();
    for( auto it : m_mapSections )
    {
        if( it.first == "" )
        {
            continue;
        }
        else{
            ss<< it.second.getLogString();
        }
    }
    
    return ss.str();
}

bool INIReader::ParseFile( const std::string& filename )
{
    std::ifstream inFile( filename );
    if( !inFile )
    {
        cerr<<"Open File error!"<<filename<<endl;
        return false;
    }
    
    //add root section
    m_mapSections.insert( make_pair( "", Section("") ) );
    
    string line;
    Section* curSec = nullptr;
    while( std::getline( inFile, line ) )
    {
        trim(line);
        // jump over empty line
        if( line.length() == 0 )
        {
            continue;
        }
        
        //jump over comment
        if( line[0] == '#' )
        {
            continue;
        }
        
        if( line.front() == '[' && line.back() == ']' )
        {
            line.erase( line.begin() );
            line.pop_back();
            
            trim( line );
            
            if( m_mapSections.find( line ) == m_mapSections.end() )
            {
                m_mapSections.insert( make_pair( line, Section(line) ) );
            }
            
            curSec = &m_mapSections[line];
            continue;
        }
        
        size_t pos = line.find( '=' );
        if( pos != string::npos )
        {
            string left = line.substr(0, pos );
            string right = line.substr( pos +1 );
            trim( left);
            trim( right );
            if( curSec )
            {
                curSec->addKeyValue( left,  right );
            }
            else{
                m_mapSections[""].addKeyValue( left,  right);
            }
        }
        else{
            continue;
        }
        
    }
    
    inFile.close();
    return true;
}

int INIReader::getInt( const string& section, const string& strKey, int defValue )
{
    auto sec = getSection( section );
    if( !sec )
    {
        return defValue;
    }
    else{
        return sec->getInt( strKey, defValue );
    }
    
}
string INIReader::getString( const string& section, const string& strKey, string defValue)
{
    auto sec = getSection( section );
    if( !sec )
    {
        return defValue;
    }
    else{
        return sec->getString( strKey, defValue );
    }
    
}

Section* INIReader::getSection( const string& strName )
{
    auto it = m_mapSections.find( strName );
    if( it == m_mapSections.end() )
    {
        return nullptr;
    }
    else{
        return &it->second;
    }
}
