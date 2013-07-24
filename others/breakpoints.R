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
		dlm <- lm (sub$value ~ sub$time, data = sub)
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
		dlm <- lm (sub$value ~ sub$time, data = sub)
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
		sub_df_o <- sub[order(sub$time),]

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
		real <- sub$value[i]
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


#
# MAIN code section
#

main <- function (file, H, nsteps, max_error, counter, group)
{
	message <- sprintf ("Working on file %s, H = %f, counter = %s, group = %d", file, H, counter, group)
	print (message)

	# Prepare my palette (based on rainbow(X) but excluding dark blue and grayscale)
	mypalette <- c("#FF0000FF","#FF8000FF","#FFFF00FF","#80FF00FF","#00FFFFFF","#009FFFFF","#8000FFFF","#FF00BFFF")
	palette(mypalette)

	# Read file
	df <- read.csv (file = file, head = TRUE, sep=";")

	df_o <- df[order(df$time),]

	print (df_o)

	# Actual computation!
	if (nrow(df_o) > 20)
	{
		H <- as.integer (max(nrow(df_o) / 20, 10))
		bp.counter <- breakpoints (df_o$value ~ df_o$time, h = H)

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
	filename <- sprintf ("%s.%f.step.original.%s.%d.png", file, H, counter, group)
	png (filename, width=1024, height=768)
	str1 <- sprintf ("h=%f", H)
	str2 <- sprintf ("Accumulated %s", counter)
	par(mar=c(5,4,4,5)+.1, cex=1.33)
	plot (df_o$value ~ df_o$time, main = file, sub = str1, xlab="Time (in ns)", ylab=str2, col="black")
	maxslope <- 0
	for (k in 2:length(bd.time))
	{
		slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_o, color = k-1, text = DEBUG)
		maxslope <- max(maxslope, slope)
	}

	maxslope <- 10
	par(new=TRUE)
	plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
	for (k in 2:length(bd.time))
		plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_o)
	axis(4)
	str3 <- sprintf ("%s / ns", counter)
	mtext(str3,side=4,line=3, cex=1.33)
#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
	dev.off()

	new.bd.time <- intersectionpoints (bd = bd.time, data = df_o)

	if (DEBUG)
	{
		print ("old bd.time (correct)")
		print (bd.time)
		print ("new bd.time (correct)")
		print (new.bd.time)
	}

	errors <- linearerrors (bd = new.bd.time, data = df_o)

	bd.time <- new.bd.time

	# Prepare the plot!
	filename <- sprintf ("%s.%f.step.corrected.%s.%d.png", file, H, counter, group)
	png (filename, width=1024, height=768)
	str1 <- sprintf ("h=%f", H)
	str2 <- sprintf ("Accumulated %s", counter)
	par(mar=c(5,4,4,5)+.1, cex=1.33)
	plot (df_o$value ~ df_o$time, main = file, sub = str1, xlab="Time (in ns)", ylab=str2, col="black")
	maxslope <- 0
	for (k in 2:length(bd.time))
	{
		slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_o, color = k-1, text = DEBUG)
		maxslope <- max(maxslope, slope)
	}
	par(new=TRUE)
	maxslope <- 10
	plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
	for (k in 2:length(bd.time))
		plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_o)
	axis(4)
	str3 <- sprintf ("%s / ns", counter)
	mtext(str3,side=4,line=3, cex=1.33)
#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
	dev.off()

	step <- 1
	while (step <= nsteps)
	{
		str <- sprintf ("Step %d", step)
		print (str)

		if (length(which(errors > max_error)) == 0)
			break

		new.bd.time <- c(0)

		for (e in 1:length(errors))
		{
			if (DEBUG)
			{
				message <- sprintf ("Error %d/%d is %f", e, length(errors), errors[e])
				print (message)
			}

			if (errors[e] > max_error) 
			{
				sub <- subset(df_o, df_o$time >= bd.time[e] & df_o$time < bd.time[e+1])
				sub_df_o <- sub[order(sub$time),]

				if (nrow(sub_df_o) > 20)
				{
					H <- as.integer (max(nrow(sub_df_o) / 20, 10))
					bp.counter <- tryCatch ({
						# Instead of bd.time <- c (0, breakdates (bp.counter), 1)
						breakpoints (sub_df_o$value ~ sub_df_o$time, h=H)
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

		new.bd.time <- intersectionpoints (bd = bd.time, data = df_o)

		if (DEBUG)
		{
			print ("old bd.time (correct)")
			print (bd.time)
			print ("new bd.time (correct)")
			print (new.bd.time)
		}

		bd.time <- new.bd.time

		# Prepare the plot!
		filename <- sprintf ("%s.%f.step.%d.%s.%d.png", file, H, step, counter, group)
		png (filename, width=1024, height=768)
		str1 <- sprintf ("h=%f", H)
		str2 <- sprintf ("Accumulated %s", counter)
		par(mar=c(5,4,4,5)+.1, cex=1.33)
		plot (df_o$value ~ df_o$time, main = file, sub=str1, xlab="Time (in ns)", ylab=str2, col="black")
		maxslope <- 0
		for (k in 2:length(bd.time))
		{
			slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_o, color = k-1, text = DEBUG)
			maxslope <- max(maxslope, slope)
		}
		par(new=TRUE)
		maxslope <- 10
		plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
		for (k in 2:length(bd.time))
			plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_o)
		axis(4)
		str3 <- sprintf ("%s / ns", counter)
		mtext(str3,side=4,line=3, cex=1.33)
		#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
		dev.off()

		new.bd.time <- fuse (bd = bd.time, data = df_o, maxerror = max_error)

		if (DEBUG)
		{
			print ("old bd.time (fuse)")
			print (bd.time)
			print ("new bd.time (fuse)")
			print (new.bd.time)
		}

		bd.time <- new.bd.time

		new.bd.time <- intersectionpoints (bd = bd.time, data = df_o)

		if (DEBUG)
		{
			print ("old bd.time (correct2)")
			print (bd.time)
			print ("new bd.time (correct2)")
			print (new.bd.time)
		}

		bd.time <- new.bd.time

		errors <- linearerrors (bd = bd.time, data = df_o)

		# Prepare the plot!
		filename <- sprintf ("%s.%f.step.%d.fused.%s.%d.png", file, H, step, counter, group)
		png (filename, width=1024, height=768)
		str1 <- sprintf ("h=%f", H)
		str2 <- sprintf ("Accumulated %s", counter)
		par(mar=c(5,4,4,5)+.1, cex=1.33)
		plot (df_o$value ~ df_o$time, main = file, sub=str1, xlab="Time (in ns)", ylab=str2, col="black")
		maxslope <- 0
		for (k in 2:length(bd.time))
		{
			slope <- plotsegment (from = bd.time[k-1], to = bd.time[k], data = df_o, color = k-1, text = DEBUG)
			maxslope <- max(maxslope, slope)
		}
		par(new=TRUE)
		maxslope <- 10
		plot(c(0), c(0), type = "l", axes = FALSE, bty = "n", xlab = "", ylab = "", ylim=c(0,ceiling(maxslope)), xlim=c(0,df_o$time[nrow(df_o)]))
		for (k in 2:length(bd.time))
			plotslopesegment (from = bd.time[k-1], to = bd.time[k], data = df_o)
		axis(4)
		str3 <- sprintf ("%s / ns", counter)
		mtext(str3,side=4,line=3, cex=1.33)
		#	legend("topleft",col=c("black","blue"),lty=c(0,1),legend=c("samples","counter rate"), pch=c(1,NA), lwd=c(1,3), box.lwd=0)
		dev.off()

		step <- step+1
	}

	print ("FINAL-RESULTS")
	print (length(bd.time))
	for (k in 1:(length(bd.time)-1))
		print (bd.time[k])
	print (1.0)

	for (k in 2:length(bd.time))
	{
		l <- linear_ndbg (from = bd.time[k-1], to = bd.time[k], data = df_o)
		print (l[2])
	}

} # main


# Example of usage
#DEBUG <- FALSE
#FILE <- "folding.R.20482"
#group <- 1
#H <- 0.01
#NSTEPS <- 0
#MAX_ERROR <- 0.002
#COUNTER <- "ctr"
#main (FILE, H, NSTEPS, MAX_ERROR, COUNTER, group)


