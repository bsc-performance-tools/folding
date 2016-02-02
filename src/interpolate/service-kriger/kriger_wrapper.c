
#include <stdio.h>
#include <stdlib.h>

#define F77(x) x##_

extern void F77(call_kriger_region) (int*, double*, double*, int*, double*,
	double*, double*, double *);

void Kriger_Region (int num_in_samples, double *X_samples, double *Y_samples,
	int num_out_samples, double *Y_out_samples, double min_value,
	double max_value, double nuget_kri)
{
	F77(call_kriger_region) (&num_in_samples, X_samples, Y_samples,
		&num_out_samples, Y_out_samples, &min_value, &max_value,
		&nuget_kri);
}

extern void F77(call_kriger_point)(int*, double*, double*, double*, double*,
	double*, double*, double*);

void Kriger_Point (int num_in_samples, double *X_samples, double *Y_samples,
	double X_in, double *Y_out, double min_value, double max_value,
	double nuget_kri)
{
	F77(call_kriger_point) (&num_in_samples, X_samples, Y_samples,
		&X_in, Y_out, &min_value, &max_value, &nuget_kri);
}
