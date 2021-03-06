/*
 * DLHTTP.Test.cpp
 * 
 * Tests the HTTP Request and Response class
 *
 * Author: James Fowkes
 *
 * www.re-innovation.co.uk
 */

/*
 * C++ Library Includes
 */

#include <string.h>
#include <stdint.h>

/*
 * Local Application Includes
 */

#include "DLUtility.Strings.h"
#include "../DLHTTP.h"

/*
 * Unity Test Framework
 */

#include "unity.h"

char requestBuffer[512];
RequestBuilder builder;
        
void test_header_isSuccessfulyParsed(void)
{
    Header header;
    
    header.setFromLine("Content-Length: 48");
    
    TEST_ASSERT_TRUE(header.matchName("content-length"));
    TEST_ASSERT_EQUAL_STRING("content-length", header.getName());
    TEST_ASSERT_EQUAL_STRING("48", header.getValue());
}

void test_requestbuilder_BuildsWithMethodAndURLOnly(void)
{
    builder.setMethodAndURL("GET", "/");   
    builder.writeToBuffer(requestBuffer, 512);
    TEST_ASSERT_EQUAL_STRING("GET / HTTP/1.1\r\n", requestBuffer);
}

void test_requestbuilder_BuildsWithHeaders(void)
{
    builder.putHeader("Host", "www.example.com");
    builder.putHeader("Content-Type", "text/html");
    builder.putHeader("Some-Other-Header", "Some-Other-Value");
    builder.writeToBuffer(requestBuffer, 512);
    TEST_ASSERT_EQUAL_STRING(
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: text/html\r\n"
        "Some-Other-Header: Some-Other-Value\r\n", requestBuffer);
}

void test_requestbuilder_BuildsWithBodyContent(void)
{
    builder.putBody("This is some data in the body.");
    builder.writeToBuffer(requestBuffer, 512);
    TEST_ASSERT_EQUAL_STRING(
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: text/html\r\n"
        "Some-Other-Header: Some-Other-Value\r\n"
        "\r\n"
        "This is some data in the body.\r\n", requestBuffer);
}

void test_requestbuilder_BuildsWithContentLengthHeader(void)
{
    builder.writeToBuffer(requestBuffer, 512, true);
    TEST_ASSERT_EQUAL_STRING(
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: text/html\r\n"
        "Some-Other-Header: Some-Other-Value\r\n"
        "Content-Length: 30\r\n"
        "\r\n"
        "This is some data in the body.\r\n", requestBuffer);
}

void test_requestbuilder_BuildsWithURLParameters(void)
{
    builder.reset();
    builder.setMethodAndURL("GET", "/update"); 
    builder.setURLParam("Param1", "Param1Value");
    builder.setURLParam("Param2", "Param2Value");
    builder.putHeader("Host", "www.example.com");
    builder.putBody(NULL);

    builder.writeToBuffer(requestBuffer, 512);

    TEST_ASSERT_EQUAL_STRING(
        "GET /update?Param1=Param1Value&Param2=Param2Value HTTP/1.1\r\n"
        "Host: www.example.com\r\n", requestBuffer); 
}

void test_responseparser_ReadsHTTPStatusLine(void)
{
    char response[] = "HTTP/1.0 200 OK\r\n";

    ResponseParser responseParser(response);

    TEST_ASSERT_EQUAL(10, responseParser.getVersion() );
    TEST_ASSERT_EQUAL(200, responseParser.getStatus() );
    TEST_ASSERT_EQUAL_STRING("OK", responseParser.getReason() );
}

void test_responseparser_ReadsHTTPHeaders(void)
{
    char response[] = 
        "HTTP/1.0 200 OK\r\n"
        "Content-Length:100\r\n"
        "Some-Other-Header: Some-Other-Value\r\n";

    ResponseParser responseParser(response);

    char headerNames[][20] = {"Content-Length", "Some-Other-Header"};
    char headerValues[][20] = {"100", "Some-Other-Value"};

    TEST_ASSERT_EQUAL(2, responseParser.headerCount());
    TEST_ASSERT_NOT_NULL(responseParser.findHeaderInList(headerNames[0]));
    TEST_ASSERT_NOT_NULL(responseParser.findHeaderInList(headerNames[1]));

    TEST_ASSERT_EQUAL_PTR(responseParser.getHeaderByIndex(0), responseParser.findHeaderInList(headerNames[0]));
    TEST_ASSERT_EQUAL_PTR(responseParser.getHeaderByIndex(1), responseParser.findHeaderInList(headerNames[1]));
    
    TEST_ASSERT_EQUAL_STRING(headerValues[0], responseParser.getHeaderValue(headerNames[0]));
    TEST_ASSERT_EQUAL_STRING(headerValues[1], responseParser.getHeaderValue(headerNames[1]));
}

int main(void)
{
    UnityBegin("DLHTTP.cpp");

    RUN_TEST(test_header_isSuccessfulyParsed);

    RUN_TEST(test_requestbuilder_BuildsWithMethodAndURLOnly);
    RUN_TEST(test_requestbuilder_BuildsWithHeaders);
    RUN_TEST(test_requestbuilder_BuildsWithBodyContent);
    RUN_TEST(test_requestbuilder_BuildsWithContentLengthHeader);
    RUN_TEST(test_requestbuilder_BuildsWithURLParameters);

    RUN_TEST(test_responseparser_ReadsHTTPStatusLine);
    RUN_TEST(test_responseparser_ReadsHTTPHeaders);
    
    return 0;
}
