#ifndef _MKLEIGENINTERFACE_H__
#define _MKLEIGENINTERFACE_H__


#include <Eigen/Dense>
#ifdef WIN32
#  include <mkl_blas.h>
#endif

// Just fall back on Eigen if no MKL is available

// computes x = A*b, assuming everything including 'x' has been initialized and has the right size
// assumes column-major ordering
template <class MatX, class MatA, class MatB> void MKL_matVecMult_double(MatX &x, const MatA &A, const MatB &b)
{
#ifndef WIN32
    x = A*b;
#else
	assert(x.rows() == A.rows() && x.cols() == 1);
	assert(A.cols() == b.rows() && b.cols() == 1);
	// assert on doubles?

	char trans = 'n';
	MKL_INT m = A.rows();
	MKL_INT n = A.cols();
	double alpha = 1.0;
	MKL_INT lda = m;
	MKL_INT incx = 1;
	double beta = 0.0;
	MKL_INT incy = 1;

	dgemv(&trans, &m, &n, &alpha, A.data(), &lda, b.data(), &incx, &beta, x.data(), &incy);
#endif
}


// computes X = A*B, assuming everything including 'X' has been initialized and has the right size
// assumes column-major ordering
template <class MatX, class MatA, class MatB> void MKL_matMatMult_double(MatX &X, const MatA &A, const MatB &B)
{
#ifndef WIN32
	X = A*B;
#else
	assert(X.rows() == A.rows());
	assert(X.cols() == B.cols());
	assert(A.cols() == B.rows());
	char transa = 'n';
	char transb = 'n';
	MKL_INT m = A.rows();
	MKL_INT k = A.cols();
	MKL_INT n = B.cols();
	double  alpha = 1.0f;
	MKL_INT lda = m;
	MKL_INT ldb = k;	
	MKL_INT ldc = m;	
	double beta = 0.0f;	
	dgemm(&transa, &transb, &m, &n, &k, &alpha, A.data(), &lda, B.data(), &ldb, &beta, X.data(), &ldc);
#endif
}


//void MKL_matVecMult(Eigen::MatrixXd &x, const Eigen::MatrixXd &A, const Eigen::MatrixXd &b)
//{
//	assert(x.rows() == A.rows() && x.cols() == 1);
//	assert(A.cols() == b.rows() && b.cols() == 1);
//
//	char trans = 'n';
//	MKL_INT m = A.rows();
//	MKL_INT n = A.cols();
//	double alpha = 1.0;
//	MKL_INT lda = m;
//	MKL_INT incx = 1;
//	double beta = 0.0;
//	MKL_INT incy = 1;
//
//	dgemv(&trans, &m, &n, &alpha, A.data(), &lda, b.data(), &incx, &beta, x.data(), &incy);
//}

//void MKL_matVecMult(Eigen::MatrixXf &x, const Eigen::MatrixXf &A, const Eigen::MatrixXf &b)
template <class MatX, class MatA, class MatB> void MKL_matVecMult_single(MatX &x, const MatA &A, const MatB &b)
{
#ifndef WIN32
    x = A*b;
#else
	assert(x.rows() == A.rows() && x.cols() == 1);
	assert(A.cols() == b.rows() && b.cols() == 1);

	char trans = 'n';
	MKL_INT m = A.rows();
	MKL_INT n = A.cols();
	float alpha = 1.0f;
	MKL_INT lda = m;
	MKL_INT incx = 1;
	float beta = 0.0f;
	MKL_INT incy = 1;

	sgemv(&trans, &m, &n, &alpha, A.data(), &lda, b.data(), &incx, &beta, x.data(), &incy);
#endif
}

// computes X = A*B, assuming everything including 'X' has been initialized and has the right size
// assumes column-major ordering
template <class MatX, class MatA, class MatB> void MKL_matMatMult_single(MatX &X, const MatA &A, const MatB &B)
{
#ifndef WIN32
	X = A*B;
#else
	assert(X.rows() == A.rows());
	assert(X.cols() == B.cols());
	assert(A.cols() == B.rows());
	char transa = 'n';
	char transb = 'n';
	MKL_INT m = A.rows();
	MKL_INT k = A.cols();
	MKL_INT n = B.cols();
	float alpha = 1.0f;
	MKL_INT lda = m;
	MKL_INT ldb = k;	
	MKL_INT ldc = m;	
	float beta = 0.0f;	
	sgemm(&transa, &transb, &m, &n, &k, &alpha, A.data(), &lda, B.data(), &ldb, &beta, X.data(), &ldc);
#endif
}


#endif
