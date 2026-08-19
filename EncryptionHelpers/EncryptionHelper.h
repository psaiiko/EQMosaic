#pragma once

#include <windows.h>
#include <string>
#include <vector>

bool ProtectString(const std::string& text, std::string& result);
bool UnprotectString(const std::string& base64, std::string& result);