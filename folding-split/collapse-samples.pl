#!/usr/bin/perl

my $sample_UF_id = 32000000;
my $new_iteration_id = 123456;
my $UF_event_id = 60000019;
my $preserve_UF = 1;
my $total_tasks = 0;
my $SaveOnlyUFSamples = 0;
my @time_of_1st_iteration;
my @remaining_iterations;
my @delta_times;
my %UFroutines;

sub ParseParameters
{
	local (@params) = @_;
	local ($index_of_file);

	$index_of_file = 0;
	for $i (0 .. $#params)
	{
		if ($params[$i] eq "-saveufsamples")
		{
			$SaveOnlyUFSamples = 1;
		}
		else
		{
			$index_of_file = $i;
		}
	}

	return $index_of_file;
}

sub ParseParaverHeader
{
	local $prvheader = $_[0];

	#Paraver (17/05/2007 at 12:21):91877877150_ns:1(9):1:9(1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1),10
	@headerfields = split(/:/, $prvheader);
	$nodes = $headerfields[3];
	@taskspernode = $nodes =~ /.*\((.+)\)$/;
	@tasks = split (/,/,$taskspernode[0]);

	for ($i = 0; $i <= $#tasks; $i++)
	{
		$total_tasks += $tasks[$i];
	}
}

sub GatherUFRoutines
{
	local $paraver_line = $_[0];

	if ($paraver_line =~ /^2.*/)
	{
		# 2:7:1:7:1:129632750:50000001:6:42000050:7314:42000059:26932:42000000:324
		($event,$cpu,$ptask,$task,$thread,$time,@events) = split(/:/,$paraver_line);
		for ($i = 0; $i <= $#events; $i = $i + 2)
		{
			if ($events[$i] == $UF_event_id)
			{
				$UFroutines[$events[$i+1]] = 1;
			}
		}
	}
}

sub ParseParaverLine
{
	local $paraver_line = $_[0];
	local $writeline = 0;

	if ($paraver_line =~ /^2.*/)
	{
		# 2:7:1:7:1:129632750:50000001:6:42000050:7314:42000059:26932:42000000:324
		($event,$cpu,$ptask,$task,$thread,$time,@events) = split(/:/,$paraver_line);

		$new_time = $time + $delta_times[$task];

		for ($i = 0; $i <= $#events; $i = $i + 2)
		{
			if ($iteration_id[$task] != 0)
			{
				if (($events[$i] >= $sample_UF_id && $events[$i] <= 100+$sample_UF_id) && $writeline == 0 && ($UFroutines[$events[1+$i]] == 1 || !$SaveOnlyUFSamples))
				{
					print $event.':'.$cpu.':'.$ptask.':'.$task.':'.$thread.':'.$new_time.':'.$events[$i].":".$events[1+$i];
					$writeline = 1;
				}
				elsif (($events[$i] >= $sample_UF_id && $events[$i] <= 100+$sample_UF_id) && $writeline != 0 && ($UFroutines[$events[1+$i]] == 1 || !$SaveOnlyUFSamples))
				{
					print ':'.$events[$i].":".$events[1+$i];
				}
			}
			if ($events[$i] == $new_iteration_id)
			{
				# If it's the first iteration, store it's time
				if ($events[1+$i] == 1)
				{
					$time_of_1st_iteration[$task] = $time;
					$delta_times[$task] = 0;
				}
				else
				{
					if ($writeline != 0)
					{
						print ':'.$events[$i].":".$events[1+$i];
					}
					else
					{
						print $event.':'.$cpu.':'.$ptask.':'.$task.':'.$thread.':'.$new_time.':'.$events[$i].":".$events[1+$i];
						$writeline = 1;
					}
					$delta_times[$task] = -$time + $time_of_1st_iteration[$task];
				}

				$iteration_id[$task] = $events[1+$i]; 
			}
		}
		if ($writeline)
		{
			print "\n";
		}
	}
	if ($iteration_id[$task] <= 1)
	{
		if (!$writeline)
		{
			if ($paraver_line =~ /^1.*/)
			{
				# 1:7:1:7:1:1343337100:1343339650:1
				($event,$cpu,$ptask,$task,$thread,$time_1,$time_2,$state) = split(/:/,$paraver_line);
				print '1:'.$cpu.':'.$ptask.':'.$task.':'.$thread.':'.$time_1.':'.$time_2.":".$state."\n";
			}
			elsif ($paraver_line =~ /^2.*/)
			{
				# 2:7:1:7:1:129632750:50000001:6:42000050:7314:42000059:26932:42000000:324
				($event,$cpu,$ptask,$task,$thread,$time,@events) = split(/:/,$paraver_line);
				if ($#events == 1)
				{
					if ($events[0] < $sample_UF_id || $events[0] > 100+$sample_UF_id)
					{
						print $event.':'.$cpu.':'.$ptask.':'.$task.':'.$thread.':'.$time.':'.$events[0].':'.$events[1]."\n";
					}
				}
				else
				{
					print $event.':'.$cpu.':'.$ptask.':'.$task.':'.$thread.':'.$time;
					for ($i = 0; $i <= $#events; $i = $i + 2)
					{
						if ($events[$i] < $sample_UF_id || $events[$i] > 100+$sample_UF_id)
						{
							print ':'.$events[$i].':'.$events[1+$i];
						}
					}
				print "\n";
				}
			}
			elsif ($paraver_line =~ /^3.*/)
			{
				# 3:4:1:4:1:1128579100:1128584300:7:1:7:1:1121028650:1128840950:184960:4000
				($event,$cpu_s,$ptask_s,$task_s,$thread_s,$time_s_1,$time_s_2,$cpu_r,$ptask_r,$task_r,$thread_r,$time_r_1,$time_r_2,$size,$tag) = split(/:/,$paraver_line);
				print '3:'.$cpu_s.':'.$ptask_s.':'.$task_s.':'.$thread_s.':'.$time_s_1.':'.$time_s_2.":".$cpu_r.':'.$ptask_r.':'.$task_r.':'.$thread_r.':'.$time_r_1.':'.$time_r_2.":".$size.":".$tag."\n";
			}
			elsif ($paraver_line =~ /^c.*/)
			{
				print $paraver_line."\n";
			}
		}
	}
}

	$infile = &ParseParameters (@ARGV);
	if ($ARGV[$infile])
	{
		open TRACE, $ARGV[$infile] or die "Cannot open file $ARGV[$infile]";
	}
	else
	{
		die "You must pass a tracefile to parse";
	}

	if ($SaveOnlyUFSamples)
	{
		$line = <TRACE> and chop $line;
		while (!eof)
		{
			&GatherUFRoutines ($line);
			$line = <TRACE> and chop $line;
		}
		seek (TRACE, 0, 0);
	}

	$header = <TRACE>;
	chop $header;
	print $header."\n";
	&ParseParaverHeader ($header);

	for $i (0 .. $total_tasks)
	{
		$time_of_1st_iteration[$i] = 0;
		$iteration_id[$i] = 0;
		$remaining_iterations[$i] = $iterations_to_collapse;
		$delta_times[$i] = 0;
	}

	$line = <TRACE> and chop $line;
	while (!eof)
	{
		&ParseParaverLine ($line);
		$line = <TRACE> and chop $line;
	}
	close TRACE;
