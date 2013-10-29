#include <math.h>
#include <stdio.h>

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)<(b)?(b):(a))

int main (int argc, char *argv[])
{
	FILE *f[2];
	int current_read;
	int res[2];
	int total_read = 0;
	double xpoint[2], ypoint[2], mean = 0, sqr_mean_err = 0;
	double acc_diff;
	double diff;
	char counter[1024];
	char region[1024];
	unsigned group;
	unsigned cgroup;

	if (argc != 6)
	{
		printf ("Invalid number of arguments given - region, group, counter, file1, file2\n");
		return -2;
	}

	f[0] = fopen (argv[4], "r");
	f[1] = fopen (argv[5], "r");
	if (f[0] == NULL || f[1] == NULL)
	{
		printf ("Cannot open passed files - %s and %s.\n", argv[4], argv[5]);
		return -1;
	}
	cgroup = (unsigned) atoi(argv[2]);

	/* Calculate the mean value */
	res[0] = fscanf (f[0], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[0], &ypoint[0]);
	res[1] = fscanf (f[1], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[1], &ypoint[1]);
	if (res[0] == 5 && res[1] == 5)
	{
		current_read = 1;
		if (ypoint[0] == 0 || ypoint[1] == 0)
			acc_diff = 0;
		else
			acc_diff = fabs(ypoint[1] - ypoint[0])/fabs(MAX(ypoint[0],ypoint[1]));
		while (!feof(f[0]))
		{
			res[0] = fscanf (f[0], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[0], &ypoint[0]);
			res[1] = fscanf (f[1], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[1], &ypoint[1]);
			if (res[0] != 5 || res[1] != 5)
				break;
	
			if (strcmp (counter, argv[3]) == 0 && strcmp (region, argv[1]) == 0 && group == cgroup)
			{
				current_read++;
	
				if (ypoint[0] == 0 || ypoint[1] == 0)
					{}
				else
					acc_diff += fabs(ypoint[1] - ypoint[0]);
			}
	
		}
		total_read = current_read;
		mean = acc_diff / total_read;
	}

	/* Calculate the standard deviation */
	rewind (f[0]);
	rewind (f[1]);
	res[0] = fscanf (f[0], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[0], &ypoint[0]);
	res[1] = fscanf (f[1], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[1], &ypoint[1]);
	if (res[0] == 5 && res[1] == 5)
	{
		if (ypoint[0] == 0 || ypoint[1] == 0)
			diff = 0;
		else
			diff = fabs(ypoint[1] - ypoint[0])/fabs(MAX(ypoint[0],ypoint[1]));
		acc_diff = (diff - mean) * (diff - mean);
		while (!feof(f[0]))
		{
			res[0] = fscanf (f[0], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[0], &ypoint[0]);
			res[1] = fscanf (f[1], "%[^;];%u;%[^;];%lf;%lf\n", region, &group, counter, &xpoint[1], &ypoint[1]);
			if (res[0] != 5 || res[1] != 5)
				break;
				
			if (strcmp (counter, argv[3]) == 0 && strcmp (counter, argv[1]) == 0 && group == cgroup)
			{
	
				if (ypoint[0] == 0 || ypoint[1] == 0)
					diff = 0;
				else
					diff = fabs(ypoint[1] - ypoint[0])/fabs(MAX(ypoint[0],ypoint[1]));
			
				acc_diff += (diff - mean) * (diff - mean);
	
			}
	
		}
		sqr_mean_err = sqrt ( acc_diff / (total_read * (total_read-1)) );
	}
	
	printf ("%lf %lf\n", mean, sqr_mean_err);

	fclose (f[0]);
	fclose (f[1]);

	return 0;
}
