#include <math.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
	FILE *f[2];
	int current_read;
	int max_position;
	double xpoint[2], ypoint[2];
	double max_diff;
	char unused[1024];

	f[0] = fopen (argv[1], "r");
	f[1] = fopen (argv[2], "r");
	if (f[0] == NULL || f[1] == NULL)
	{
		printf ("Cannot open passed files.\n");
		return -1;
	}

	fscanf (f[0], "%s %lf %lf", unused, &xpoint[0], &ypoint[0]);
	fscanf (f[1], "%s %lf %lf", unused, &xpoint[1], &ypoint[1]);
	current_read = 1;
	max_position = current_read;
	max_diff = fabs (ypoint[1] - ypoint[0]);
	while (!feof(f[0]))
	{
		fscanf (f[0], "%s %lf %lf", unused, &xpoint[0], &ypoint[0]);
		fscanf (f[1], "%s %lf %lf", unused, &xpoint[1], &ypoint[1]);
		current_read++;

		if (fabs(ypoint[1] - ypoint[0]) > max_diff)
		{
			max_diff = fabs(ypoint[1] - ypoint[0]);
			max_position = current_read;
		}
	}

	printf ("%lf\n", max_diff);

	fclose (f[0]);
	fclose (f[1]);

	return 0;
}