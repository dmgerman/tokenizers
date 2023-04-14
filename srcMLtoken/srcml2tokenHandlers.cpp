// it has been a long time since I wrote this code that I can't remember
// what I did

// at the time, I didn't even know C++ the way I do now, so this code is probably
// ugly pre modern C++ code. On top of that Xerces is really a C library, used
// from C++
//
// but I think the parsing is still easier with C++ than C. Just a hunch,

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <string.h>
#include <iostream>
#include <stack>
#include <string>
#include <stdio.h>
#include <algorithm>

#include "srcml2token.hpp"
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include <map>
#include <vector>

std::vector<int> lineMarkers {0};
std::vector<int> lineNumbers {1};


std::string get_attribute_value(const Attributes& attrs, std::string name) {
    XMLCh* revName= XMLString::transcode(name.c_str());
    char* st = XMLString::transcode(attrs.getValue(revName));

    // TODO: ugly remove the need for a variable
    std::string result = "";
    if (st != NULL)
        result = st;
    return result;
}

// remove whitespace from both sides of the string
std::string mytrim(const std::string& str,
                   const std::string& whitespace = " \t\n")
{
    std::size_t strBegin = str.find_first_not_of(whitespace);

    if (strBegin == std::string::npos)
        return ""; // no content

    int  strEnd = str.find_last_not_of(whitespace);
    int  strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

// remove whitespace from front of string
std::string mytrimBegin(const std::string& str,
                 const std::string& whitespace = " \t\n")
{
    std::size_t strBegin = str.find_first_not_of(whitespace);

    if (strBegin == std::string::npos)
        return ""; // no content


    return str.substr(strBegin);
}

std::pair<int,int> findRow(int position) {
    //it is most likely we will use the last ones
    // rather than any so search from the end
    assert(lineMarkers.size() > 0);
    unsigned int i = lineMarkers.size()-1;
    while (i >= 0 && lineMarkers[i]> position) {
        i--;
    }
    assert(i >=0 && i < lineMarkers.size());
    
    return {lineMarkers[i], lineNumbers[i]} ;
}


std::string srcml2tokenHandlers::newGetPosition() {

    auto st = mytrimBegin(currentContentOriginal);

    int beginning = all_size - st.size();

    auto prevRow = findRow(beginning);

    auto col = beginning + 1 -  prevRow.first;
    return std::to_string(prevRow.second) + ":" + std::to_string(col);
}

void srcml2tokenHandlers::advance(std::string st) {
    int i=1;

    for(auto c:st) {
        if (c == '\n') {
            row++;
            auto offset = all_size + i;
            lineMarkers.push_back(offset);
            lineNumbers.push_back(row);
            col = 1;
        } else {
            col++;
        }
        i++;
    }
    all_size += st.size();
}



// ---------------------------------------------------------------------------
//  srcml2tokenHandlers: Constructors and Destructor
// ---------------------------------------------------------------------------
srcml2tokenHandlers::srcml2tokenHandlers(bool withPos) :
  depth(0)
{
    withPositions = withPos;
}

srcml2tokenHandlers::~srcml2tokenHandlers()
{
}



// ---------------------------------------------------------------------------
//  srcml2tokenHandlers: Implementation of the SAX DocumentHandler interface
// ---------------------------------------------------------------------------
void srcml2tokenHandlers::startElement(const XMLCh* const //uri
                                     , const XMLCh* const localname
                                     , const XMLCh* const qname
                                       , const Attributes& attrs)
{
    std::string tagLocal = XMLString::transcode(localname);
//    char *tagName = XMLString::transcode(qname);
//    std::string savePos = position(attrs);
    std::string savePos = newGetPosition();
    if (savePos != "")  {
        currentPos = savePos;
    }    

    if (depth <= 1)  {

        //outputSt << "-" << "\t" << tagName << " " << depth << std::endl;

        if (tagLocal == "unit") {

//<unit xmlns="http://www.srcML.org/srcML/src" xmlns:cpp="http://www.srcML.org/srcML/cpp" xmlns:pos="http://www.srcML.org/srcML/position" revision="0.9.5" language="C" filename="test-exec/count.cpp" pos:tabs="8"><cpp:include pos:line="1" pos:column="1">#<cpp:directive pos:line="1" pos:column="2">include<pos:position pos:line="1" pos:column="9"/></cpp:directive> <cpp:file pos:line="1" pos:column="10">&lt;iostream&gt;<pos:position pos:line="1" pos:column="23"/></cpp:file></cpp:include>

            auto revision = get_attribute_value(attrs, "revision");
            auto language = get_attribute_value(attrs, "language");
            if (revision != "" && language != "") {
                if (withPositions)
                    outputSt << "-:-" << "\t";
                
                outputSt << "begin_unit|" <<
                    "revision:" << revision << ";" <<
                    "language:" << language << ";" <<
                    "cregit-version:" << CREGIT_VERSION <<
                    std::endl;
            } else {
                if (withPositions)
                    outputSt << "-:-" << "\t" ;
                    
                outputSt << "begin_" << tagLocal << std::endl;
            }
        } else {
            if (withPositions)
                outputSt << "-:-" << "\t" ;
            
            outputSt << "begin_" << tagLocal << std::endl;
        }
    }

    if (currentContent.length() > 0 ) {
        // we output the content of the previous tag here...
        if (withPositions) {
            outputSt << newGetPosition() << "\t" ;
        }
        outputSt << currentContent << std::endl;
        currentContent = "";
        currentContentOriginal = "";
    }
    
    
    mystack.push(tagLocal);
    toOutputStack.push(0);
    depth++;

//    XMLString::release(line);
    //XMLString::release(col);

}

void srcml2tokenHandlers::endElement (const XMLCh *const /*uri*/,
                                    const XMLCh *const localname,
                                    const XMLCh *const /*qname*/)
{
    char *tagName = XMLString::transcode(localname);

    // No escapes are legal here
    if (currentContent.length() > 0 ) {
        if (withPositions)
            outputSt << newGetPosition() << "\t" ;
        outputSt << currentContent << std::endl;

        currentContent = "";
        currentContentOriginal = "";
    } 

    std::string parent = mystack.top();


    mystack.pop();
    toOutputStack.pop();
    depth--;
    if (depth <= 1)  {
        if (withPositions)
            outputSt << "-:-" << "\t" ;
        outputSt << "end_" << tagName << std::endl;
    }


}


void srcml2tokenHandlers::characters(  const   XMLCh* const    chars 
								    , const XMLSize_t length)
{
    std::string original = XMLString::transcode(chars);
    std::string st = mytrim(original);
    std::string node = mystack.top();

//    outputSt << "ORIGInAL [" << original <<"]" << std::endl;
    advance(original);
    currentContentOriginal+= original;
    
    if (st.length() > 0) {
        std::replace( st.begin(), st.end(), '\n', ' ');

        if (currentContent.length() == 0) {
            currentContent = mystack.top() + "|";
        }
        currentContent = currentContent + st;
    }
}        
    

void srcml2tokenHandlers::ignorableWhitespace( const   XMLCh* const /* chars */
										    , const XMLSize_t length)
{

}

void srcml2tokenHandlers::startDocument()
{
    outputSt.str(""); // initialize the output
}


// ---------------------------------------------------------------------------
//  srcml2tokenHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void srcml2tokenHandlers::error(const SAXParseException& e)
{
    XERCES_STD_QUALIFIER cerr << "\nError at file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void srcml2tokenHandlers::fatalError(const SAXParseException& e)
{
    XERCES_STD_QUALIFIER cerr << "\nFatal Error at file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
                 << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void srcml2tokenHandlers::warning(const SAXParseException& e)
{
    XERCES_STD_QUALIFIER cerr << "\nWarning at file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void srcml2tokenHandlers::resetErrors()
{

}

std::string srcml2tokenHandlers::tokens()
{
    return outputSt.str();
}
