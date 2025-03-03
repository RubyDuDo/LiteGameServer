//
//  INIReader.hpp
//  ModuleTest
//
//  Created by pinky on 2025-03-02.
//

#ifndef INIReader_hpp
#define INIReader_hpp

#include <stdio.h>

#include <map>
#include <string>
#include <vector>

using namespace std;

class Section
{
public:
    Section():m_secName(""), m_mapValues(){};
    Section( const std::string& strName) : m_secName( strName), m_mapValues(){};
    void addKeyValue( const string& strKey, const string& strValue );
public:
    int getInt( const string& strKey, int defValue = 0 );
    string getString( const string& strKey, string defValue = "" );
    
    string getLogString();
    string m_secName;
    map<string, string> m_mapValues;
    
};

class INIReader
{
public:
    INIReader();
    
    bool ParseFile( const std::string& filename );
    
    Section* getSection( const string& strName );
    
    int getInt( const string& section, const string& strKey, int defValue = 0 );
    string getString( const string& section, const string& strKey, string defValue ="");
    
    
    string getLogString();
    
private:
    map<string, Section> m_mapSections;
    

};

#endif /* INIReader_hpp */
