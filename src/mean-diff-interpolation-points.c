#include <math.h>
#include <stdio.h>

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)<(b)?(b):(a))

int main (int argc, char *argv[])
{
	FILE *f[2];
	int current_read;
	int res[2];
	int total_read;
	double xpoint[2], ypoint[2], mean, sqr_mean_err;
	double acc_diff;
	double diff;
	char counter[1024];

	if (argc != 4)
	{
		printf ("Invalid number of arguments given - counter, file1, file2\n");
		return -2;
	}

	f[0] = fopen (argv[2], "r");
	f[1] = fopen (argv[3], "r");
	if (f[0] == NULL || f[1] == NULL)
	{
		printf ("Cannot open passed files.\n");
		return -1;
	}

	/* Calculate the mean value */
	fscanf (f[0], "%s %lf %lf", counter, &xpoint[0], &ypoint[0]);
	fscanf (f[1], "%s %lf %lf", counter, &xpoint[1], &ypoint[1]);
	current_read = 1;
	if (ypoint[0] == 0 || ypoint[1] == 0)
		acc_diff = 0;
	else
		acc_diff = fabs(ypoint[1] - ypoint[0])/fabs(MAX(ypoint[0],ypoint[1]));
	while (!feof(f[0]))
	{
		res[0] = fscanf (f[0], "%s %lf %lf", counter, &xpoint[0], &ypoint[0]);
		res[1] = fscanf (f[1], "%s %lf %lf", counter, &xpoint[1], &ypoint[1]);
		if (res[0] != 3 || res[1] != 3)
			break;

		if (strcmp (counter, argv[1]) == 0)
		{

			//printf ("parsing <%lf,%lf> vs <%lf,%lf>\n", xpoint[0], ypoint[0], xpoint[1], ypoint[1]);

			current_read++;

			if (ypoint[0] == 0 || ypoint[1] == 0)
				{}
			else
				//acc_diff += fabs(ypoint[1] - ypoint[0])/fabs(MAX(ypoint[0],ypoint[1]));
				acc_diff += fabs(ypoint[1] - ypoint[0]);

			//printf ("ypoint[1] = %lf ypoint[0] = %lf abs_diff = %lf acc = %lf \n", ypoint[1], ypoint[0], fabs(ypoint[1] - ypoint[0]), acc_diff);
		}

	}
	total_read = current_read;
	mean = acc_diff / total_read;

	/* Calculate the standard deviation */
	rewind (f[0]);
	rewind (f[1]);
	fscanf (f[0], "%s %lf %lf", counter, &xpoint[0], &ypoint[0]);
	fscanf (f[1], "%s %lf %lf", counter, &xpoint[1], &ypoint[1]);
	if (ypoint[0] == 0 || ypoint[1] == 0)
		diff = 0;
	else
		diff = fabs(ypoint[1] - ypoint[0])/fabs(MAX(ypoint[0],ypoint[1]));
	acc_diff = (diff - mean) * (diff - mean);
	while (!feof(f[0]))
	{
		res[0] = fscanf (f[0], "%s %lf %lf", counter, &xpoint[0], &ypoint[0]);
		res[1] = fscanf (f[1], "%s %lf %lf", counter, &xpoint[1], &ypoint[1]);
		if (res[0] != 3 || res[1] != 3)
			break;
			
		if (strcmp (counter, argv[1]) == 0)
		{

			if (ypoint[0] == 0 || ypoint[1] == 0)
				diff = 0;
			else
				diff = fabs(ypoint[1] - ypoint[0])/fabs(MAX(ypoint[0],ypoint[1]));
		
			acc_diff += (diff - mean) * (diff - mean);

		}

	}
	sqr_mean_err = sqrt ( acc_diff / (total_read * (total_read-1)) );

	printf ("%lf %lf\n", mean, sqr_mean_err);

	fclose (f[0]);
	fclose (f[1]);

	return 0;
}
