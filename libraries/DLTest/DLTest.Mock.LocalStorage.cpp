/*
 * DLTest.Mock.LocalStorage.cpp
 * 
 * Mock object for filesystem access
 *
 * Author: James Fowkes
 *
 * www.re-innovation.co.uk
 */

/*
 * C++ Library Includes
 */

#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>

#include "DLLocalStorage.h"
#include "DLTest.Mock.LocalStorage.h"

#include "DLUtility.Strings.h"

static std::fstream s_file;

LocalStorageInterface * LocalStorage_GetLocalStorageInterface(LOCAL_STORAGE_TYPE storage_type)
{
    (void)storage_type;
    return new TestStorageInterface();
}

TestStorageInterface::TestStorageInterface()
{
    m_echo = false;
}

bool TestStorageInterface::fileExists(char const * const filePath)
{
    struct stat info;

    if (!filePath) { return false; }

    return stat(filePath, &info) == 0;
}

bool TestStorageInterface::directoryExists(char const * const dirPath)
{
    struct stat info;

    if(stat( dirPath, &info ) != 0)
    {
        return false;
    }
    else if(info.st_mode & S_IFDIR)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool TestStorageInterface::mkDir(char const * const dirPath)
{
    if(!dirPath) { return false; }
    return mkdir(dirPath, S_IRWXU | S_IRWXG) == 0;
}

FILE_HANDLE TestStorageInterface::openFile(char const * const filename, bool forWrite)
{

    if (!filename) { return INVALID_HANDLE; }

    s_file.open(filename, forWrite ? std::ios::app : std::ios::in);

    return 0;
}

void TestStorageInterface::write(FILE_HANDLE file, char const * const toWrite)
{
    (void)file;
    if (!s_file.is_open()) { return; }
    if (!toWrite) { return; }

    s_file << toWrite;

    if (m_echo)
    {
        std::cout << toWrite;
    }
}

uint32_t TestStorageInterface::readBytes(FILE_HANDLE file, char * buffer, uint32_t n)
{
    (void)file;
    if (!s_file.is_open()) { return 0; }
    if (!buffer) { return 0; }
    s_file.read(buffer, n);
    return s_file.gcount();
}

uint32_t TestStorageInterface::readLine(FILE_HANDLE file, char * buffer, uint32_t n, bool stripCRLF)
{
    (void)file;

    if (!s_file.is_open()) { return 0; }
    if (!buffer) { return 0; }

    std::string strBuffer;
    std::getline(s_file, strBuffer);

    strncpy_safe(buffer, strBuffer.c_str(), n);

    int32_t count = strBuffer.length();

    if (count && stripCRLF)
    {
        count--; // Skip back to last char read
        while((count >= 0) && (buffer[count] == '\r' || buffer[count] == '\n'))
        {
            buffer[count--] = '\0';
        }
        count+=2; // Undo skip and also include NULL terminator
    }
    else
    {
        buffer[count++] = '\n'; // Append the \n that was removed by getline
        buffer[count++] = '\0';
    }
    return count;
}

void TestStorageInterface::closeFile(FILE_HANDLE file)
{
    (void)file;
    s_file.close();
}

bool TestStorageInterface::endOfFile(FILE_HANDLE file)
{
    (void)file;
    return s_file.eof();
}

void TestStorageInterface::setEcho(bool set)
{
    m_echo = set;
}

void TestStorageInterface::removeFile(char const * const dirPath)
{
    remove(dirPath);
}

