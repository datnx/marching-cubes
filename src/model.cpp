#include <fstream>
#include <iostream>
#include "model.h"
#include <vector>
#include <map>
#include <glm/glm.hpp>

std::vector<float> vertices;
std::map<int, std::map<int, std::map<int, std::map<char, unsigned int>>>> vertex_map;
std::vector<unsigned int> indices;
std::vector<float> colors;
std::vector<float> normals;
std::vector<int> normal_counts;
glm::mat3 dual_matrix(glm::vec3 a);
glm::vec3 cross(glm::vec3 a, glm::vec3 b);
glm::vec3 normalize(glm::vec3 a);

int get_vertex_index(int i, int j, int k, char direction)
{
    if (vertex_map.count(i))
    {
        if (vertex_map[i].count(j))
        {
            if (vertex_map[i][j].count(k))
            {
                if (vertex_map[i][j][k].count(direction))
                {
                    return vertex_map[i][j][k][direction];
                }
            }
        }
    }
    return -1;
}

unsigned char find_index(unsigned short* data, short* dim, int i, int j, int k)
{
    unsigned char index = 0;
    if (data[(i - 1) * dim[2] * dim[1] + (j - 1) * dim[1] + k - 1] == 1) index |= 1;
    if (data[(i - 1) * dim[2] * dim[1] + (j - 1) * dim[1] + k] == 1) index |= 2;
    if (data[(i - 1) * dim[2] * dim[1] + j * dim[1] + k] == 1) index |= 4;
    if (data[(i - 1) * dim[2] * dim[1] + j * dim[1] + k - 1] == 1) index |= 8;
    if (data[i * dim[2] * dim[1] + (j - 1) * dim[1] + k - 1] == 1) index |= 16;
    if (data[i * dim[2] * dim[1] + (j - 1) * dim[1] + k] == 1) index |= 32;
    if (data[i * dim[2] * dim[1] + j * dim[1] + k] == 1) index |= 64;
    if (data[i * dim[2] * dim[1] + j * dim[1] + k - 1] == 1) index |= 128;
    return index;
}

void push_vertices(int i, int j, int k, char direction, float* pixdim)
{
    switch (direction)
    {
    case 'i':
        vertices.push_back(j * pixdim[2]);
        vertices.push_back(k * pixdim[1]);
        vertices.push_back((i - 0.5) * pixdim[3]);
        break;
    
    case 'j':
        vertices.push_back((j - 0.5) * pixdim[2]);
        vertices.push_back(k * pixdim[1]);
        vertices.push_back(i * pixdim[3]);
        break;

    case 'k':
        vertices.push_back(j * pixdim[2]);
        vertices.push_back((k - 0.5) * pixdim[1]);
        vertices.push_back(i * pixdim[3]);
        break;

    default:
        std::cout << "invalid direction" << std::endl;
        break;
    }
    normals.push_back(0);
    normals.push_back(0);
    normals.push_back(0);
    normal_counts.push_back(0);
}

int load_model(std::string file_path)
{
    int edgeTable[256] = {
    0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
    0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
    0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
    0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
    0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
    0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
    0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
    0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
    0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
    0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
    0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
    0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
    0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
    0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
    0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
    0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
    0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
    0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
    0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
    0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
    0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   };

    int triTable[256][16] =
    {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
    {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
    {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
    {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
    {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
    {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
    {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
    {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
    {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
    {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
    {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
    {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
    {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
    {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
    {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
    {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
    {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
    {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
    {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
    {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
    {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
    {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
    {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
    {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
    {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
    {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
    {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
    {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
    {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
    {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
    {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
    {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
    {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
    {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
    {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
    {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
    {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
    {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
    {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
    {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
    {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
    {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
    {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
    {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
    {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
    {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
    {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
    {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
    {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
    {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
    {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
    {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
    {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
    {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
    {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
    {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
    {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
    {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
    {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
    {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
    {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
    {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
    {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
    {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
    {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
    {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
    {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
    {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
    {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
    {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
    {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
    {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
    {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
    {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
    {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
    {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
    {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
    {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
    {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
    {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
    {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
    {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
    {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
    {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
    {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
    {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
    {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
    {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
    {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
    {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
    {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
    {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
    {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
    {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
    {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
    {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
    {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

    std::ifstream infile(file_path, std::ios::binary);

    if (infile.fail())
    {
        std::cout << "Fail to read file" << std::endl;
        return -1;
    }
    
    size_t bytes_read = 0;

    std::cout << "Reading header" << std::endl;

    int sizeof_header;
    infile.read(reinterpret_cast<char*>(&sizeof_header), sizeof(sizeof_header));
    bytes_read += infile.gcount();

    if (sizeof_header != 348)
    {
        std::cout << "Invalid header size: should be 348 bytes" << std::endl;
        return -1;
    }

    char data_type[10];
    infile.read(reinterpret_cast<char*>(&data_type), sizeof(data_type));
    bytes_read += infile.gcount();

    char db_name[18];
    infile.read(reinterpret_cast<char*>(&db_name), sizeof(db_name));
    bytes_read += infile.gcount();

    int extents;
    infile.read(reinterpret_cast<char*>(&extents), sizeof(extents));
    bytes_read += infile.gcount();

    short session_error;
    infile.read(reinterpret_cast<char*>(&session_error), sizeof(session_error));
    bytes_read += infile.gcount();

    char regular;
    infile.read(reinterpret_cast<char*>(&regular), sizeof(regular));
    bytes_read += infile.gcount();

    char dim_info;
    infile.read(reinterpret_cast<char*>(&dim_info), sizeof(dim_info));
    bytes_read += infile.gcount();

    short dim[8];
    infile.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    bytes_read += infile.gcount();

    std::cout << dim[0] << " dimensions" << std::endl;
    unsigned long num_voxels = 1;
    for (int i = 0; i < dim[0]; i++)
    {
        std::cout << "Dim " << i + 1 << ": " << dim[i + 1] << std::endl;
        num_voxels *= dim[i + 1];
    }

    float intent_p1;
    infile.read(reinterpret_cast<char*>(&intent_p1), sizeof(intent_p1));
    bytes_read += infile.gcount();

    float intent_p2;
    infile.read(reinterpret_cast<char*>(&intent_p2), sizeof(intent_p2));
    bytes_read += infile.gcount();

    float intent_p3;
    infile.read(reinterpret_cast<char*>(&intent_p3), sizeof(intent_p3));
    bytes_read += infile.gcount();

    short intent_code;
    infile.read(reinterpret_cast<char*>(&intent_code), sizeof(intent_code));
    bytes_read += infile.gcount();

    short datatype;
    infile.read(reinterpret_cast<char*>(&datatype), sizeof(datatype));
    bytes_read += infile.gcount();

    short bitpix;
    infile.read(reinterpret_cast<char*>(&bitpix), sizeof(bitpix));
    bytes_read += infile.gcount();

    short slice_start;
    infile.read(reinterpret_cast<char*>(&slice_start), sizeof(slice_start));
    bytes_read += infile.gcount();

    float pixdim[8];
    infile.read(reinterpret_cast<char*>(&pixdim), sizeof(pixdim));
    bytes_read += infile.gcount();

    float vox_offset;
    infile.read(reinterpret_cast<char*>(&vox_offset), sizeof(vox_offset));
    bytes_read += infile.gcount();

    float scl_slope;
    infile.read(reinterpret_cast<char*>(&scl_slope), sizeof(scl_slope));
    bytes_read += infile.gcount();

    float scl_inter;
    infile.read(reinterpret_cast<char*>(&scl_inter), sizeof(scl_inter));
    bytes_read += infile.gcount();

    short slice_end;
    infile.read(reinterpret_cast<char*>(&slice_end), sizeof(slice_end));
    bytes_read += infile.gcount();

    char slice_code;
    infile.read(reinterpret_cast<char*>(&slice_code), sizeof(slice_code));
    bytes_read += infile.gcount();

    char xyzt_units;
    infile.read(reinterpret_cast<char*>(&xyzt_units), sizeof(xyzt_units));
    bytes_read += infile.gcount();

    float cal_max;
    infile.read(reinterpret_cast<char*>(&cal_max), sizeof(cal_max));
    bytes_read += infile.gcount();

    float cal_min;
    infile.read(reinterpret_cast<char*>(&cal_min), sizeof(cal_min));
    bytes_read += infile.gcount();

    float slice_duration;
    infile.read(reinterpret_cast<char*>(&slice_duration), sizeof(slice_duration));
    bytes_read += infile.gcount();

    float toffset;
    infile.read(reinterpret_cast<char*>(&toffset), sizeof(toffset));
    bytes_read += infile.gcount();

    int glmax;
    infile.read(reinterpret_cast<char*>(&glmax), sizeof(glmax));
    bytes_read += infile.gcount();

    int glmin;
    infile.read(reinterpret_cast<char*>(&glmin), sizeof(glmin));
    bytes_read += infile.gcount();

    char descrip[80];
    infile.read(reinterpret_cast<char*>(&descrip), sizeof(descrip));
    bytes_read += infile.gcount();

    char aux_file[24];
    infile.read(reinterpret_cast<char*>(&aux_file), sizeof(aux_file));
    bytes_read += infile.gcount();

    short qform_code;
    infile.read(reinterpret_cast<char*>(&qform_code), sizeof(qform_code));
    bytes_read += infile.gcount();

    short sform_code;
    infile.read(reinterpret_cast<char*>(&sform_code), sizeof(sform_code));
    bytes_read += infile.gcount();

    float quatern_b;
    infile.read(reinterpret_cast<char*>(&quatern_b), sizeof(quatern_b));
    bytes_read += infile.gcount();

    float quatern_c;
    infile.read(reinterpret_cast<char*>(&quatern_c), sizeof(quatern_c));
    bytes_read += infile.gcount();

    float quatern_d;
    infile.read(reinterpret_cast<char*>(&quatern_d), sizeof(quatern_d));
    bytes_read += infile.gcount();

    float qoffset_x;
    infile.read(reinterpret_cast<char*>(&qoffset_x), sizeof(qoffset_x));
    bytes_read += infile.gcount();

    float qoffset_y;
    infile.read(reinterpret_cast<char*>(&qoffset_y), sizeof(qoffset_y));
    bytes_read += infile.gcount();

    float qoffset_z;
    infile.read(reinterpret_cast<char*>(&qoffset_z), sizeof(qoffset_z));
    bytes_read += infile.gcount();

    float srow_x[4];
    infile.read(reinterpret_cast<char*>(&srow_x), sizeof(srow_x));
    bytes_read += infile.gcount();

    float srow_y[4];
    infile.read(reinterpret_cast<char*>(&srow_y), sizeof(srow_y));
    bytes_read += infile.gcount();

    float srow_z[4];
    infile.read(reinterpret_cast<char*>(&srow_z), sizeof(srow_z));
    bytes_read += infile.gcount();

    char intent_name[16];
    infile.read(reinterpret_cast<char*>(&intent_name), sizeof(intent_name));
    bytes_read += infile.gcount();

    char magic[4];
    infile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    bytes_read += infile.gcount();

    std::cout << "Read " << bytes_read << " bytes" << std::endl;

    char extension[4];
    infile.read(reinterpret_cast<char*>(&extension), sizeof(extension));

    if (bytes_read != 348)
    {
        std::cout << "Error reading header" << std::endl;
        return -1;
    }

    std::cout << "Reading " << num_voxels << " voxels" << std::endl;

    std::cout << "Data type: unsigned short" << std::endl;
    unsigned short* data = new unsigned short[num_voxels];
    infile.read(reinterpret_cast<char*>(data), sizeof(unsigned short) * num_voxels);
    bytes_read = infile.gcount();
    if (bytes_read != sizeof(unsigned short) * num_voxels)
    {
        std::cout << "Error reading data" << std::endl;
        std::cout << "Only read " << bytes_read << "/" << sizeof(unsigned short) * num_voxels << std::endl;
        return -1;
    }
    std::cout << "Reading done" << std::endl;

    // Marching cubes
    unsigned char cube_index;
    unsigned int count = 0;
    unsigned int vertex_index;
    unsigned int vertlist[12];
    unsigned int triangle[3];
    glm::vec3 normal, a, b, c;
    for (int i = 1; i < dim[3]; i++)
    {
        for (int j = 1; j < dim[2]; j++)
        {
            for (int k = 1; k < dim[1]; k++)
            {
                cube_index = find_index(data, dim, i, j, k);
                if (edgeTable[cube_index] != 0)
                {
                    if (edgeTable[cube_index] & 1)
                    {
                        vertex_index = get_vertex_index(i - 1, j - 1, k, 'k');
                        if (vertex_index == -1)
                        {
                            push_vertices(i - 1, j - 1, k, 'k', pixdim);
                            vertlist[0] = count;
                            vertex_map[i - 1][j - 1][k]['k'] = count;
                            count++;
                        }
                        else
                        {
                            vertlist[0] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 2) 
                    {
                        vertex_index = get_vertex_index(i - 1, j, k, 'j');
                        if (vertex_index == -1)
                        {
                            push_vertices(i - 1, j, k, 'j', pixdim);
                            vertlist[1] = count;
                            vertex_map[i - 1][j][k]['j'] = count;
                            count++;
                        }
                        else
                        {
                            vertlist[1] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 4) 
                    {
                        vertex_index = get_vertex_index(i - 1, j, k, 'k');
                        if (vertex_index == -1)
                        {
                            push_vertices(i - 1, j, k, 'k', pixdim);
                            vertlist[2] = count;
                            vertex_map[i - 1][j][k]['k'] = count;
                            count++;
                        }
                        else
                        {
                            vertlist[2] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 8) 
                    {
                        vertex_index = get_vertex_index(i - 1, j, k - 1, 'j');
                        if (vertex_index == -1)
                        {
                            push_vertices(i - 1, j, k - 1, 'j', pixdim);
                            vertlist[3] = count;
                            vertex_map[i - 1][j][k - 1]['j'] = count;
                            count++;
                        }
                        else
                        {
                            vertlist[3] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 16) 
                    {
                        vertex_index = get_vertex_index(i, j - 1, k, 'k');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j - 1, k, 'k', pixdim);
                            vertlist[4] = count;
                            vertex_map[i][j - 1][k]['k'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[4] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 32) 
                    {
                        vertex_index = get_vertex_index(i, j, k, 'j');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j, k, 'j', pixdim);
                            vertlist[5] = count;
                            vertex_map[i][j][k]['j'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[5] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 64) 
                    {
                        vertex_index = get_vertex_index(i, j, k, 'k');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j, k, 'k', pixdim);
                            vertlist[6] = count;
                            vertex_map[i][j][k]['k'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[6] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 128) 
                    {
                        vertex_index = get_vertex_index(i, j, k - 1, 'j');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j, k - 1, 'j', pixdim);
                            vertlist[7] = count;
                            vertex_map[i][j][k - 1]['j'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[7] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 256) 
                    {
                        vertex_index = get_vertex_index(i, j - 1, k - 1, 'i');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j - 1, k - 1, 'i', pixdim);
                            vertlist[8] = count;
                            vertex_map[i][j - 1][k - 1]['i'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[8] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 512) 
                    {
                        vertex_index = get_vertex_index(i, j - 1, k, 'i');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j - 1, k, 'i', pixdim);
                            vertlist[9] = count;
                            vertex_map[i][j - 1][k]['i'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[9] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 1024) 
                    {
                        vertex_index = get_vertex_index(i, j, k, 'i');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j, k, 'i', pixdim);
                            vertlist[10] = count;
                            vertex_map[i][j][k]['i'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[10] = vertex_index;
                        }
                    }
                    if (edgeTable[cube_index] & 2048) 
                    {
                        vertex_index = get_vertex_index(i, j, k - 1, 'i');
                        if (vertex_index == -1)
                        {
                            push_vertices(i, j, k - 1, 'i', pixdim);
                            vertlist[11] = count;
                            vertex_map[i][j][k - 1]['i'] = count;
                            count++;    
                        }
                        else
                        {
                            vertlist[11] = vertex_index;
                        }
                    }

                    for (int t = 0; triTable[cube_index][t] != -1; t += 3)
                    {
                        triangle[0] = vertlist[triTable[cube_index][t]];
                        triangle[1] = vertlist[triTable[cube_index][t + 1]];
                        triangle[2] = vertlist[triTable[cube_index][t + 2]];
                        indices.push_back(triangle[0]);
                        indices.push_back(triangle[1]);
                        indices.push_back(triangle[2]);
                        a = glm::vec3(vertices[triangle[0] * 3], vertices[triangle[0] * 3 + 1], vertices[triangle[0] * 3 + 2]);
                        b = glm::vec3(vertices[triangle[1] * 3], vertices[triangle[1] * 3 + 1], vertices[triangle[1] * 3 + 2]);
                        c = glm::vec3(vertices[triangle[2] * 3], vertices[triangle[2] * 3 + 1], vertices[triangle[2] * 3 + 2]);
                        normal = normalize(cross(b - a, c - b));
                        normals[triangle[0] * 3] += normal[0];
                        normals[triangle[0] * 3 + 1] += normal[1];
                        normals[triangle[0] * 3 + 2] += normal[2];
                        normals[triangle[1] * 3] += normal[0];
                        normals[triangle[1] * 3 + 1] += normal[1];
                        normals[triangle[1] * 3 + 2] += normal[2];
                        normals[triangle[2] * 3] += normal[0];
                        normals[triangle[2] * 3 + 1] += normal[1];
                        normals[triangle[2] * 3 + 2] += normal[2];
                        normal_counts[triangle[0]]++;
                        normal_counts[triangle[1]]++;
                        normal_counts[triangle[2]]++;
                    }
                }
            }
        }
    }

    int num_vertices = vertices.size();
    for (int i = 0; i < num_vertices; i += 3)
    {
        colors.push_back(1);
        colors.push_back(0);
        colors.push_back(0);
    }

    // center the object
    float object_center[3];
    object_center[0] = (dim[2] - 1) * pixdim[2] / 2;
    object_center[1] = (dim[1] - 1) * pixdim[1] / 2;
    object_center[2] = (dim[3] - 1) * pixdim[3] / 2;
    std::cout << object_center[0] << std::endl;
    std::cout << object_center[1] << std::endl;
    std::cout << object_center[2] << std::endl;
    for (int i = 0; i < num_vertices; i += 3)
    {
        vertices[i] -= object_center[0];
        vertices[i + 1] -= object_center[1];
        vertices[i + 2] -= object_center[2];
    }

    // average the normals
    for (int i = 0; i < count; i++)
    {
        normals[i * 3] /= normal_counts[i];
        normals[i * 3 + 1] /= normal_counts[i];
        normals[i * 3 + 2] /= normal_counts[i];
        normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
        normal = normalize(normal);
        normals[i * 3] = normal[0];
        normals[i * 3 + 1] = normal[1];
        normals[i * 3 + 2] = normal[2];
    }

    return 1;
}