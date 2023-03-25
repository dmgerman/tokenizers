/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <stack>
#include <string>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <sstream>

XERCES_CPP_NAMESPACE_USE

class srcml2tokenHandlers : public DefaultHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    srcml2tokenHandlers();
    ~srcml2tokenHandlers();


    //  Handlers for the SAX ContentHandler interface
    // -----------------------------------------------------------------------
    void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);
    void characters(const XMLCh* const chars, const XMLSize_t length);
    void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length);
    void startDocument();
    void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);


    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
    void resetErrors();
    std::string getPosition();
    std::string newGetPosition();
    void setPosition(const std::string newPos);
    void setPosition(const Attributes& attrs);
    std::string position(const Attributes& attrs);
    void advance(std::string st);
    std::string tokens();

private:
    bool            fSawErrors;
    int depth;   // how many tags are we inside of
    int col {1}; // keeps track of current column and row
    int row {1};
    int all_size {0};
    std::string pos; 
    std::string currentContent;
    std::string currentContentOriginal;
    std::string currentPos;
    std::stack<std::string> mystack;
    std::stack<int> toOutputStack;
    std::stack<std::string> fStack;
    std::ostringstream outputSt;
};
