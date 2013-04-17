require (strucchange)

crosspoint <- function (slope1, intercept1, slope2, intercept2)
{
	if (DEBUG)
	{
		str <- sprintf ("crosspoint (%f, %f, %f, %f)", slope1, intercept1, slope2, intercept2)
		print (str)
	}

	if (!is.na(slope1) && !is.na(slope2) && slope1 != slope2)
	{
		temporal <- (intercept2-intercept1)/(slope1-slope2)
		result <- c(temporal, temporal*slope1+intercept1)
	}
	else
		result <- c(NA, NA)

	names(result) <- NULL

	if (DEBUG)
	{
		print (result)
	}

	return (result)
}

linear_ndbg <- function (from, to, data)
{
	sub <- subset(data, data$time >= from & data$time < to)
	if (nrow(sub) > 1)
	{
		dlm <- lm (sub$counter ~ sub$time, data = sub)
		sdlm <- summary(dlm)
		result <- c(coef(dlm)[1], coef(dlm)[2], coef(sdlm)[4], 0)
	}
	else
		result <- c(0, 0, 0, 0)

	names(result) <- NULL

	return (result)
}


linear <- function (from, to, data)
{
	if (DEBUG)
	{
		str <- sprintf ("linear (%f, %f)", from, to)
		print (str)
	}

	sub <- subset(data, data$time >= from & data$time < to)
	if (nrow(sub) > 1)
	{
		dlm <- lm (sub$counter ~ sub$time, data = sub)
		sdlm <- summary(dlm)
		result <- c(coef(dlm)[1], coef(dlm)[2], coef(sdlm)[4], 0)
	}
	else
		result <- c(0, 0, 0, 0)

	names(result) <- NULL

	return (result)
}

linearerrors <- function (bd, data)
{
	if (DEBUG)
	{
		print ("linearerrors")
	}

	errors <- c()
	for (s in 2:length(bd))
	{
		params <- linear (from = bd[s-1], to = bd[s], data = data)
		errors <- c(errors, params[3])
	}

	if (DEBUG)
	{
		print ("linearerrors results:")
		print (errors)
	}

	return (errors)
}

intersectionpoints <- function (bd, data)
{
	if (DEBUG)
	{
		str <- sprintf ("intersectionpoints")
		print (str)
	}

	res <- c(0)
	for (s in 2:length(bd))
	{
		if (s > 2)
			preparams <- params

		sub <- subset(data, data$time >= bd[s-1] & data$time < bd[s])
		#sub_df_s <- sub[sample(nrow(sub), size=min(NSAMPLES,nrow(sub))), ]
		sub_df_s <- sub[c(1:min(NSAMPLES,nrow(sub))),]
		sub_df_o <- sub_df_s[order(sub_df_s$time),]

		params <- linear (from = bd[s-1], to = bd[s], data = sub_df_o)
		if (s > 2)
		{
			cpoint <- crosspoint (preparams[2], preparams[1], params[2], params[1])

			if (!is.na(cpoint[1]) && !is.na(cpoint[2]) && cpoint[1] >= 0 && cpoint[2] >= 0 && cpoint[1] < bd[s])
			{
				params1 <- linear (from = bd[s-1], to = cpoint[1], data = sub_df_o)
				params2 <- linear (from = cpoint[1], to = bd[s], data = sub_df_o)

				#print (params1[2])
				#print (params2[2])
				#print (cpoint[1])

				if (is.na(params1[2]) || is.na(params2[2]))
				{
					res <- c(res, bd[s-1])
				}
				else if (params1[2] <= 0 || params2[2] <= 0)
				{
					res <- c(res, bd[s-1])
				}
				else if (params1[2] > 0 && params2[2] > 0 && cpoint[1] > res[length(res)])
				{
					res <- c(res, cpoint[1])
				}
				else
				{
					if (bd[s-1] > res[length(res)])
						res <- c(res, bd[s-1])
				}
			}
			else
			{
				if (bd[s-1] > res[length(res)])
					res <- c(res, bd[s-1])
			}
		}
	}
	res <- c(res, max(sub$time))

	if (DEBUG)
	{
		str <- sprintf ("END intersectionpoints")
		print (str)
	}

	return (res)
}

plotslopesegment <- function (from, to, data, slope)
{
	if (DEBUG)
	{
		str <- sprintf ("plotslopesegment (%f, %f)", from, to)
		print (str)
	}

	l <- linear (from, to, data = data)
	a <- l[1]
	b <- l[2]
	stderr <- l[3]
	stderrp <- l[4]

	lines (c(from, to), c(b, b), lwd=5, col="blue")
}

plotsegment <- function (from, to, data, color, text)
{

	if (DEBUG)
	{
		str <- sprintf ("plotsegment (%f, %f)", from, to)
		print (str)
	}

	l <- linear (from, to, data = data)
	a <- l[1]
	b <- l[2]
	stderr <- l[3]
	stderrp <- l[4]

	x_vals <- c(from, to)
	y_vals <- c(a+b*from,a+b*to)
	lines (x_vals,y_vals,lwd=5,col=color)
	if (FALSE) # text
	{
		str <- sprintf ("y = %.2f*X%+.e2 (e1=%.2e)", b, a, stderr)
		text (x=(x_vals[2]-x_vals[1])/2+x_vals[1], y=0, labels=str, col=color, srt=90, adj = c(0,0))
	}

	return (b);
}

checkmyerror <- function (l, from, to, data)
{
	error <- 0
	sub <- subset(data, data$time >= from & data$time < to)
	for (i in 1:nrow(sub))
	{
		predicted <- l[1] + l[2]*sub$time[i]
		real <- sub$counter[i]
		error <- error + abs (real - predicted)	
	}

	res <- c(nrow(sub),error)

	return (res)
}

fuse <- function (bd, data, maxerror)
{
	if (DEBUG)
	{
		print ("fuse")
		print ("bd")
		print (bd)
	}

	bd_result <- bd

	fused <- TRUE
	while (fused && length(bd_result) > 2)
	{
		fused <- F

		for (s in 2:(length(bd_result)-1))
		{
			lorig1 <- linear (from = bd_result[s-1], to = bd_result[s], data = data)
			lorig2 <- linear (from = bd_result[s], to = bd_result[s+1], data = data)
			l <- linear (from = bd_result[s-1], to = bd_result[s+1], data = data)

			e1 <- checkmyerror (l = lorig1, from = bd_result[s-1], to = bd_result[s], data = data)
			e2 <- checkmyerror (l = lorig2, from = bd_result[s], to = bd_result[s+1], data = data)
			e <- checkmyerror (l = l, from = bd_result[s-1], to = bd_result[s+1], data = data)

			if (DEBUG)
			{
				str <- sprintf ("error(lorig1)=%f error(lorig2)=%f error(l)=%f", lorig1[3], lorig2[3], l[3])
				print (str)
				str <- sprintf ("error(lorig1)=%e error(lorig2)=%e (sum=%e) error(l)=%e", e1[1], e2[1], e1[1]+e2[1], e[1])
				print (str)
			}


			str <- sprintf ("e1[1] = %d e2[1] = %d", e1[1], e2[1])
			print (str)

			if (is.na(e1[2]) || is.na(e2[2]) || e1[1] == 0 || e2[1] == 0)
			{
				bd_result <- c(bd_result[1:(s-1)], bd_result[(s+1):length(bd_result)])
				fused <- TRUE
				break
			}

#			if (!is.na(l[3]) && !is.na(lorig1[3]) && !is.na(lorig2[3]) &&
#           l[3] < min(lorig1[3],lorig2[3])) # l[3] < min(lorig1[3],lorig2[3])) #  && l[3] < maxerror)

			if (e[2] <= 1.1*(e1[2]+e2[2]) )
			{
				if (DEBUG)
				{
					str <- sprintf ("Fusion of s=%d [%f - %f]", s, bd_result[s-1], bd_result[s+1])
					print (str)
				}
				bd_result <- c(bd_result[1:(s-1)], bd_result[(s+1):length(bd_result)])
				fused <- TRUE
				break
			}
			else
			{
				if (DEBUG)
				{
					str <- sprintf ("Do not fuse [%f - %f] (e1 = %f, e2 = %f, min(e1,e2)=%f new=%f)", bd_result[s-1], bd_result[s+1], lorig1[3], lorig2[3], min(lorig1[3],lorig2[3]), l[3])
					print (str)
				}
			}
		}
	}

	return (bd_result)
}

removeoutliersinstances <- function (data)
{
	maxinstance <- max(data$instance)
	maxtimes <- c()
	for (i in 1:maxinstance)
	{
		sub <- subset(data, data$instance == i)
		if (nrow(sub) > 0)
			maxtimes <- c(maxtimes, max(sub$time))
		else
			maxtimes <- c(maxtimes, NA)
	}

	y <- quantile(maxtimes, na.rm = TRUE)

	maxtimesQ3 <- c()
	for (i in 1:maxinstance)
	{
		sub <- subset(data, data$instance == i)
		if (nrow(sub) > 0 && max(sub$time) <= y[3])
			maxtimesQ3 <- c(maxtimesQ3, max(sub$time))
		else
			maxtimesQ3 <- c(maxtimesQ3, NA)
	}

	mean <- mean (maxtimesQ3, na.rm = TRUE)
	stdev <- sd(maxtimesQ3, na.rm = TRUE)
	Ub <- mean+1.25*stdev
	Lb <- mean-1.25*stdev

	#str <- sprintf ("mean = %f stdev = %f RANGE = [%f..%f]", mean, stdev, Lb, Ub)
	#print (str)

	removeinstances <- c()
	for (i in 1:maxinstance)
		if (!is.na(maxtimes[i]))
		{
			# Outside limits of mean +/- X * stdev ?
			if (!(maxtimes[i] > Lb && maxtimes[i] < Ub))
			{
				#str <- sprintf ("Removing instance %d which has max time %f", i, maxtimes[i])
				#print (str)
				removeinstances <- c(removeinstances, i)
			}
		}

	new_data <- data
	if (length(removeinstances) > 0)
		for (i in 1:length(removeinstances))
			new_data <- subset (new_data, new_data$instance != removeinstances[i])

	# Should we remove the end points?
	new_data <- subset (new_data, new_data$rtime < 1)

	return (new_data)
}

#
# MAIN code section
#

main <- function (COUNTER, file, H, NSTEPS, MAX_ERROR, NSAMPLES)
{
	message <- sprintf ("Working on file %s, H = %f, counter = %s", file, H, COUNTER)
	print (message)

	# Prepare my palette (based on rainbow(X) but excluding dark blue and grayscale)
	mypalette <- c("#FF0000FF","#FF8000FF","#FFFF00FF","#80FF00FF","#00FFFFFF","#009FFFFF","#8000FFFF","#FF00BFFF")
	palette(mypalette)

	# Read file
	inp <- readLines(file)
	df <- read.table(textConnection(inp))
	names(df) <- c("cid","rtime","rcounter","instance","time","counter")

	df <- removeoutliersinstances (df)
	
	df_counter <- df[df[,"cid"]==COUNTER,]

	# df<-df[1:1000,]
	#df_s <- df_counter[sample(nrow(df_counter), size=min(NSAMPLES,nrow(df_counter))), ]
	df_s <- df_counter[c(1:min(NSAMPLES,nrow(df_counter))),]
	df_o <- df_s[order(df_s$time),]
	# bp.counter <- breakpoints (df$counter ~ df$time, h = 0.05, breaks = 10)

	# Actual computation!
	if (nrow(df_o) > 20)
	{
		H <- as.integer (max(nrow(df_o) / 20, 10))
		bp.counter <- breakpoints (df_o$counter ~ df_o$time, h = H)

		if (DEBUG)
			summary (bp.counter)

		# Instead of bd.time <- c (0, breakdates (bp.counter), 1)
		bpoints <- bp.counter[[1]]
		bd.time <- c(0)
		if (!is.na(bpoints[1]))
		{
			for (bp in 1:length(bpoints))
				bd.time <- c (bd.time, df_o$time[bpoints[bp]])
		}
		bd.time <- c(bd.time, df_o$time[nrow(df_o)])
	}
	else
	{
		bd.time <- c (0, df_o$time[nrow(df_o)])
	}

	# Prepare the plot!
	filename <- sprintf ("%s.%f.step.original.%s.png", file,H,COUNTER)
	png (filename, width=1024, height=768)
	str1 <- sprintf ("h=%f", H)
	str2 <- sprintf ("Accumulated %s", COUNTER)
	par(mar=c(5,4,4,5)+.1, cex=1.33)
	plot (df_counter$counter ~ df_counter$time, main = file, sub = str1, xlab="Time (in ns)", ylab=str2, col="black")
	maxslope <- 0
	for (k in 2:length(bd.time))
	{
		slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_counter, color = k-1, text = DEBUG)
		maxslope <- max(maxslope, slope)
	}

	maxslope <- 10
	par(new=TRUE)
	plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
	for (k in 2:length(bd.time))
		plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_counter)
	axis(4)
	str3 <- sprintf ("%s / ns", COUNTER)
	mtext(str3,side=4,line=3, cex=1.33)
#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
	dev.off()

	new.bd.time <- intersectionpoints (bd = bd.time, data = df_counter)

	if (DEBUG)
	{
		print ("old bd.time (correct)")
		print (bd.time)
		print ("new bd.time (correct)")
		print (new.bd.time)
	}

	errors <- linearerrors (bd = new.bd.time, data = df_counter)

	bd.time <- new.bd.time

	# Prepare the plot!
	filename <- sprintf ("%s.%f.step.corrected.%s.png", file,H,COUNTER)
	png (filename, width=1024, height=768)
	str1 <- sprintf ("h=%f", H)
	str2 <- sprintf ("Accumulated %s", COUNTER)
	par(mar=c(5,4,4,5)+.1, cex=1.33)
	plot (df_counter$counter ~ df_counter$time, main = file, sub = str1, xlab="Time (in ns)", ylab=str2, col="black")
	maxslope <- 0
	for (k in 2:length(bd.time))
	{
		slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_counter, color = k-1, text = DEBUG)
		maxslope <- max(maxslope, slope)
	}
	par(new=TRUE)
	maxslope <- 10
	plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
	for (k in 2:length(bd.time))
		plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_counter)
	axis(4)
	str3 <- sprintf ("%s / ns", COUNTER)
	mtext(str3,side=4,line=3, cex=1.33)
#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
	dev.off()

	step <- 1
	while (step <= NSTEPS)
	{
		str <- sprintf ("Step %d", step)
		print (str)

		if (length(which(errors > MAX_ERROR)) == 0)
			break

		new.bd.time <- c(0)

		for (e in 1:length(errors))
		{
			if (DEBUG)
			{
				message <- sprintf ("Error %d/%d is %f", e, length(errors), errors[e])
				print (message)
			}

			if (errors[e] > MAX_ERROR) 
			{
				sub <- subset(df_counter, df_counter$time >= bd.time[e] & df_counter$time < bd.time[e+1])
				sub_df_s <- sub[c(1:min(NSAMPLES,nrow(sub))),]
				sub_df_o <- sub_df_s[order(sub_df_s$time),]

				if (nrow(sub_df_o) > 20)
				{
					H <- as.integer (max(nrow(sub_df_o) / 20, 10))
					bp.counter <- tryCatch ({
						# Instead of bd.time <- c (0, breakdates (bp.counter), 1)
						breakpoints (sub_df_o$counter ~ sub_df_o$time, h=H)
					}, error = function(e){
						return (NA)
					})
					#print (bp.counter)
					#summary (bp.counter)

					bpoints <- bp.counter[[1]]
					bd_tmp <- c()
					if (!is.na(bpoints[1]))
					{
						for (bp in 1:length(bpoints))
							bd_tmp <- c (bd_tmp, sub_df_o$time[bpoints[bp]])
					}
					bd_tmp <- c(bd_tmp, bd.time[e+1])
					new.bd.time <- c(new.bd.time, bd_tmp)
				}
				else
				{
					new.bd.time <- c(new.bd.time, bd.time[e+1])
				}
			}
			else
			{
				new.bd.time <- c(new.bd.time, bd.time[e+1])
			}
		}

		if (DEBUG)
		{
			print ("old bd.time (breakpoints)")
			print (bd.time)
			print ("new bd.time (breakpoints)")
			print (new.bd.time)
		}

		bd.time <- new.bd.time

		new.bd.time <- intersectionpoints (bd = bd.time, data = df_counter)

		if (DEBUG)
		{
			print ("old bd.time (correct)")
			print (bd.time)
			print ("new bd.time (correct)")
			print (new.bd.time)
		}

		bd.time <- new.bd.time

		# Prepare the plot!
		filename <- sprintf ("%s.%f.step.%d.%s.png", file, H, step, COUNTER)
		png (filename, width=1024, height=768)
		str1 <- sprintf ("h=%f", H)
		str2 <- sprintf ("Accumulated %s", COUNTER)
		par(mar=c(5,4,4,5)+.1, cex=1.33)
		plot (df_counter$counter ~ df_counter$time, main = file, sub=str1, xlab="Time (in ns)", ylab=str2, col="black")
		maxslope <- 0
		for (k in 2:length(bd.time))
		{
			slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_counter, color = k-1, text = DEBUG)
			maxslope <- max(maxslope, slope)
		}
		par(new=TRUE)
		maxslope <- 10
		plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
		for (k in 2:length(bd.time))
			plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_counter)
		axis(4)
		str3 <- sprintf ("%s / ns", COUNTER)
		mtext(str3,side=4,line=3, cex=1.33)
		#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
		dev.off()

		new.bd.time <- fuse (bd = bd.time, data = df_counter, maxerror = MAX_ERROR)

		if (DEBUG)
		{
			print ("old bd.time (fuse)")
			print (bd.time)
			print ("new bd.time (fuse)")
			print (new.bd.time)
		}

		bd.time <- new.bd.time

		new.bd.time <- intersectionpoints (bd = bd.time, data = df_counter)

		if (DEBUG)
		{
			print ("old bd.time (correct2)")
			print (bd.time)
			print ("new bd.time (correct2)")
			print (new.bd.time)
		}

		bd.time <- new.bd.time

		errors <- linearerrors (bd = bd.time, data = df_counter)

		# Prepare the plot!
		filename <- sprintf ("%s.%f.step.%d.fused.%s.png", file, H, step, COUNTER)
		png (filename, width=1024, height=768)
		str1 <- sprintf ("h=%f", H)
		str2 <- sprintf ("Accumulated %s", COUNTER)
		par(mar=c(5,4,4,5)+.1, cex=1.33)
		plot (df_counter$counter ~ df_counter$time, main = file, sub=str1, xlab="Time (in ns)", ylab=str2, col="black")
		maxslope <- 0
		for (k in 2:length(bd.time))
		{
			slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_counter, color = k-1, text = DEBUG)
			maxslope <- max(maxslope, slope)
		}
		par(new=TRUE)
		maxslope <- 10
		plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
		for (k in 2:length(bd.time))
			plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_counter)
		axis(4)
		str3 <- sprintf ("%s / ns", COUNTER)
		mtext(str3,side=4,line=3, cex=1.33)
		#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
		dev.off()

		step <- step+1
	}

	print ("FINAL-RESULTS")
	print (length(bd.time))
	for (k in 1:length(bd.time))
		print (bd.time[k])

	for (k in 2:length(bd.time))
	{
		l <- linear_ndbg (from = bd.time[k-1], to = bd.time[k], data = df_counter)
		print (l[2])
	}

} # main


# Example of usage

#files<-c("openmx.p1.32tasks.o1.task1.clustered.fused.extract.1.1.Cluster_1.0.points","cgpop.task1.clustered.fused.extract.1.1.Cluster_1.0.points","pmemd.pme.clustered.fused.extract.1.1.Cluster_1.0.points","rrrmhd_mpi.16tasks.ppc.clustered.fused.extract.1.1.Cluster_1.0.points")
#files<-c("pmemd.pme.clustered.fused.extract.1.1.Cluster_1.0.points")
#files<-c("rrrmhd_mpi.16tasks.ppc.clustered.fused.extract.1.1.Cluster_1.0.points")
#files<-c("openmx.p1.32tasks.o1.task1.clustered.fused.extract.1.1.Cluster_1.0.points")
#files<-c("cgpop.task1.clustered.fused.extract.1.1.Cluster_1.0.points")
#Hs<-c(0.1,0.075,0.05,0.025,0.01)
#Hs<-c(0.1)
#NSTEPS <- 1
#MAX_ERROR <- 0.002
#NSAMPLES <- 1000
#NSAMPLES <- 500
#COUNTER <- "PM_INST_CMPL"
#DEBUG <- T

# main (files, COUNTER, Hs, NSTEPS, MAX_ERROR, NSAMPLES)


