#!/usr/bin/python

import wx
import os
import sys

class FoldingResultGroup:
	def __init__ (self, duration, mips, instances, phases):
		try:
			self.duration = float(duration)
			self.mips = float(mips)
			self.instances = int(instances)
			self.phases = int(phases)
		except ValueError:
			print "Error creating FoldingResultGroup"
			return

class FoldingResult:
	def __init__(self, name, counters, numgroups, metrics):
		self.name = name
		self.counters = counters.split (',')
		try:
			self.numgroups = int(numgroups)
		except ValueError:
			print "Error creating FoldingResult"
			return
		self.groupmetrics = []
		for metric in metrics.split (','):
			duration, mips, instances, phases = metric.split ('-');
			frg = FoldingResultGroup (duration, mips, instances, phases)
			self.groupmetrics.append (frg)

	def __hash__(self):
		return hash (self.name)

	def __eq__(self, other):
		return (self.name) == (other.name)

class wxFoldingViewerDialog(wx.Dialog):

	def __init__(self, foldingresults, filename, foldedobject, *args, **kw):
		super(wxFoldingViewerDialog, self).__init__(*args, **kw) 
		self.FoldingResults = foldingresults
		self.FilePrefix = filename[0:filename.rfind (".wxfolding")]
		self.FoldedObject = foldedobject
		self.InitUI()
		self.SetSize((600, 320))
        
	def InitUI(self):
		panel = wx.Panel (self)

		self.sizerv = wx.BoxSizer (wx.VERTICAL)

		# Region selector (and its groups)

		self.regionselector = wx.StaticBox (panel, -1, "Region selector")
		self.regionselectorszr = wx.StaticBoxSizer (self.regionselector, wx.HORIZONTAL)
		self.regions = []
		for key in self.FoldingResults:
			self.regions.append (key)
		self.regions.sort()
		self.rs_chooseregiontxt = wx.StaticText (panel, -1, "Choose region: ")
		self.rs_chooseregion = wx.Choice (panel, -1, choices = self.regions)
		self.rs_chooseregion.Bind (wx.EVT_CHOICE, self.OnChangeRegion )
		self.rs_chooseregion.SetSelection (0)
		self.rs_viewgroups = wx.Button (panel, 1, "View groups for the region");
		self.Bind (wx.EVT_BUTTON, self.OnViewGroups, id = 1)
		self.regionselectorszr.AddSpacer (10)
		self.regionselectorszr.Add (self.rs_chooseregiontxt, 0, wx.CENTER)
		self.regionselectorszr.AddSpacer (10)
		self.regionselectorszr.Add (self.rs_chooseregion)
		self.regionselectorszr.Add ((0, 0), 1, wx.EXPAND) # Take space to align next to the right
		self.regionselectorszr.Add (self.rs_viewgroups)
		self.regionselectorszr.AddSpacer (10)

		# Group & Counter selector

		self.groupcounterselector = wx.StaticBox (panel, -1, "Performance metrics")
		self.groupcounterselectorszr = wx.StaticBoxSizer (self.groupcounterselector, wx.HORIZONTAL)
		self.gc_choosegrouptxt = wx.StaticText (panel, -1, "Choose group: ")
		self.gc_choosegroup = wx.Choice (panel, -1, choices = [])
		self.gc_choosegroup.Bind (wx.EVT_CHOICE, self.OnChangeGroup )
		self.gc_group_duration = wx.StaticText (panel, -1, "")
		self.gc_group_mips = wx.StaticText (panel, -1, "")
		self.gc_group_instances = wx.StaticText (panel, -1, "")
		self.gc_group_phases = wx.StaticText (panel, -1, "")
		self.innercounterselector = wx.StaticBox (panel, -1, "Available counters")
		self.innercounterselectorszr = wx.StaticBoxSizer (self.innercounterselector, wx.VERTICAL)
		self.gc_lccounters = wx.ListCtrl(panel, -1, style = wx.LC_REPORT | wx.LC_NO_HEADER | wx.LC_SINGLE_SEL)
		self.gc_lccounters.Bind (wx.EVT_LIST_ITEM_ACTIVATED, self.OnViewCounter)
		self.gc_lccounters.InsertColumn(0, 'Counters')
		self.innercounterselectorszr.Add (self.gc_lccounters, 1, wx.EXPAND)
		self.gc_hsizer1 = wx.BoxSizer (wx.HORIZONTAL)
		self.gc_hsizer1.Add (self.gc_choosegrouptxt, 0, wx.CENTER)
		self.gc_hsizer1.AddSpacer (10)
		self.gc_hsizer1.Add (self.gc_choosegroup)
		self.gc_hsizer1.AddSpacer (10)
		self.gc_vsizer1 = wx.BoxSizer (wx.VERTICAL)
		self.gc_vsizer1.Add (self.gc_hsizer1)
		self.gc_vsizer1.AddSpacer (10)
		self.gc_vsizer1.Add (self.gc_group_duration)
		self.gc_vsizer1.AddSpacer (10)
		self.gc_vsizer1.Add (self.gc_group_mips)
		self.gc_vsizer1.AddSpacer (10)
		self.gc_vsizer1.Add (self.gc_group_instances)
		self.gc_vsizer1.AddSpacer (10)
		self.gc_vsizer1.Add (self.gc_group_phases)
		self.gc_vsizer1.AddSpacer (10)

		self.groupcounterselectorszr.AddSpacer (10)
		self.groupcounterselectorszr.Add (self.gc_vsizer1, 0, wx.CENTER)
		self.groupcounterselectorszr.AddSpacer (50)
		self.groupcounterselectorszr.Add (self.innercounterselectorszr, 3, wx.EXPAND)
		self.groupcounterselectorszr.AddSpacer (10)

		self.FeedGroupAndCounter (self.rs_chooseregion.GetStringSelection())

		# Quit button, to finish!

		self.quitBtn = wx.Button (panel, label="Quit")
		self.quitBtn.Bind (wx.EVT_BUTTON, self.OnQuit )
		self.quitszr = wx.BoxSizer (wx.HORIZONTAL)
		self.quitszr.Add (self.quitBtn, 0, wx.ALL, border = 10)

		# Pack all and show

		self.sizerv.AddSpacer (15)
		self.sizerv.Add (self.regionselectorszr, 0, wx.EXPAND, border = 5)
		self.sizerv.AddSpacer (15)
		self.sizerv.Add (self.groupcounterselectorszr, 0, wx.EXPAND, border = 5)
		self.sizerv.Add ((0, 0), 1, wx.EXPAND) # Take remaining vertical space 
		self.sizerv.Add (self.quitszr, 0, wx.CENTER, border = 5)
		self.sizerv.AddSpacer (15)

		panel.SetSizer (self.sizerv)
		self.Fit()

	# Handlers for buttons
	def OnQuit (self, event = None):
		self.EndModal (wx.ID_OK)

	def OnViewGroups (self, event = None):
		f = self.FilePrefix + "." + self.rs_chooseregion.GetStringSelection() \
		  + "." + self.FoldedObject + ".groups.gnuplot"
		command = "gnuplot -persist " + f
		print "Executing command: " + command
		if not os.system (command) == 0:
			print "Error while executing : " + command
			return

	def OnChangeRegion (self, event = None):
		r = self.rs_chooseregion.GetStringSelection()
		self.FeedGroupAndCounter (r)

	def FeedGroupAndCounter (self, region):
		fr = self.FoldingResults[region]
		self.gc_lccounters.DeleteAllItems()
		self.gc_lccounters.InsertStringItem (0, str("All"))
		for line in fr.counters:
			num_items = self.gc_lccounters.GetItemCount()
			self.gc_lccounters.InsertStringItem (num_items, str(line))
		self.gc_lccounters.SetColumnWidth (0, wx.LIST_AUTOSIZE)

		self.gc_choosegroup.Clear()
		for i in range (fr.numgroups):
			self.gc_choosegroup.Append (str (i+1))
		self.gc_choosegroup.SetSelection (0)
		self.FeedGroupInfo (region, 0)

	def OnChangeGroup (self, event = None):
		r = self.rs_chooseregion.GetStringSelection()
		n = self.gc_choosegroup.GetSelection()
		self.FeedGroupInfo (r, n)

	def FeedGroupInfo (self, region, ngroup):
		fr = self.FoldingResults[region]
		frg = fr.groupmetrics[ngroup]
		self.gc_group_duration.SetLabel ( "Average duration : " + str (frg.duration) + " ms" )
		if frg.mips > 0:
			self.gc_group_mips.SetLabel ("Average MIPS: " + str (frg.mips) )
		else:
			self.gc_group_mips.SetLabel ("Average MIPS: N/A")
		self.gc_group_instances.SetLabel ("No. of Instances: " + str (frg.instances) )
		self.gc_group_phases.SetLabel ("No. of Phases: " + str (frg.phases))

	def OnViewCounter(self, e):
		index = self.gc_lccounters.GetFocusedItem()
		if index > 0:
			counter = self.gc_lccounters.GetItem (index, 0).GetText()
		else:
			counter = "slopes"

		region = self.rs_chooseregion.GetStringSelection()
		group = self.gc_choosegroup.GetSelection() + 1

		f = self.FilePrefix + "." + region + "." + self.FoldedObject + "." + \
		  str(group) + "." + counter + ".gnuplot"

		command = "gnuplot -persist " + f
		print "Executing command: " + command
		if not os.system (command) == 0:
			print "Error while executing : " + command
			return

#
# wxFolding, main class, orchestrator for the rest
#

class wxFoldingViewer(wx.Frame):
	def __init__(self, dfile, *args, **kw):
		super(wxFoldingViewer, self).__init__(*args, **kw) 
		self.DataFile = dfile
		self.regions = {}
		self.InitUI()

	def InitUI(self):
		self.Show(False)

		lines = [line.strip() for line in open(self.DataFile)]
		useless, foldedobject = lines[1].split("=");
		ptask, task, thread = foldedobject.split (".");

		for line in lines[2:]:
				region, counters, numgroups, infogroups = line.split (';')
				fr = FoldingResult (region, counters, numgroups, infogroups)
				self.regions[region] = fr

		if os.path.dirname (self.DataFile):
			os.chdir (os.path.dirname(self.DataFile))

		dialog = wxFoldingViewerDialog (parent = self, foldingresults = self.regions,
		  filename = os.path.basename (self.DataFile), foldedobject = foldedobject,
		  title = "Folding results viewer")
		if not dialog.ShowModal() == wx.ID_OK:
			self.Destroy()
			return

		self.Destroy()


#
# MAIN
#

if __name__ == "__main__":

	if len (sys.argv) != 2:
		print 'Error! Command ' + sys.argv[0] + ' must be given the .wxfolding file';
		sys.exit (-1)
	elif len (sys.argv) == 2:
		if not sys.argv[1].endswith (".wxfolding"):
			print 'Error! Command ' + sys.argv[0] + ' must be given the .wxfolding file';
			sys.exit (-1)
		datafile = sys.argv[1]

	TMPDIR = os.getenv ("TMPDIR")
	if not TMPDIR:
		TMPDIR = "/tmp"
	FOLDING_HOME = os.getenv ("FOLDING_HOME")
	if FOLDING_HOME:
		ex = wx.App()
		f = wxFoldingViewer (parent = None, dfile = datafile)
		ex.MainLoop()
	else:
		print "FOLDING_HOME is not set!"

