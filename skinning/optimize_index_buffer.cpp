#include "optimize_index_buffer.h"
#include "vcacheopt.h"
#include <cstdio>

void optimize_index_buffer(
  const Eigen::MatrixXi& F, 
  const bool silent,
  Eigen::MatrixXi& OF)
{
  const int numTris = F.rows();
  std::vector<int> triBuf;
  for (int t=0; t<numTris; t++) 
  {
    triBuf.push_back(F(t,0));
    triBuf.push_back(F(t,1));
    triBuf.push_back(F(t,2));
  }
  VertexCache vertex_cache;
  int misses = vertex_cache.GetCacheMissCount(&triBuf[0], numTris);

  if (!silent)
  {
    printf("*** Before optimization ***\n");
    printf("Cache misses\t: %d\n", misses);
    printf("ACMR\t\t: %f\n", (float)misses / (float)numTris);
  }

  VertexCacheOptimizer vco;
  if(!silent)
  {
    printf("Optimizing ... \n");  
  }
  // vco.draw_list is the new order
  VertexCacheOptimizer::Result res = vco.Optimize(&triBuf[0], numTris);  
  if (res)
  {
    printf("Error in vertex cache optimization...\n");    
  }

  misses = vertex_cache.GetCacheMissCount(&triBuf[0], numTris);

  if (!silent)
  {
    printf("*** After optimization ***\n");
    printf("Cache misses\t: %d\n", misses);
    printf("ACMR\t\t: %f\n", (float)misses / (float)numTris);
  }

  for (int t=0; t<numTris; t++) 
  {
    OF(t,0) = triBuf[3*t];
    OF(t,1) = triBuf[3*t + 1];
    OF(t,2) = triBuf[3*t + 2];
  }
}

