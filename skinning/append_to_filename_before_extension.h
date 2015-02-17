#include <string>
// Appends str to name before extension.
// 
// Inputs:
//   name  original path
//   str  str to be inserted
//   ext  new extension to be used
// Returns new string
//
// Example
//   /foo/bar.txt.zip becomes /foo/bar.txt + str + .ext
//
inline std::string append_to_filename_before_extension(
  const std::string name,
  const std::string str,
  const std::string ext);

// Implementation
#include <igl/pathinfo.h>
#include <iostream>
inline std::string append_to_filename_before_extension(
  const std::string name,
  const std::string str,
  const std::string ext)
{
  std::string dirname, basename, extension, filename;
  igl::pathinfo(name,dirname,basename,extension,filename);
  std::stringstream result;
  result << dirname << "/" << filename << str << "." << ext;
  return result.str();
}
