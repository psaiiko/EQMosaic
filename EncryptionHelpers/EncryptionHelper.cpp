// EncryptionHelpers.cpp : Defines the functions for the static library.
//

#include "pch.h"

// -------------------------------------------------------------------------------------------------
// Includes
// -------------------------------------------------------------------------------------------------

#include "EncryptionHelper.h"

#include <windows.h>
#include <wincrypt.h>
#include <string>
#include <vector>

#pragma comment(lib, "Crypt32.lib")

bool ProtectString(const std::string& text, std::string& result)
{
    DATA_BLOB input{};
    input.pbData = (BYTE*)text.data();
    input.cbData = (DWORD)text.size();

    DATA_BLOB output{};

    if (!CryptProtectData(
        &input,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        0,
        &output))
    {
        return false;
    }

    // Get required Base64 buffer size
    DWORD base64Length = 0;

    if (!CryptBinaryToStringA(
        output.pbData,
        output.cbData,
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
        nullptr,
        &base64Length))
    {
        LocalFree(output.pbData);
        return false;
    }

    // Allocate string buffer
    result.resize(base64Length);

    if (!CryptBinaryToStringA(
        output.pbData,
        output.cbData,
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
        result.data(),
        &base64Length))
    {
        LocalFree(output.pbData);
        return false;
    }

    // Remove trailing null terminator added by CryptBinaryToStringA
    if (!result.empty() && result.back() == '\0')
        result.pop_back();

    LocalFree(output.pbData);

    return true;
}

bool UnprotectString(const std::string& base64, std::string& result)
{
    DWORD binarySize = 0;

    if (!CryptStringToBinaryA(
        base64.c_str(),
        0,
        CRYPT_STRING_BASE64,
        nullptr,
        &binarySize,
        nullptr,
        nullptr))
    {
        return false;
    }

    std::vector<BYTE> binary(binarySize);

    if (!CryptStringToBinaryA(
        base64.c_str(),
        0,
        CRYPT_STRING_BASE64,
        binary.data(),
        &binarySize,
        nullptr,
        nullptr))
    {
        return false;
    }

    DATA_BLOB input{};
    input.pbData = binary.data();
    input.cbData = binarySize;

    DATA_BLOB output{};

    if (!CryptUnprotectData(
        &input,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        0,
        &output))
    {
        return false;
    }

    result.assign((char*)output.pbData, output.cbData);

    LocalFree(output.pbData);

    return true;
}