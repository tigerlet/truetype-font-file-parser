#pragma once

#include <cstdio>
#include <iostream>
#include <fstream>
#include "type.h"

namespace ttf_dll {

#define BIT(b)  (1 << (b))

void IFStreamReadBigEndian(std::ifstream &fin, void *buf, unsigned unit_size, unsigned unit_num = 1);
#define FREAD(f, b) IFStreamReadBigEndian((f), (b), sizeof(*(b)))
#define FREAD_N(f, b, n) IFStreamReadBigEndian((f), (b), sizeof(*(b)), (n))

}
