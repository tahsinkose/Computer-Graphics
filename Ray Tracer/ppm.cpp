#include "ppm.h"
#include <stdexcept>


void write_ppm(const char* filename, std::vector<std::vector<std::vector<unsigned char> > >& data, int width, int height)
{
    FILE *outfile;

    if ((outfile = fopen(filename, "w")) == NULL) 
    {
        throw std::runtime_error("Error: The ppm file cannot be opened for writing.");
    }

    (void) fprintf(outfile, "P3\n%d %d\n255\n", width, height);

    unsigned char color;
    for (size_t j = 0; j < height; ++j)
    {
        for (size_t i = 0; i < width; ++i)
        {
            for (size_t c = 0; c < 3; ++c)
            {
                color = data[j][i][c];
                if (i == width - 1 && c == 2)
                {
                    (void) fprintf(outfile, "%d", color);
                }
                else
                {
                    (void) fprintf(outfile, "%d ", color);
                }
            }
        }

        (void) fprintf(outfile, "\n");
    }

    (void) fclose(outfile);
}
