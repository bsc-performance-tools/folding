#include <math.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
	FILE *f[2];
	int current_read;
	int res[2];
	int total_read;
	double xpoint[2], ypoint[2], mean, std_dev;
	double acc_diff;
	char unused[1024];

	f[0] = fopen (argv[1], "r");
	f[1] = fopen (argv[2], "r");
	if (f[0] == NULL || f[1] == NULL)
	{
		printf ("Cannot open passed files.\n");
		return -1;
	}

	/* Calculate the mean value */
	fscanf (f[0], "%s %lf %lf", unused, &xpoint[0], &ypoint[0]);
	fscanf (f[1], "%s %lf %lf", unused, &xpoint[1], &ypoint[1]);
	current_read = 1;
	acc_diff = fabs (ypoint[1] - ypoint[0]);
	while (!feof(f[0]))
	{
		res[0] = fscanf (f[0], "%s %lf %lf", unused, &xpoint[0], &ypoint[0]);
		res[1] = fscanf (f[1], "%s %lf %lf", unused, &xpoint[1], &ypoint[1]);
		if (res[0] != 3 || res[1] != 3)
			break;

		current_read++;
		acc_diff += fabs(ypoint[1] - ypoint[0]);
	}
	total_read = current_read;
	mean = acc_diff / total_read;

	/* Calculate the standard deviation */
	rewind (f[0]);
	rewind (f[1]);
	fscanf (f[0], "%s %lf %lf", unused, &xpoint[0], &ypoint[0]);
	fscanf (f[1], "%s %lf %lf", unused, &xpoint[1], &ypoint[1]);
	acc_diff = ((ypoint[1] - ypoint[0]) - mean) * ((ypoint[1] - ypoint[0]) - mean);
	while (!feof(f[0]))
	{
		res[0] = fscanf (f[0], "%s %lf %lf", unused, &xpoint[0], &ypoint[0]);
		res[1] = fscanf (f[1], "%s %lf %lf", unused, &xpoint[1], &ypoint[1]);
		if (res[0] != 3 || res[1] != 3)
			break;
			
		acc_diff += ((ypoint[1] - ypoint[0]) - mean) * ((ypoint[1] - ypoint[0]) - mean);
	}
	std_dev = sqrt ((1.0f/((double)total_read-1))*acc_diff);

	printf ("%lf %lf\n", mean, std_dev);

	fclose (f[0]);
	fclose (f[1]);

	return 0;
}