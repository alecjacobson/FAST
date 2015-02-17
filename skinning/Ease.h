#ifndef EASE_H
#define EASE_H
enum Ease
{
  EASE_CUBIC, // -2*t.^3+3*t.^2;
  EASE_QUINTIC, /// 10*t.^3 - 15*t.^4 + 6*t.^5;
  EASE_CUBIC_BSPLINE, // 2*t.^3.*(t<0.5) + (1-6*t+12*t.^2-6*t.^3).*(t>=0.5);
};
#define NUM_EASE 3

// Apply ease curve to scalar value, assumes input is between 0 and 1
//
// Templates:
//   T scalar type, like double
// Inputs:
//   ease  type of ease filter to use (see enum above)
//   t  input scalar
// Returns ease filter applied to t
//
// Example: 
//   // Apply to Eigen::MatrixXd t
//   MatrixXd s = 
//     t.array().unaryExpr(bind1st(ptr_fun(ease<MatrixXd::Scalar>),EASE_CUBIC));
// 
template <typename T>
T ease(const Ease ease, const T t);

template <typename T>
T ease(const Ease ease, const T t)
{
  switch(ease)
  {
    case EASE_CUBIC:
      return -2.0*t*t*t+3.0*t*t;
    case EASE_QUINTIC:
      return 10.0*t*t*t - 15.0*t*t*t*t + 6.0*t*t*t*t*t;
    case EASE_CUBIC_BSPLINE:
      return (t<0.5 ? 2*t*t*t : 1-6*t+12*t*t-6*t*t*t);
    default:
      assert(false);
      return t;
  }
}


#endif
