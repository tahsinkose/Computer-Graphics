#ifndef __ppm_h__
#define __ppm_h__
#include <vector>
#include "parser.h"


void write_ppm(const char* filename, std::vector<std::vector<std::vector<unsigned char> > >& data, int width, int height);

#endif // __ppm_h__
