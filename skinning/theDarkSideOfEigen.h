#include <igl/get_seconds_hires.h>

#include "MKLEigenInterface.h"

void theDarkSideOfEigen()
{
	Eigen::MatrixXf CSM_M0(100, 100);
	Eigen::MatrixXf L(100, 1);
	Eigen::MatrixXf Mres(100, 1);	
	Eigen::MatrixXf Mres2(100, 1);	

	CSM_M0.setOnes();
	L.setOnes();

	double sec_start = get_seconds_hires();
	for (int test=0; test<1000000000; test++)
	{		
		//Mres = CSM_M0 * L; // seems like some dynamic memory allocations are happening here

		MKL_matVecMult(Mres2, CSM_M0, L);

		//Eigen::MatrixXd diff = Mres - Mres2;
		//double diffNorm = diff.norm();

		//int hu = 1;


		//for (int i=0; i<Mres.rows(); i++)
		//{
		//	double tmp = 0.0;

		//	for (int j=0; j<CSM_M0.cols(); j++)
		//		tmp += CSM_M0(i, j) * L(j, 0);

		//	Mres(i, 0) = tmp;
		//}

		if (test % 1000 == 0)
		{
			double elapsed_sec = get_seconds_hires() - sec_start;
			printf("elapsed = %fms\n", elapsed_sec*1000.0);
			sec_start = get_seconds_hires();
		}
	}
}
