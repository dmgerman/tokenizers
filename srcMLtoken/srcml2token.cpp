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
#include <srcml.h>
#include <string>
#include <iostream>
#include <fstream>
#include <getopt.h>

// ---------------------------------------------------------------------------
//  Local helper methods
// ---------------------------------------------------------------------------
void usage()
{
    std::cerr << "\nUsage:\n"
            "    srcml2token [-p] [--language=c|c++|java]  <source file>\n\n"
            "This program converts a source code file into a simplified tokenized version\n"
         << XERCES_STD_QUALIFIER endl;
    exit(1);
}

/*
convert the string into its srcm equivalent

return a string

in theory we are creating an extra string...

ideally we should return a xercesc::MemBufInputSource  but I was not able do to it

see comment above

*/

std::string source_to_srcml(const std::string &src, const char* language) {
    
    struct srcml_archive* archive = srcml_archive_create();
    struct srcml_unit* unit = srcml_unit_create(archive);
    srcml_unit_set_language(unit, language);
    srcml_unit_parse_memory(unit, src.c_str(), src.size());
    const char *result = srcml_unit_get_srcml(unit);
    std::string retVal(result);
    srcml_unit_free(unit);
    srcml_archive_free(archive);
    return retVal;
}


// read stdin in one go
std::string read_stdin()
{
    std::string src_code((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
    return src_code;
}

// read file in one go
std::string read_file(const std::string& file_name)
{
    std::ifstream input(file_name);
    std::string src_code((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    return src_code;
}

// ---------------------------------------------------------------------------
//  Program entry point
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    bool withPositions = false;
    const char *language = SRCML_LANGUAGE_C;
    int option_index = 0;
    char option;

    static struct option long_options[] = {
        {"language", required_argument, 0, 'l'},
        {"withPositions", no_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "lp", long_options, &option_index)) != -1) {
        switch (option) {
            case 'l':
                if (optarg!=NULL) {
                    if (strcmp(optarg, "c++") == 0) {
                        language = SRCML_LANGUAGE_CXX;
                    } else if (strcmp(optarg, "c") == 0) {
                        language = SRCML_LANGUAGE_C;
                    } else if (strcmp(optarg, "java") == 0) {
                        language = SRCML_LANGUAGE_JAVA;
                    } else {
                        std::cerr << "invalid language: " << optarg << std::endl;
                        usage();
                    }
                } else {
                    std::cerr << "no language specified with option -l" << std::endl;
                    usage();
                }
                break;
            case 'p':
                withPositions = true;
                std::cerr << "withPositions: true" << std::endl;
                break;
            case '?':
                std::cerr << "unknown option: " << optopt << std::endl;
                usage();
                break;
        }
    }


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
            // read from standard input if only one file
            // or from file otherwise
            std::string contents;

            if (optind < argc) {
                contents = read_file(argv[optind]);
            } else {
                contents = read_stdin();
            }

            std::string sml = source_to_srcml(contents, language);
            // we need to convert the string to a 

            xercesc::MemBufInputSource buf((XMLByte*)sml.c_str(), sml.size(),
                                           "sml(in memory)");

            srcml2tokenHandlers handler(withPositions);
            parser->setContentHandler(&handler);
            parser->setErrorHandler(&handler);
            parser->parse(buf);
            std::cout << handler.tokens();
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


