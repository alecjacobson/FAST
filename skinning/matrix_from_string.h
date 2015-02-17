#ifndef MATRIX_FROM_STRING_H
#define MATRIX_FROM_STRING_H
#include <string>
// Parses a ROW-MAJOR ORDER matrix (of known size) from a string
//
// Templates
//   mat  matrix type with R rows and C columns
// Inputs:
//   R  number of rows
//   C  number of columns
//   str  input string
// Outputs:
//   M  output matrix
// Returns true only if read successfully
template <typename Mat>
bool matrix_from_string(
  const int R, 
  const int C, 
  const std::string str, 
  Mat & M);


// Implementation

#include "tokenize_str.h"
#include <igl/verbose.h>

template <typename Mat>
bool matrix_from_string(
  const int R, 
  const int C, 
  const std::string str, 
  Mat & M)
{
  using namespace std;
  using namespace igl;
  vector<string> tokens = tokenize_str(str," \t,;][\n\r");
  if(int(tokens.size()) != R*C)
  {
    verbose("Tokens found (%d) != rows*cols (%d)\n",tokens.size(),R*C);
    return false;
  }

  // Double, temporary array
  std::vector<double> D(R*C);
  for(int i = 0;i<R;i++)
  {
    for(int j = 0;j<C;j++)
    {
      D[i*C+j] = atof(tokens[i*C+j].c_str());
    }
  }
  
  for(int i = 0;i<R;i++)
  {
    for(int j = 0;j<C;j++)
    {
      M(i,j) = D[i*C+j];
    }
  }
  return true;
}


#endif
