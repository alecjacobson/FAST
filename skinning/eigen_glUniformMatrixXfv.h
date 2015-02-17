#include <Eigen/Core>
#if __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef _WIN32
#    define NOMINMAX
#    include <Windows.h>
#    undef NOMINMAX
#  endif
#  include <GL/gl.h>
#endif

// wrapper for glUniformMatrixXfv where X could be #diag or #colsx#rows for
// square and non square matrices respectively
// Inputs:
//   location  shader variable location (see glUniformMatrix)
//   count  number of matrices to pass
//   T  #rows by #cols*count eigen matrix 
// Returns true on success, false on errors
bool eigen_glUniformMatrixXfv(
  const GLint location,
  const GLsizei count,
  const Eigen::MatrixXf & T);


// Implementation

#ifdef _WIN32

bool eigen_glUniformMatrixXfv(
  const GLint location,
  const GLsizei count,
  const Eigen::MatrixXf & T)
{
  int rows = T.rows();
  int cols = T.cols()/count;
  // Windows has the wrong return types for glUniformMatrix etc so for now only 4x4 will be supported 
  assert(rows==4);
  assert(cols==4);
 // Transpose is "screwed up" when using non-square matrices. It should not be
  // trusted:
  //  http://stackoverflow.com/questions/7658402/
#define FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES false
  if(T.Options & Eigen::RowMajor)
  {
    // Create temporary copy of transpose
    Eigen::MatrixXf TT = T.transpose();
    glUniformMatrix4fv(
      location,
      count,
      FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES,
      TT.data());
  }else
  {
    glUniformMatrix4fv(
      location,
      count,
      FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES,
      T.data());
  }
  GLenum err = igl::report_gl_error("transformations(T): ");
  if(err != GL_NO_ERROR)
  {
    return false;
  }
  return true;
#undef FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES
}

#else

bool eigen_glUniformMatrixXfv(
  const GLint location,
  const GLsizei count,
  const Eigen::MatrixXf & T)
{
  int rows = T.rows();
  int cols = T.cols()/count;
  assert(rows<=4 && rows>=2);
  assert(cols<=4 && cols>=2);

  // function pointer to corresponding glUniformMatrixXfv function
  void (*glUniformMatrixXfv)(
    GLint location, 
    GLsizei  count, 
    GLboolean  transpose, 
    const GLfloat * value);

  switch(cols)
  {
    case 4:
      switch(rows)
      {
        case 4:
          glUniformMatrixXfv = &glUniformMatrix4fv;
          break;
        case 3:
          glUniformMatrixXfv = &glUniformMatrix4x3fv;
          break;
        case 2:
          glUniformMatrixXfv = &glUniformMatrix4x2fv;
          break;
        default:
          fprintf(stderr,
            "Error: eigen_glUniformMatrixXfv()"
            " #rows (%d) must be between 2 and 4\n",
            rows);
          return false;
      }
      break;
    case 3:
      switch(rows)
      {
        case 4:
          glUniformMatrixXfv = &glUniformMatrix3x4fv;
          break;
        case 3:
          glUniformMatrixXfv = &glUniformMatrix3fv;
          break;
        case 2:
          glUniformMatrixXfv = &glUniformMatrix3x2fv;
          break;
        default:
          fprintf(stderr,
            "Error: eigen_glUniformMatrixXfv()"
            " #rows (%d) must be between 2 and 4\n",
            rows);
          return false;
      }
      break;
    case 2:
      switch(rows)
      {
        case 4:
          glUniformMatrixXfv = &glUniformMatrix2x4fv;
          break;
        case 3:
          glUniformMatrixXfv = &glUniformMatrix2x3fv;
          break;
        case 2:
          glUniformMatrixXfv = &glUniformMatrix2fv;
          break;
        default:
          fprintf(stderr,
            "Error: eigen_glUniformMatrixXfv()"
            " #rows (%d) must be between 2 and 4\n",
            rows);
          return false;
      }
      break;
    default:
      fprintf(stderr,
        "Error: eigen_glUniformMatrixXfv() #cols"
        " (%d) must be between 2 and 4\n",
        cols);
      return false;
  }

  // Transpose is "screwed up" when using non-square matrices. It should not be
  // trusted:
  //  http://stackoverflow.com/questions/7658402/
#define FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES false
  if(T.Options & Eigen::RowMajor)
  {
    // Create temporary copy of transpose
    Eigen::MatrixXf TT = T.transpose();
    glUniformMatrixXfv(
      location,
      count,
      FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES,
      TT.data());
  }else
  {
    glUniformMatrixXfv(
      location,
      count,
      FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES,
      T.data());
  }
  GLenum err = igl::report_gl_error("transformations(T): ");
  if(err != GL_NO_ERROR)
  {
    return false;
  }
  return true;
#undef FALSE_NEVER_USE_TRANSPOSE_WITH_NON_SQUARE_MATRICES
}

#endif
