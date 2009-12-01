
#ifndef KRIGER_WRAPPER
#define KRIGER_WRAPPER

#ifdef __cplusplus
extern "C" {
#endif

void Kriger_Region (int num_in_samples, double *X_samples, double *Y_samples, int num_out_samples, double *Y_out_samples, double min_value, double max_value);
void Kriger_Point (int num_in_samples, double *X_samples, double *Y_samples, double *X_in, double *Y_out, double min_value, double max_value);

#ifdef __cplusplus
}
#endif

#endif
