template <class Mat>
inline bool any_isnan(Mat & A)
{
  for(int a = 0;a<A.size();a++)
  {
    if(A(a) != A(a))
      return true;
  }
  return false;
}
