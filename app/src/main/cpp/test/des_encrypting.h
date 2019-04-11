#pragma once

#include <iostream>
#include <fstream>
#include <bitset>
#include <string>


void InitFromKey(const std::string& k);

std::bitset<64> charToBitset(const char s[8]);

std::bitset<64> encrypt(std::bitset<64>& plain);

std::bitset<64> decrypt(std::bitset<64>& cipher);