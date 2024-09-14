/**
* C port of K.jpg's OpenSimplex 2, smooth varient ("SuperSimplex")
*/

void opensimplex_init();
void opensimple_free();

float opensimplex2s_noise2(long seed, double x, double y);
float opensimplex2s_noise2_ImproveX(long seed, double x, double y);

float opensimplex2s_noise3_ImproveXY(long seed, double x, double y, double z);
float opensimplex2s_noise3_ImproveXZ(long seed, double x, double y, double z);
float opensimplex2s_noise3_Fallback(long seed, double x, double y, double z);

float opensimplex2s_noise4_ImproveXYZ_ImproveXY(long seed, double x, double y, double z, double w);
float opensimplex2s_noise4_ImproveXYZ_ImproveXZ(long seed, double x, double y, double z, double w);
float opensimplex2s_noise4_ImproveXYZ(long seed, double x, double y, double z, double w);
float opensimplex2s_noise_ImproveXY_ImproveZW(long seed, double x, double y, double z, double w);
float opensimplex_noise_Fallback(long seed, double x, double y, double z, double w);
