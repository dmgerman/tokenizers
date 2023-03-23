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
#include <xercesc/framework/StdInInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include "srcml2token.hpp"
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

// ---------------------------------------------------------------------------
//  Local helper methods
// ---------------------------------------------------------------------------
void usage()
{
    XERCES_STD_QUALIFIER cout << "\nUsage:\n"
            "    srcml2token  <XML file>\n\n"
            "This program converts the output of srcML into a simplified tokenized version\n"
         << XERCES_STD_QUALIFIER endl;
}


// ---------------------------------------------------------------------------
//  Program entry point
// ---------------------------------------------------------------------------
int main(int argC, char* argV[])
{
    // Initialize the XML4C system
    try
    {
         XMLPlatformUtils::Initialize();
    }

    catch (const XMLException& toCatch)
    {
         XERCES_STD_QUALIFIER cerr << "Error during initialization! Message:\n"
              << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
         return 1;
    }


    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();

    srcml2tokenHandlers handler;
    parser->setContentHandler(&handler);
    parser->setErrorHandler(&handler);

    int errorCount = 0;
    // create a faux scope so that 'src' destructor is called before
    // XMLPlatformUtils::Terminate
    {
        //
        //  Kick off the parse and catch any exceptions. Create a standard
        //  input input source and tell the parser to parse from that.
        //
        try
        {
            std::string rip = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n                <unit xmlns=\"http://www.srcML.org/srcML/src\" revision=\"1.0.0\" language=\"C\" filename=\"rip.c\"><function><type><name>int</name></type> <name>main</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type=\"number\">0</literal></expr>;</return></block_content>}</block></function>"
                ;
            std::string rip2 = rip + "\n</unit>";

            if (argC < 2) {
                xercesc::MemBufInputSource buf((XMLByte*)rip.c_str(), rip.size(),
                                               "rip (in memory)");
                parser->parse(buf);
                std::cerr << "Hello world\n";
                xercesc::MemBufInputSource buf2((XMLByte*)rip2.c_str(), rip2.size(),
                                               "rip2 (in memory)");
                parser->parse(buf2);
            } else {
                parser->parse(argV[1]);
            }
        }
        catch (const OutOfMemoryException&)
        {
            XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
            errorCount = 2;
            return 4;
        }
        catch (const XMLException& e)
        {
            XERCES_STD_QUALIFIER cerr << "\nError during parsing: \n"
                 << StrX(e.getMessage())
                 << "\n" << XERCES_STD_QUALIFIER endl;
            errorCount = 1;
            return 4;
        }

    }

    //
    //  Delete the parser itself.  Must be done prior to calling Terminate, below.
    //
    delete parser;

    XMLPlatformUtils::Terminate();

    if (errorCount > 0)
        return 4;
    else
        return 0;
}


