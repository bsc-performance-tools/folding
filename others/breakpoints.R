require (strucchange)

#
# MAIN code section
#

main <- function (file, H, nsteps, max_error, counter, group, ncores)
{
	message <- sprintf ("Working on file %s, H = %f, counter = %s, group = %d", file, H, counter, group)
	print (message)

	if (ncores > 1)
	{
		require (doParallel)
		registerDoParallel(cores=ncores)
		HPC <- "foreach"
	}
	else
	{
		HPC <- "none"
	}

	# Prepare my palette (based on rainbow(X) but excluding dark blue and grayscale)
	mypalette <- c("#FF0000FF","#FF8000FF","#FFFF00FF","#80FF00FF","#00FFFFFF","#009FFFFF","#8000FFFF","#FF00BFFF")
	palette(mypalette)

	# Read file
	df <- read.csv (file = file, head = TRUE, sep=";")
	df_o <- df[order(df$time),]

	#df_r_o <- df_o[rev(rownames(df_o)),]
	df_r_o <- data.frame (time = rep(0, nrow(df_o)), value = rep (0, nrow(df_o)))
	for (i in 1:nrow(df_o))
		df_r_o[i, ] <- c(df_o$time[nrow(df_o)-i+1], df_o$value[nrow(df_o)-i+1])

	# Actual computation!
	if (nrow(df_o) > 10)
	{
		bp.counter   <- breakpoints (df_o$value   ~ df_o$time,   h = H, hpc = HPC)
		bp.r.counter <- breakpoints (df_r_o$value ~ df_r_o$time, h = H, hpc = HPC)

		if (DEBUG)
		{
			summary (bp.counter)
			summary (bp.r.counter)
		}

		# Construct breakpoints vector (fwd values)
		# Instead of bd.time <- c (0, breakdates (bp.counter), 1)
		bpoints <- bp.counter[[1]]
		bd.time <- c(0)
		if (!is.na(bpoints[1]))
		{
			for (bp in 1:length(bpoints))
				bd.time <- c (bd.time, df_o$time[bpoints[bp]])
		}
		bd.time <- c(bd.time, 1)

		# Construct reverse breakpoints vector (bwd values)
		bpoints <- bp.r.counter[[1]]
		bd.r.time <- c(1)
		if (!is.na(bpoints[1]))
		{
			for (bp in 1:length(bpoints))
				bd.r.time <- c (bd.r.time, df_r_o$time[bpoints[bp]])
		}
		bd.r.time <- c(bd.r.time, 0)
	}
	else
	{
		bd.time <- c (0, 1)
		bd.r.time <- c (1, 0)
	}

	if (length(bd.time) == length(bd.r.time))
	{
		# Calculate the average between the fwd and bwd
		bd.f.time <- c()
		for (bp in 1:length(bd.time))
			bd.f.time <- c(bd.f.time, (bd.time[bp] + bd.r.time[length(bd.r.time)-bp+1])/2)
	}
	else
	{
		# We must choose one of them, choose blindly
		bd.f.time <- bd.time;
	}

	bd.f.value <- c(0)
	for (t in 2:(length(bd.f.time)-1))
	{
		# Select the 5 samples that are closer to bd.f.time[t]
		# First calculate distance to bd.f.time[t]
		r_df_o <- df_o
		for (i in 1:nrow(r_df_o))
			r_df_o[i,1] <- abs (r_df_o[i,1] - bd.f.time[t])
		# Now sort wrt to time
		r_df_o <- r_df_o[order(r_df_o$time),]
		# Get the 5 top of the data
		r_df_o.head <- head(r_df_o, n = 5)

		# Calculate the mean value
		if (nrow(r_df_o.head) > 0)
		{
			value <- 0
			for (i in 1:nrow(r_df_o.head))
				value <- value + r_df_o.head[i, 2]
			value <- value / nrow(r_df_o.head)
		}
		else
		{
			value <- 0
		}
		bd.f.value <- c(bd.f.value, value)

		if (DEBUG)
		{
			print ("t")
			print (t)
			print (bd.f.time[t])
			print (r_df_o)
			print (r_df_o.head)
		}
	}
	bd.f.value <- c(bd.f.value, 1)

	# Prepare the plot!
	filename <- sprintf ("%s.%f.%s.%d.png", file, H, counter, group)
	png (filename, width=1600, height=1200)
	str1 <- sprintf ("h=%f", H)
	str2 <- sprintf ("Accumulated %s", counter)
	par(mar=c(5,4,4,5)+.1, cex=1.33)
	plot (df_o$value ~ df_o$time, main = file, sub = str1, xlab="Time", ylab=str2, col="black")

	for (t in 1:length(bd.time))
		abline (v = bd.time[t], col = "gray70", lwd=2)
	for (t in 1:length(bd.r.time))
		abline (v = bd.r.time[t], col = "gray70", lwd=3)
	for (t in 1:length(bd.f.time))
		abline (v = bd.f.time[t], col = "gray50", lwd=2)
	for (t in 1:length(bd.f.value))
		abline (h = bd.f.value[t], col = "gray50", lwd=3)

	print ("FINAL-RESULTS")
	print (length(bd.time))
	for (k in 1:(length(bd.f.time)))
		print (bd.f.time[k])

	for (k in 2:length(bd.f.time))
	{
		slope <- ((bd.f.value[k] - bd.f.value[k-1]) / (bd.f.time[k] - bd.f.time[k-1]))
		print (slope)
		lines (c(bd.f.time[k-1], bd.f.time[k]), c(bd.f.value[k-1], bd.f.value[k]), lwd=5, col = k-1)
	}

	dev.off()

} # main


# Example of usage
#DEBUG <- FALSE
#FILE <- "folding.R.20482"
#group <- 1
#H <- 0.01
#NSTEPS <- 0
#MAX_ERROR <- 0.002
#COUNTER <- "ctr"
#NCORES <- 4
#main (FILE, H, NSTEPS, MAX_ERROR, COUNTER, group, NCORES)


