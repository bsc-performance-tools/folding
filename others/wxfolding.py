#!/usr/bin/python

import wx
import os
import sys
import wxfolding_viewer

class wxFoldingEventChooser(wx.Dialog):
	def __init__(self, *args, **kw):
		super(wxFoldingEventChooser, self).__init__(*args, **kw) 
            
		self.InitUI()
		self.SetSize((480, 240))
		self.SetTitle("Choose folding event type")

		self.EventTypeChosen = None
		self.EventDescriptionChosen = None
        
	def InitUI(self):

		self.lc = wx.ListCtrl(self, -1, style=wx.LC_REPORT|wx.LC_SINGLE_SEL)
		self.lc.InsertColumn(0, 'Event type')
		self.lc.InsertColumn(1, 'Event type description')
		self.lc.SetColumnWidth(0, 160)
		self.lc.SetColumnWidth(1, 300)
		self.lc.Bind (wx.EVT_LIST_ITEM_ACTIVATED, self.OnChoose)
		self.lc.Bind (wx.EVT_LIST_ITEM_SELECTED, self.OnListClick)

		listfile = TMPDIR + "/folding-types." + str (os.getpid())

		lines = [line.strip() for line in open(listfile)]
		for line in lines:
			evttype, evtdescription = line.split (";")
			num_items = self.lc.GetItemCount()
			self.lc.InsertStringItem(num_items, str(evttype))
			self.lc.SetStringItem(num_items, 1, evtdescription)

		self.chooseBtn = wx.Button(self, label="Choose")
		self.chooseBtn.Disable()
		self.chooseBtn.Bind(wx.EVT_BUTTON, self.OnChoose )
		self.cancelBtn = wx.Button(self, label="Cancel")
		self.cancelBtn.Bind(wx.EVT_BUTTON, self.OnCancel )

		self.sizerh1 = wx.BoxSizer (wx.HORIZONTAL)
		self.sizerh1.Add (self.cancelBtn, border = 5)
		self.sizerh1.Add (self.chooseBtn, wx.LEFT | wx.BOTTOM, border = 5)

		self.sizerv = wx.BoxSizer (wx.VERTICAL)
		self.sizerv.Add (self.lc, 1, wx.EXPAND|wx.ALL, border = 5)
		self.sizerv.Add (self.sizerh1, 0, wx.CENTER, border = 3)

		self.SetSizerAndFit(self.sizerv)

		self.result = None

	def OnListClick(self, e):
		self.chooseBtn.Enable()

	def OnChoose(self, e):
		index = self.lc.GetFocusedItem()
		self.EventTypeChosen = self.lc.GetItem (index, 0).GetText()
		self.EventDescriptionChosen = self.lc.GetItem(index, 1).GetText()
		self.EndModal(wx.ID_OK)

	def OnCancel(self, e):
		self.EndModal(wx.ID_CANCEL)



class wxFoldingInputDialog(wx.Dialog):

	def __init__(self, tracefile, *args, **kw):
		super(wxFoldingInputDialog, self).__init__(*args, **kw) 
		self.TraceFile = tracefile
		self.EventType = None
		self.SourceDir = None
		self.WorkDir = os.getcwd()
		self.InitUI()
		self.SetSize((600, 240))

        
	def InitUI(self):
		self.sizerv = wx.BoxSizer (wx.VERTICAL)

		# Source code directory sizer
		self.btnChooseSourceDir = wx.Button (self, -1, "Choose source code location")
		self.btnChooseSourceDir.Bind (wx.EVT_BUTTON, self.OnChooseSourceDir )
		self.btnChooseSourceDir.Disable()
		self.txtsourceDir = wx.StaticText (self, -1, "(optional)")
		self.txtsourceDir.Disable()
		self.sizerh2 = wx.BoxSizer (wx.HORIZONTAL)
		self.sizerh2.Add (self.btnChooseSourceDir, 0, wx.EXPAND|wx.ALL, border = 10)
		self.sizerh2.Add (self.txtsourceDir, 1, wx.CENTER, border = 10)

		# PRV chooser sizer
		self.btnChoosePrv = wx.Button (self, -1, "Choose tracefile", size = self.btnChooseSourceDir.GetSize())
		self.btnChoosePrv.Bind (wx.EVT_BUTTON, self.OnChoosePrv )
		self.txtPrv = wx.StaticText (self, -1, "")
		self.sizerh1 = wx.BoxSizer (wx.HORIZONTAL)
		self.sizerh1.Add (self.btnChoosePrv, 0, wx.EXPAND|wx.ALL, border = 10)
		self.sizerh1.Add (self.txtPrv, 1, wx.CENTER, border = 10)

		# Event chooser sizer
		self.btnChooseEvent = wx.Button (self, -1, "Choose folding event", size = self.btnChooseSourceDir.GetSize())
		self.btnChooseEvent.Disable()
		self.btnChooseEvent.Bind (wx.EVT_BUTTON, self.OnChooseEvent)
		self.txtEventType = wx.StaticText (self, -1, "")
		self.txtEventType.Disable()
		self.sizerh3 = wx.BoxSizer (wx.HORIZONTAL)
		self.sizerh3.Add (self.btnChooseEvent, 0, wx.EXPAND|wx.ALL, border = 10)
		self.sizerh3.Add (self.txtEventType, 0, wx.CENTER, border = 10)

		# Working directory chooser
		self.btnChooseWorkDir = wx.Button (self, -1, "Choose working directory", size = self.btnChooseSourceDir.GetSize())
		self.btnChooseWorkDir.Bind (wx.EVT_BUTTON, self.OnChooseWorkDir )
		self.txtworkDir = wx.StaticText (self, -1, self.WorkDir )
		self.sizerh4 = wx.BoxSizer (wx.HORIZONTAL)
		self.sizerh4.Add (self.btnChooseWorkDir, 0, wx.EXPAND|wx.ALL, border = 10)
		self.sizerh4.Add (self.txtworkDir, 1, wx.CENTER, border = 10)

		self.continueBtn = wx.Button(self, label="Proceed >")
		self.continueBtn.Disable()
		self.continueBtn.Bind (wx.EVT_BUTTON, self.OnContinue )
		self.sizerh5 = wx.BoxSizer (wx.HORIZONTAL)
		self.sizerh5.Add (self.continueBtn, 0, wx.ALL, border = 10)

		# Form layout
		self.sizerv.Add (self.sizerh1, 0, wx.ALL, border = 1)
		self.sizerv.Add (self.sizerh2, 0, wx.ALL, border = 1)
		self.sizerv.Add (self.sizerh3, 0, wx.ALL, border = 1)
		self.sizerv.Add (self.sizerh4, 0, wx.ALL, border = 1)
		self.sizerv.AddSpacer (10)
		self.sizerv.Add (self.sizerh5, 0, wx.CENTER, border = 1)

		self.SetSizerAndFit(self.sizerv)

		# If tracefile is not none, just process it as a button call
		if self.TraceFile:
			self.ChoosePrv (self.TraceFile)

	# Auxiliary functions 
	def ChoosePrv (self, TraceFile):
		self.TraceFile = TraceFile
		if len(self.TraceFile) > 55:
			label = "..."+self.TraceFile[len(self.TraceFile)-55:len(self.TraceFile)]
		else:
			label = self.TraceFile;
		self.txtPrv.SetLabel ( label )
		self.btnChooseEvent.Enable()
		self.txtEventType.Enable()
		self.btnChooseSourceDir.Enable()
		self.txtsourceDir.SetLabel ("optional")
		self.txtsourceDir.Disable()
		self.txtEventType.SetLabel ("")
		self.SourceDir = None
		self.EventType = None

		# Generate the available types list
		command = FOLDING_HOME + "/bin/foldingtypes " + self.TraceFile + " > " + TMPDIR + "/folding-types." + str (os.getpid())
		print "Executing command: "+command
		if not os.system (command) == 0:
			print "Error while executing : " + command
			return

	# Handlers for buttons
	def OnContinue(self, event=None):
		self.EndModal (wx.ID_OK)

	def OnChoosePrv(self, event=None):
		dialog = wx.FileDialog (None, wildcard = "Paraver tracefiles (*.prv)|*.prv", style = wx.OPEN)
		if dialog.ShowModal() == wx.ID_OK:
			self.ChoosePrv (dialog.GetPath())
			self.Fit()
		dialog.Destroy()

	def OnChooseSourceDir(self, event=None):
		dialog = wx.DirDialog (None, "Choose a directory:", style = wx.DD_DEFAULT_STYLE)
		if dialog.ShowModal() == wx.ID_OK:
			self.SourceDir = dialog.GetPath()
			if len(self.SourceDir) > 55:
				label = "..."+self.SourceDir[len(self.SourceDir)-55:len(self.SourceDir)]
			else:
				label = self.SourceDir;
			self.txtsourceDir.SetLabel ( label )
			self.txtsourceDir.Enable()
			self.Fit()
		dialog.Destroy()

	def OnChooseWorkDir(self, event=None):
		dialog = wx.DirDialog (None, "Choose a directory:", style = wx.DD_DEFAULT_STYLE | wx.DD_NEW_DIR_BUTTON)
		if dialog.ShowModal() == wx.ID_OK:
			self.WorkDir = dialog.GetPath()
			if len(self.WorkDir) > 55:
				label = "..."+self.WorkDir[len(self.WorkDir)-55:len(self.WorkDir)]
			else:
				label = self.WorkDir;
			self.txtworkDir.SetLabel ( label )
			self.Fit()
		dialog.Destroy()

	def OnChooseEvent(self, event=None):
		dialog = wxFoldingEventChooser (None, title = "Choose event type")
		if dialog.ShowModal() == wx.ID_OK:
			self.EventType = int (dialog.EventTypeChosen)
			label = dialog.EventTypeChosen + " (" + dialog.EventDescriptionChosen + ")"
			if len(label) > 55:
				label = label[0:55];
			self.txtEventType.SetLabel ( label )
			self.continueBtn.Enable()
			self.Fit()
		dialog.Destroy()



class wxFoldingInterpolateKrigerDialog(wx.Dialog):

	def __init__(self, tracefile, *args, **kw):
		super(wxFoldingInterpolateKrigerDialog, self).__init__(*args, **kw) 
		self.TraceFile = os.path.basename (tracefile)
		self.InitUI()
		self.SetSize((700, 480))

	def InitUI(self):
		panel = wx.Panel (self)

		self.sizerv = wx.BoxSizer (wx.VERTICAL)

		# Data management frame

		self.boxdata = wx.StaticBox (panel, -1, "Data management")
		self.boxdataszr = wx.StaticBoxSizer (self.boxdata, wx.VERTICAL)
		self.d_useallobjects = wx.ToggleButton (panel, -1, "Use all objects to interpolate")
		self.d_useallobjects.SetValue (True)
		self.d_useallobjects.Bind (wx.EVT_TOGGLEBUTTON, self.OnChangeUseAllObjects )
		self.d_chooseobjecttxt = wx.StaticText (panel, -1, "Choose object:")
		self.d_chooseobjecttxt.Disable()
		objectsfile = self.TraceFile[0:self.TraceFile.rfind (".prv")] + ".objects"
		self.objects = [line.strip() for line in open(objectsfile)]
		self.objects.sort()
		self.d_chooseobject = wx.Choice (panel, -1, choices = self.objects)
		self.d_chooseobject.Disable()
		self.d_hsizer1 = wx.BoxSizer (wx.HORIZONTAL)
		self.d_hsizer1.Add (self.d_useallobjects, 0, wx.EXPAND|wx.ALL);
		self.d_hsizer1.AddSpacer (10)
		self.d_hsizer1.Add (self.d_chooseobjecttxt, 0, wx.CENTER)
		self.d_hsizer1.AddSpacer (5)
		self.d_hsizer1.Add (self.d_chooseobject, 0, wx.EXPAND|wx.ALL)
		self.d_splitgroups = wx.ToggleButton (panel, -1, "Split instances in groups")
		self.d_splitgroups.SetValue (True)
		self.d_limitsamples = wx.ToggleButton (panel, -1, "Limit samples")
		self.d_limitsamples.SetValue (False)
		self.d_limitsamples.Bind (wx.EVT_TOGGLEBUTTON, self.OnChangeLimitSamples )
		self.d_limitsamplesnum = wx.SpinCtrl (panel, -1)
		self.d_limitsamplesnum.SetRange (1, 10000)
		self.d_limitsamplesnum.SetValue (1000)
		self.d_limitsamplesnum.Disable()
		self.d_limitsamplesnumtxt = wx.StaticText (panel, -1, "# of samples:")
		self.d_limitsamplesnumtxt.Disable()
		self.d_hsizer2 = wx.BoxSizer (wx.HORIZONTAL)
		self.d_hsizer2.Add (self.d_limitsamples, 0, wx.EXPAND|wx.ALL);
		self.d_hsizer2.AddSpacer (5)
		self.d_hsizer2.Add (self.d_limitsamplesnumtxt, 0, wx.CENTER);
		self.d_hsizer2.AddSpacer (5)
		self.d_hsizer2.Add (self.d_limitsamplesnum, 0, wx.EXPAND|wx.ALL);
		self.d_useallregions = wx.ToggleButton (panel, -1, "All regions")
		self.d_useallregions.SetValue (True)
		self.d_useallregions.Bind (wx.EVT_TOGGLEBUTTON, self.OnChangeUseAllRegions )
		self.d_lcregions = wx.ListCtrl(panel, -1, style = wx.LC_REPORT | wx.LC_NO_HEADER)
		self.d_lcregions.InsertColumn(0, 'Regions')
		regionsfile = self.TraceFile[0:self.TraceFile.rfind (".prv")] + ".regions"
		self.regions = [line.strip() for line in open(regionsfile)]
		self.regions.sort()
		for line in self.regions:
			num_items = self.d_lcregions.GetItemCount()
			self.d_lcregions.InsertStringItem(num_items, str(line))
		self.d_lcregions.SetColumnWidth (0, wx.LIST_AUTOSIZE)
		self.d_lcregions.Disable()
		self.d_useallcounters = wx.ToggleButton (panel, -1, "All counters")
		self.d_useallcounters.SetValue (True)
		self.d_useallcounters.Bind (wx.EVT_TOGGLEBUTTON, self.OnChangeUseAllCounters )
		self.d_lccounters = wx.ListCtrl(panel, -1, style = wx.LC_REPORT | wx.LC_NO_HEADER)
		self.d_lccounters.InsertColumn(0, 'Counters')
		countersfile = self.TraceFile[0:self.TraceFile.rfind (".prv")] + ".counters"
		self.counters = [line.strip() for line in open(countersfile)]
		self.counters.sort()
		for line in self.counters:
			num_items = self.d_lccounters.GetItemCount()
			self.d_lccounters.InsertStringItem(num_items, str(line))
		self.d_lccounters.SetColumnWidth (0, wx.LIST_AUTOSIZE)
		self.d_lccounters.Disable()
		self.d_hsizer3 = wx.BoxSizer (wx.HORIZONTAL)
		self.d_hsizer3.Add (self.d_useallregions, 1, wx.CENTER)
		self.d_hsizer3.AddSpacer (5)
		self.d_hsizer3.Add (self.d_lcregions, 3, wx.EXPAND|wx.ALL)
		self.d_hsizer3.AddSpacer (20)
		self.d_hsizer3.Add (self.d_useallcounters, 0, wx.CENTER)
		self.d_hsizer3.AddSpacer (5)
		self.d_hsizer3.Add (self.d_lccounters, 3, wx.EXPAND|wx.ALL)
		self.boxdataszr.AddSpacer (10)
		self.boxdataszr.Add (self.d_hsizer1, 0, wx.EXPAND|wx.ALL)
		self.boxdataszr.AddSpacer (10)
		self.boxdataszr.Add (self.d_splitgroups)
		self.boxdataszr.AddSpacer (10)
		self.boxdataszr.Add (self.d_hsizer2, 0, wx.EXPAND|wx.ALL)
		self.boxdataszr.AddSpacer (10)
		self.boxdataszr.Add (self.d_hsizer3, 0, wx.EXPAND|wx.ALL)
		self.boxdataszr.AddSpacer (10)

		# Outliers frame

		self.outlier = wx.StaticBox (panel, -1, "Outlier removal")
		self.outlierszr = wx.StaticBoxSizer (self.outlier, wx.VERTICAL)
		self.o_statistictxt1 = wx.StaticText (panel, -1, "Exclude instances using ")
		self.o_statistic = wx.Choice (panel, -1, choices = ['mean', 'median'])
		self.o_statistic.Bind (wx.EVT_CHOICE, self.OnChangeStatistic )
		self.o_statistic.SetStringSelection ('mean')
		self.o_statistictxt2 = wx.StaticText (panel, -1, "statistic and which are beyond ")		
		self.o_sigmatimes = wx.Choice (panel, -1, choices = ['0.25', '0.50', '0.75', '1.00', '1.25', '1.50', '1.75', '2.00', '3.00', '4.00'])
		self.o_sigmatimes.SetStringSelection ('2.00')
		self.o_statistictxt3 = wx.StaticText (panel, -1, "sigma times")
		self.o_hsizer1 = wx.BoxSizer (wx.HORIZONTAL)
		self.o_hsizer1.Add (self.o_statistictxt1, 0, wx.CENTER)
		self.o_hsizer1.Add (self.o_statistic, 0, wx.EXPAND|wx.ALL)
		self.o_hsizer1.AddSpacer (5)
		self.o_hsizer1.Add (self.o_statistictxt2, 0, wx.CENTER)
		self.o_hsizer1.Add (self.o_sigmatimes, 0, wx.EXPAND|wx.ALL)
		self.o_hsizer1.AddSpacer (5)
		self.o_hsizer1.Add (self.o_statistictxt3, 0, wx.CENTER)
		self.outlierszr.Add (self.o_hsizer1)

		# Interpolation frame

		self.interpolation = wx.StaticBox (panel, -1, "Interpolation options")		
		self.interpolationszr = wx.StaticBoxSizer (self.interpolation, wx.VERTICAL)
		self.i_stepsnumtxt = wx.StaticText (panel, -1, "# of interpolation steps:")
		self.i_stepsnum = wx.SpinCtrl (panel, -1)
		self.i_stepsnum.SetRange (1, 10000)
		self.i_stepsnum.SetValue (1000)
		self.i_nugettxt = wx.StaticText (panel, -1, "Kriger nuget")
		self.i_nuget = wx.TextCtrl(panel, -1, "0.0001")
		self.i_hsizer2 = wx.BoxSizer (wx.HORIZONTAL)
		self.i_prefilter = wx.ToggleButton (panel, -1, "Apply interpolation prefilter step")
		self.i_hsizer1 = wx.BoxSizer (wx.HORIZONTAL)
		self.i_hsizer1.AddSpacer (5)
		self.i_hsizer1.Add (self.i_stepsnumtxt, 0, wx.CENTER)
		self.i_hsizer1.AddSpacer (5)
		self.i_hsizer1.Add (self.i_stepsnum, 0, wx.CENTER)
		self.i_hsizer1.AddSpacer (10)
		self.i_hsizer1.Add (self.i_nugettxt, 0, wx.CENTER)
		self.i_hsizer1.AddSpacer (5)
		self.i_hsizer1.Add (self.i_nuget, 0, wx.CENTER)
		self.i_hsizer1.AddSpacer (10)
		self.i_hsizer1.Add (self.i_prefilter, 0, wx.CENTER)
		self.i_hsizer1.AddSpacer (5)
		self.interpolationszr.Add (self.i_hsizer1)

		# Feed frame

		self.feed = wx.StaticBox (panel, -1, "Generate additional output")
		self.feedszr = wx.StaticBoxSizer (self.feed, wx.VERTICAL)
		self.f_genprv = wx.ToggleButton (panel, -1, "Paraver tracefile")
		self.f_genprv.Bind (wx.EVT_TOGGLEBUTTON, self.OnChangeGenPRV )
		self.f_genprv.SetValue (True)
		self.f_genprvobjecttxt = wx.StaticText (panel, -1, "Choose object")
		self.f_genprvobject = wx.Choice (panel, -1, choices = self.objects)
		self.f_hsizer1 = wx.BoxSizer (wx.HORIZONTAL)
		self.f_hsizer1.Add (self.f_genprv, 0, wx.EXPAND|wx.ALL);
		self.f_hsizer1.AddSpacer (10)
		self.f_hsizer1.Add (self.f_genprvobjecttxt, 0, wx.CENTER)
		self.f_hsizer1.AddSpacer (5)
		self.f_hsizer1.Add (self.f_genprvobject, 0, wx.EXPAND|wx.ALL)
		self.feedszr.Add (self.f_hsizer1)

		# Continue button
		
		self.continueBtn = wx.Button(panel, label="Proceed >")
		self.continueBtn.Bind (wx.EVT_BUTTON, self.OnContinue )
		self.continueszr = wx.BoxSizer (wx.HORIZONTAL)
		self.continueszr.Add (self.continueBtn, 1, wx.ALL, border = 10)

		# Pack all and show

		self.sizerv.Add (self.boxdataszr, 0, wx.EXPAND, border = 5)
		self.sizerv.AddSpacer (15)
		self.sizerv.Add (self.outlierszr, 0, wx.EXPAND, border = 5)
		self.sizerv.AddSpacer (15)
		self.sizerv.Add (self.interpolationszr, 0, wx.EXPAND, border = 5)
		self.sizerv.AddSpacer (15)
		self.sizerv.Add (self.feedszr, 0, wx.EXPAND, border = 5)
		self.sizerv.AddSpacer (15)
		self.sizerv.Add (self.continueszr, 1, wx.CENTER, border = 10)

		panel.SetSizer (self.sizerv)
		self.Fit()	

	def OnChangeGenPRV(self, event=None):
		if self.f_genprv.GetValue():
			self.f_genprvobjecttxt.Enable()
			self.f_genprvobject.Enable()
		else:
			self.f_genprvobjecttxt.Disable()
			self.f_genprvobject.Disable()

	def OnChangeUseAllRegions(self, event=None):
		if not self.d_useallregions.GetValue():
			self.d_lcregions.Enable()
		else:
			self.d_lcregions.Disable()
			current = self.d_lcregions.GetNextSelected (-1)
			while current != -1:
				self.d_lcregions.SetItemState (current , 0, wx.LIST_STATE_SELECTED)
				current = self.d_lcregions.GetNextSelected (current)

	def OnChangeUseAllCounters(self, event=None):
		if not self.d_useallcounters.GetValue():
			self.d_lccounters.Enable()
		else:
			self.d_lccounters.Disable()
			current = self.d_lccounters.GetNextSelected (-1)
			while current != -1:
				self.d_lccounters.SetItemState (current , 0, wx.LIST_STATE_SELECTED)
				current = self.d_lccounters.GetNextSelected (current)

	def OnChangeUseAllObjects(self, event=None):
		if not self.d_useallobjects.GetValue():
			self.d_chooseobjecttxt.Enable()
			self.d_chooseobject.Enable()
		else:
			self.d_chooseobjecttxt.Disable()
			self.d_chooseobject.Disable()

	def OnChangeLimitSamples(self, event=None):
		if self.d_limitsamples.GetValue():
			self.d_limitsamplesnum.Enable()
			self.d_limitsamplesnumtxt.Enable()
		else:
			self.d_limitsamplesnum.Disable()
			self.d_limitsamplesnumtxt.Disable()

	def OnChangeStatistic(self, event=None):
		if self.o_statistic.GetStringSelection() == "median":
			self.o_statistictxt3.SetLabel ("mad times")
		else:
			self.o_statistictxt3.SetLabel ("sigma times")

	# Handlers for buttons
	def OnContinue(self, event=None):

		# Check for consistency of the data first

		# Check that the list use all or have something selected
		if not self.d_useallcounters.GetValue():
			if self.d_lccounters.GetNextSelected (-1) == -1:
				wx.MessageBox ('Choose some of the performance counters or just click on \'All counters\'', 'Stop', wx.OK | wx.ICON_EXCLAMATION)
				return
		if not self.d_useallregions.GetValue():
			if self.d_lcregions.GetNextSelected (-1) == -1:
				wx.MessageBox ('Choose some of the regions or just click on \'All regions\'', 'Stop', wx.OK | wx.ICON_EXCLAMATION)
				return

		# Check that nuget is floatable
		try:
			float (self.i_nuget.GetValue())
		except ValueError:
			wx.MessageBox ('Cannot convert \'' + self.i_nuget.GetValue() + '\' into a floating point value', 'Stop', wx.OK | wx.ICON_EXCLAMATION)
			return

		# Prepare output variables for the dialog
		if not self.d_useallobjects.GetValue():
			self.r_useobject = self.d_chooseobject.GetStringSelection()
		else:
			self.r_useobject = '*.*.*'
		self.r_splitingroups = self.d_splitgroups.GetValue()
		self.r_limitsamples = self.d_limitsamples.GetValue()
		self.r_limitsamplesnum = self.d_limitsamplesnum.GetValue()
		if not self.d_useallregions.GetValue():
			self.r_regions = []
			current = self.d_lcregions.GetNextSelected (-1)
			while current != -1:
				self.r_regions.append (self.regions[current])
				current = self.d_lcregions.GetNextSelected (current)
		else:
			self.r_regions = [ 'all' ]
		if not self.d_useallcounters.GetValue():
			self.r_counters = []
			current = self.d_lccounters.GetNextSelected (-1)
			while current != -1:
				self.r_counters.append (self.counters[current])
				current = self.d_lccounters.GetNextSelected (current)
		else:
			self.r_counters = [ 'all' ]
		self.r_statistic = self.o_statistic.GetStringSelection()
		self.r_sigmatimes = self.o_sigmatimes.GetStringSelection()
		self.r_steps = self.i_stepsnum.GetValue()
		self.r_kriger = float(self.i_nuget.GetValue())
		self.r_preinterpolate = self.i_prefilter.GetValue()
		self.r_genprv = self.f_genprv.GetValue()
		self.r_genprv_object = self.f_genprvobject.GetStringSelection()

		self.EndModal (wx.ID_OK)

#
# wxFolding, main class, orchestrator for the rest
#

class wxFolding(wx.Frame):
	def __init__(self, tracefile, *args, **kw):
		super(wxFolding, self).__init__(*args, **kw) 
		self.TraceFile = tracefile
		self.InitUI()

	def InitUI(self):    
		self.Show(False)

		dialog = wxFoldingInputDialog (parent = self, tracefile = self.TraceFile,
		  title = "Folding (step 1: Input data)")
		if not dialog.ShowModal() == wx.ID_OK:
			self.Destroy()
			return

		self.TraceFile = dialog.TraceFile
		self.SourceDir = dialog.SourceDir
		self.EventType = dialog.EventType
		self.WorkDir = dialog.WorkDir
		dialog.Destroy()

		os.chdir (self.WorkDir)

		# Execute 1st step for the folding process
		#  generate codeblocks
		#  fuse the tracefile
		#  data extraction

		command = FOLDING_HOME + "/bin/codeblocks"
		if self.SourceDir:
			command += " -source " + self.SourceDir
		command += " " + self.TraceFile
		print "Executing command: " + command
		if not os.system (command) == 0:
			print "Error while executing : " + command
			return

		position = self.TraceFile.rfind (".prv")
		TraceFileCB = self.TraceFile[0:position]+".codeblocks.prv"
		command = FOLDING_HOME + "/bin/fuse " + TraceFileCB
		print "Executing command: " + command
		if not os.system (command) == 0:
			print "Error while executing : " + command
			return

		position = TraceFileCB.rfind (".prv")
		TraceFileF = TraceFileCB[0:position]+".fused.prv"
		command = FOLDING_HOME + "/bin/extract -separator " + str(self.EventType) + " " + TraceFileF
		print "Executing command: " + command
		if not os.system (command) == 0:
			print "Error while executing : " + command
			return

		dialog = wxFoldingInterpolateKrigerDialog (parent = self, tracefile = TraceFileF, title = "Folding (step 2: Data mangement)")
		if not dialog.ShowModal() == wx.ID_OK:
			self.Destroy()
			return

		# Construct the interpolate command line
		command = FOLDING_HOME + "/bin/interpolate"
		command += " -use-object \"" + dialog.r_useobject + "\""
		if dialog.r_splitingroups:
			command += " -split-in-groups yes"
		else:
			command += " -split-in-groups no"
		command += " -use-" + dialog.r_statistic
		command += " -sigma-times " + dialog.r_sigmatimes
		if dialog.r_limitsamples:
			command += " -max-samples " + str(dialog.r_limitsamplesnum)
		for r in dialog.r_regions:
			command += " -region " + r
		for c in dialog.r_counters:
			command += " -counter " + c
		command += " -interpolation kriger " + str(dialog.r_steps) + " " + str(dialog.r_kriger)
		if dialog.r_preinterpolate:
			command += " yes"
		else:
			command += " no"
		if dialog.r_genprv:
			command += " -feed-first-occurrence " + dialog.r_genprv_object
		extractFile = TraceFileF[0:TraceFileF.rfind(".prv")]+".extract"
		command += " " + os.path.basename(extractFile)

		dialog.Destroy ()
		print "Executing command: " + command
		if not os.system (command) == 0:
			print "Error while executing : " + command
			return

		wxFile = TraceFileF[0:TraceFileF.rfind(".prv")]+".wxfolding"

		f = wxfolding_viewer.wxFoldingViewer (parent = None, dfile = os.path.basename (wxFile))

		self.Destroy()

#
# MAIN
#

if __name__ == "__main__":

	if len (sys.argv) > 2:
		print 'Error! Command ' + sys.argv[0] + ' can only process one tracefile at a time';
		sys.exit (-1)
	elif len (sys.argv) == 2:
		tracefile = sys.argv[1]
		if tracefile[0] != '/':
			tracefile = os.getcwd() + '/' + tracefile
	else:
		tracefile = None

	TMPDIR = os.getenv ("TMPDIR")
	if not TMPDIR:
		TMPDIR = "/tmp"
	FOLDING_HOME = os.getenv ("FOLDING_HOME")
	if FOLDING_HOME:
		ex = wx.App()
		f = wxFolding (parent = None, tracefile = tracefile)
		ex.MainLoop()
	else:
		print "FOLDING_HOME is not set!"

