#include <stdio.h>
#include <stdlib.h>
#include <math.h>


float r = 0.14;
int samples = 32;
int currSamples = 0;

float sampled[32][2] = {0};


float getDist(float a, float b, float x, float y)
{
  return sqrt(pow(x - a, 2.0) + pow(y - b, 2.0)); 

}


int statisfy(float a, float b)
{
  if (currSamples == 0) return 1;

  if (a * a + b * b > 1.0f) return 0;

  for (int i = 0; i < currSamples; i++)
  {
    if (getDist(a, b, sampled[i][0], sampled[i][1]) < r) return 0;
  }
  return 1;
}

int main()
{
  while (1)
  {
    if (currSamples == samples) break;

    float x = (rand() % 101) / 100.0;
    float y = (rand() % 101) / 100.0;

    x = x * 2.0 - 1.0;
    y = y * 2.0 - 1.0;

    if (!statisfy(x, y)) continue;

    sampled[currSamples][0] = x;
    sampled[currSamples][1] = y;
    currSamples += 1;
  }

  FILE* fp;

  fp = fopen("data.txt", "w");

  for (int i = 0; i < samples; i++)
  {
    fprintf(fp, "%f %f\n", sampled[i][0], sampled[i][1]);
  }
 
  return 0;
}

