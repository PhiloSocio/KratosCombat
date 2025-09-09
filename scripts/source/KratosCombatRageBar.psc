ScriptName KratosCombatRageBar Extends MCM_ConfigBase

iWant_Widgets Property iWidgets Auto

GlobalVariable Property Rage Auto
GlobalVariable Property RageLimit Auto
Int Property iRageBar Auto Hidden

Int Property RageBarZoom = 70 Auto Hidden
Int Property RageBarPosX = 1200 Auto Hidden
Int Property RageBarPosY = 460 Auto Hidden

Float _updateTimer = 0.5
Bool loaded = False
Float ragePercent

Event OnInit()
    RegisterForModEvent("iWantWidgetsReset", "OniWantWidgetsReset")
    CreateRageBar()
	RegisterForSingleUpdate(_updateTimer)
EndEvent

Event OniWantWidgetsReset(String eventName, String strArg, Float numArg, Form sender)
    If eventName == "iWantWidgetsReset"
        iWidgets = sender As iWant_Widgets
        Utility.Wait(1)
        If iWidgets
            CreateRageBar()
        Endif 
    Endif
Endevent

Event OnUpdate()
	UpdateStatus()
	RegisterForSingleUpdate(_updateTimer)
EndEvent

Function UpdateStatus()
	If loaded
        ragePercent = 100 * Rage.GetValue() / RageLimit.GetValue()
        setMeterPercentFloat(iWidgets, iRageBar, ragePercent)
	EndIf
EndFunction

Function CreateRageBar() 
    iRageBar = iWidgets.loadMeter(Visible = True)
    iWidgets.setRotation(iRageBar, RageBarRotation)
    iWidgets.setMeterRGB(iRageBar, 0, 0, 0, 22, 152, 74)
    iWidgets.SetZoom(iRageBar, RageBarZoom, RageBarZoom)
    iWidgets.setTransparency(iRageBar, 0)
    iWidgets.SetPos(iRageBar, RageBarPosX, RageBarPosY)
    iWidgets.sendToFront(iRageBar)
EndFunction

Function setMeterPercentFloat(iWant_Widgets akWidget, Int id, Float percent)
    String s
    String[] value
    
    value = new String[2]
    value[0] = id As String
    value[1] = percent As String

    s = akWidget._serializeArray(value)
    UI.InvokeString(akWidget.HUD_MENU, akWidget.WidgetRoot + ".setMeterPercent", s)
EndFunction