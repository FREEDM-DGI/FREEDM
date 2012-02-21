PSCAD 4.2.1

Settings
 {
 Id = "1285713896.1323446212"
 Author = "tpzmb.tprfh7"
 Desc = ""
 Arch = "windows"
 Options = 32
 Build = 18
 Warn = 1
 Check = 7
 Libs = ""
 Source = "psocket.c"
 RunInfo = 
  {
  Fin = 3
  Step = 2e-006
  Plot = 2.5e-005
  Chat = 0.001
  Brch = 0.0005
  Lat = 100
  Options = 0
  Advanced = 4607
  Debug = 0
  StartFile = ""
  OFile = "control3.out"
  SFile = "noname.snp"
  SnapTime = 0.3
  Mruns = 10
  Mrunfile = 0
  StartType = 1
  PlotType = 0
  SnapType = 0
  MrunType = "mrun"
  }

 }

Definitions
 {
 Module("A")
  {
  Desc = ""
  FileDate = 1288995317
  Nodes = 
   {
   Input("I1A",-54,-36)
    {
    Type = Real
    }
   Input("I1B",-54,0)
    {
    Type = Real
    }
   Input("I1C",-54,36)
    {
    Type = Real
    }
   Output("ID1",54,-18)
    {
    Type = Real
    }
   Output("IQ1",54,18)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-57,39,57)
   Pen(ByNode,ByNode,ByNode,I1A)
   Line(-54,-36,-39,-36)
   Pen(,Solid)
   Line(-39,-36,-44,-41)
   Line(-39,-36,-44,-31)
   Text(-46,-40,"I1A")
   Pen(,ByNode,,I1B)
   Line(-54,0,-39,0)
   Pen(,Solid)
   Line(-39,0,-44,-5)
   Line(-39,0,-44,5)
   Text(-46,-4,"I1B")
   Pen(,ByNode,,I1C)
   Line(-54,36,-39,36)
   Pen(,Solid)
   Line(-39,36,-44,31)
   Line(-39,36,-44,41)
   Text(-46,32,"I1C")
   Pen(,ByNode,,ID1)
   Line(54,-18,39,-18)
   Text(46,-22,"ID1")
   Pen(,,,IQ1)
   Line(54,18,39,18)
   Text(46,14,"IQ1")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.mult([504,162],0,0,100)
    {
    }
   0.emtconst([432,324],0,0,50)
    {
    Name = ""
    Value = "12"
    }
   0.const([432,162],0,0,20)
    {
    Name = ""
    Value = "-0.57735"
    }
   0.sumjct([612,252],1,0,150)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.mult([504,324],0,0,120)
    {
    }
   0.import([504,396],3,0,-1)
    {
    Name = "I1C"
    }
   -Wire-([540,162],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([612,216],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([540,324],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([540,252],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.export([612,324],5,0,190)
    {
    Name = "ID1"
    }
   0.mult([792,126],0,0,90)
    {
    }
   0.mult([792,270],0,0,110)
    {
    }
   0.mult([792,414],0,0,130)
    {
    }
   0.const([720,126],0,0,10)
    {
    Name = ""
    Value = "0.66667"
    }
   0.import([792,198],3,0,30)
    {
    Name = "I1A"
    }
   0.import([504,234],3,0,-1)
    {
    Name = "I1B"
    }
   0.import([792,342],3,0,60)
    {
    Name = "I1B"
    }
   0.import([792,486],3,0,80)
    {
    Name = "I1C"
    }
   0.const([720,270],0,0,40)
    {
    Name = ""
    Value = "-0.33333"
    }
   0.const([720,414],0,0,70)
    {
    Name = ""
    Value = "-0.33333"
    }
   0.sumjct([918,306],1,0,140)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "1"
    F = "1"
    G = "0"
    }
   -Wire-([828,126],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([918,270],0,0,-1)
    {
    Vertex="0,0;0,-144"
    }
   -Wire-([828,270],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([828,414],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([882,414],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   0.export([918,378],5,0,180)
    {
    Name = "IQ1"
    }
   0.pgb([450,522],0,59082144,170)
    {
    Name = "<Untitled>"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([450,522],0,0,-1)
    {
    Name = "ID1"
    }
   0.pgb([558,558],0,59082960,160)
    {
    Name = "<Untitled>"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([558,558],0,0,-1)
    {
    Name = "IQ1"
    }
   -Plot-([90,504],0)
    {
    Title = "$(GROUP) : Graphs"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,414]
    Icon = [90,504]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(59082960,"<Untitled>",0,,,)
     }
    }
   -Plot-([90,432],0)
    {
    Title = "$(GROUP) : Graphs"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,378]
    Icon = [90,432]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(59082144,"<Untitled>",0,,,)
     }
    }
   }
  }
 Module("Inverter1Control")
  {
  Desc = ""
  FileDate = 1291341105
  Nodes = 
   {
   Output("S13",54,-90)
    {
    Type = Real
    }
   Output("S16",54,-54)
    {
    Type = Real
    }
   Output("S14",54,-18)
    {
    Type = Real
    }
   Output("S17",54,18)
    {
    Type = Real
    }
   Output("S15",54,54)
    {
    Type = Real
    }
   Output("S18",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,S13)
   Line(54,-90,39,-90)
   Text(46,-94,"S13")
   Pen(,,,S16)
   Line(54,-54,39,-54)
   Text(46,-58,"S16")
   Pen(,,,S14)
   Line(54,-18,39,-18)
   Text(46,-22,"S14")
   Pen(,,,S17)
   Line(54,18,39,18)
   Text(46,14,"S17")
   Pen(,,,S15)
   Line(54,54,39,54)
   Text(46,50,"S15")
   Pen(,,,S18)
   Line(54,90,39,90)
   Text(46,86,"S18")
   Text(-1,3,"1")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.export([846,216],4,0,390)
    {
    Name = "S15"
    }
   0.export([846,270],4,0,380)
    {
    Name = "S18"
    }
   0.export([810,432],4,0,340)
    {
    Name = "S14"
    }
   0.export([828,504],4,0,330)
    {
    Name = "S17"
    }
   0.export([864,756],4,0,290)
    {
    Name = "S13"
    }
   0.export([882,810],4,0,280)
    {
    Name = "S16"
    }
   0.compar([648,216],0,0,170)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.modulator([558,180],0,0,160)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([558,36],1,0,10)
    {
    Name = ""
    Value = "0"
    }
   -Wire-([594,180],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([648,180],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   -Wire-([720,216],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([738,270],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.inv([738,270],0,0,180)
    {
    INTR = "0"
    }
   0.pgb([648,288],0,59087856,50)
    {
    Name = "G7tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([648,288],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([774,216],0,0,-1)
    {
    Name = "S15"
    }
   0.datalabel([774,270],0,0,-1)
    {
    Name = "S18"
    }
   0.signalgen([612,252],0,0,40)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   -Wire-([648,180],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.pgb([720,180],0,59089488,410)
    {
    Name = "G7sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([774,216],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([774,198],0,59089896,400)
    {
    Name = "S13"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([774,324],0,59090304,370)
    {
    Name = "S16"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([774,324],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.compar([594,432],0,0,210)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.signalgen([522,486],0,0,70)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   -Wire-([594,396],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   -Wire-([558,468],0,0,-1)
    {
    Vertex="0,0;0,18"
    }
   -Wire-([558,468],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([666,432],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([684,504],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([684,504],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.inv([702,504],0,0,220)
    {
    INTR = "0"
    }
   -Wire-([558,486],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([594,396],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([738,432],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([720,432],0,0,-1)
    {
    Name = "S14"
    }
   0.datalabel([738,504],0,0,-1)
    {
    Name = "S17"
    }
   -Wire-([756,540],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.pgb([648,396],0,59092752,360)
    {
    Name = "G8Sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([594,504],0,59093160,80)
    {
    Name = "G8Tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([594,504],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([738,396],0,59093568,350)
    {
    Name = "S14"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.pgb([756,540],0,59093976,320)
    {
    Name = "S17"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.compar([648,756],0,0,250)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.modulator([576,684],0,0,240)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([576,540],1,0,90)
    {
    Name = ""
    Value = "-4.189"
    }
   -Wire-([612,684],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([648,756],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([720,756],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([756,810],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([756,810],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.inv([774,810],0,0,260)
    {
    INTR = "0"
    }
   -Wire-([648,810],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([648,810],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([720,810],0,59096016,140)
    {
    Name = "G9tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([648,684],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.pgb([720,684],0,59096424,310)
    {
    Name = "G9Sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.signalgen([612,792],0,0,130)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   0.datalabel([792,756],0,0,-1)
    {
    Name = "S13"
    }
   0.datalabel([810,810],0,0,-1)
    {
    Name = "S16"
    }
   0.pgb([810,738],0,59098056,300)
    {
    Name = "S15"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.pgb([810,846],0,59098464,270)
    {
    Name = "S18"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   -Wire-([810,846],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([792,756],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([792,738],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.modulator([558,396],0,0,200)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([558,252],1,0,30)
    {
    Name = ""
    Value = "-2.094"
    }
   0.sumjct([558,108],7,0,150)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.sumjct([558,324],7,0,190)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.sumjct([576,612],7,0,230)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.const([450,756],3,0,120)
    {
    Name = "Magnitude"
    Value = "1.0"
    }
   -Wire-([450,720],0,0,-1)
    {
    Vertex="0,0;0,-540"
    }
   -Wire-([684,342],0,0,-1)
    {
    Vertex="0,0;252,0"
    }
   -Wire-([594,324],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([936,612],0,0,-1)
    {
    Vertex="0,0;0,-504"
    }
   -Wire-([684,342],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([612,612],0,0,-1)
    {
    Vertex="0,0;324,0"
    }
   0.const([972,612],2,0,100)
    {
    Name = "Phi(in radians)"
    Value = "3.5"
    }
   -Wire-([594,108],0,0,-1)
    {
    Vertex="0,0;342,0"
    }
   -Wire-([450,180],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([450,396],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([450,684],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([738,504],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([720,432],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([774,216],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([774,270],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([738,432],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([756,504],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([792,756],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([810,810],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.const([522,216],0,0,20)
    {
    Name = ""
    Value = "60"
    }
   0.const([522,432],0,0,60)
    {
    Name = ""
    Value = "60"
    }
   0.const([540,720],0,0,110)
    {
    Name = ""
    Value = "60"
    }
   }
  }
 Module("Inverter2Control")
  {
  Desc = ""
  FileDate = 1291341105
  Nodes = 
   {
   Output("S7",54,-90)
    {
    Type = Real
    }
   Output("S8",54,-54)
    {
    Type = Real
    }
   Output("S9",54,-18)
    {
    Type = Real
    }
   Output("S10",54,18)
    {
    Type = Real
    }
   Output("S11",54,54)
    {
    Type = Real
    }
   Output("S12",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,6,"2")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,S7)
   Line(54,-90,39,-90)
   Text(46,-94,"S7")
   Pen(,,,S8)
   Line(54,-54,39,-54)
   Text(46,-58,"S8")
   Pen(,,,S9)
   Line(54,-18,39,-18)
   Text(46,-22,"S9")
   Pen(,,,S10)
   Line(54,18,39,18)
   Text(46,14,"S10")
   Pen(,,,S11)
   Line(54,54,39,54)
   Text(46,50,"S11")
   Pen(,,,S12)
   Line(54,90,39,90)
   Text(46,86,"S12")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.export([900,216],4,0,390)
    {
    Name = "S7"
    }
   0.export([900,270],4,0,380)
    {
    Name = "S8"
    }
   0.export([864,432],4,0,340)
    {
    Name = "S9"
    }
   0.export([882,504],4,0,330)
    {
    Name = "S10"
    }
   0.export([918,756],4,0,290)
    {
    Name = "S11"
    }
   0.export([936,810],4,0,280)
    {
    Name = "S12"
    }
   0.compar([702,216],0,0,170)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.modulator([612,180],0,0,160)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([612,36],1,0,10)
    {
    Name = ""
    Value = "0"
    }
   -Wire-([648,180],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([702,180],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   -Wire-([774,216],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([792,270],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.inv([792,270],0,0,180)
    {
    INTR = "0"
    }
   0.pgb([702,288],0,59707584,50)
    {
    Name = "G4tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([702,288],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([828,216],0,0,-1)
    {
    Name = "S7"
    }
   0.datalabel([828,270],0,0,-1)
    {
    Name = "S8"
    }
   0.signalgen([666,252],0,0,40)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   -Wire-([702,180],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.pgb([774,180],0,59709216,410)
    {
    Name = "G4sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([828,216],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([828,198],0,59709624,400)
    {
    Name = "S7"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([828,324],0,59710032,370)
    {
    Name = "S8"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([828,324],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.compar([648,432],0,0,210)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.signalgen([576,486],0,0,70)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   -Wire-([648,396],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   -Wire-([612,468],0,0,-1)
    {
    Vertex="0,0;0,18"
    }
   -Wire-([612,468],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([720,432],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([738,504],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([738,504],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.inv([756,504],0,0,220)
    {
    INTR = "0"
    }
   -Wire-([612,486],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([648,396],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([792,432],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([774,432],0,0,-1)
    {
    Name = "S9"
    }
   0.datalabel([792,504],0,0,-1)
    {
    Name = "S10"
    }
   -Wire-([810,540],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.pgb([702,396],0,59712480,360)
    {
    Name = "G5Sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([648,504],0,59712888,80)
    {
    Name = "G5Tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([648,504],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([792,396],0,59713296,350)
    {
    Name = "S9"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.pgb([810,540],0,59713704,320)
    {
    Name = "S10"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.compar([702,756],0,0,250)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.modulator([630,684],0,0,240)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([630,540],1,0,90)
    {
    Name = ""
    Value = "-4.189"
    }
   -Wire-([666,684],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([702,756],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([774,756],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([810,810],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([810,810],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.inv([828,810],0,0,260)
    {
    INTR = "0"
    }
   -Wire-([702,810],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([702,810],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([774,810],0,59715744,140)
    {
    Name = "G6tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([702,684],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.pgb([774,684],0,59716152,310)
    {
    Name = "G6Sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.signalgen([666,792],0,0,130)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   0.datalabel([846,756],0,0,-1)
    {
    Name = "S11"
    }
   0.datalabel([864,810],0,0,-1)
    {
    Name = "S12"
    }
   0.pgb([864,738],0,59717784,300)
    {
    Name = "S11"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.pgb([864,846],0,59718192,270)
    {
    Name = "S12"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   -Wire-([864,846],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([846,756],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([846,738],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.modulator([612,396],0,0,200)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([612,252],1,0,30)
    {
    Name = ""
    Value = "-2.094"
    }
   0.sumjct([612,108],7,0,150)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.sumjct([612,324],7,0,190)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.sumjct([630,612],7,0,230)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.const([504,756],3,0,120)
    {
    Name = "Magnitude"
    Value = "1.0"
    }
   -Wire-([504,720],0,0,-1)
    {
    Vertex="0,0;0,-540"
    }
   -Wire-([738,342],0,0,-1)
    {
    Vertex="0,0;252,0"
    }
   -Wire-([648,324],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([990,612],0,0,-1)
    {
    Vertex="0,0;0,-504"
    }
   -Wire-([738,342],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([666,612],0,0,-1)
    {
    Vertex="0,0;324,0"
    }
   0.const([1026,612],2,0,100)
    {
    Name = "Phi(in radians)"
    Value = "3.5"
    }
   -Wire-([648,108],0,0,-1)
    {
    Vertex="0,0;342,0"
    }
   -Wire-([504,180],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([504,396],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([504,684],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([792,504],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([774,432],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([828,216],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([828,270],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([792,432],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([810,504],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([846,756],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([864,810],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.const([576,216],0,0,20)
    {
    Name = ""
    Value = "60"
    }
   0.const([576,432],0,0,60)
    {
    Name = ""
    Value = "60"
    }
   0.const([594,720],0,0,110)
    {
    Name = ""
    Value = "60"
    }
   }
  }
 Module("Inverter3Control")
  {
  Desc = ""
  FileDate = 1291341105
  Nodes = 
   {
   Output("S1",54,-90)
    {
    Type = Real
    }
   Output("S2",54,-54)
    {
    Type = Real
    }
   Output("S3",54,-18)
    {
    Type = Real
    }
   Output("S4",54,18)
    {
    Type = Real
    }
   Output("S5",54,54)
    {
    Type = Real
    }
   Output("S6",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,S1)
   Line(54,-90,39,-90)
   Text(46,-94,"S1")
   Pen(,,,S2)
   Line(54,-54,39,-54)
   Text(46,-58,"S2")
   Pen(,,,S3)
   Line(54,-18,39,-18)
   Text(46,-22,"S3")
   Pen(,,,S4)
   Line(54,18,39,18)
   Text(46,14,"S4")
   Pen(,,,S5)
   Line(54,54,39,54)
   Text(46,50,"S5")
   Pen(,,,S6)
   Line(54,90,39,90)
   Text(46,86,"S6")
   Text(-1,5,"3")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.export([648,216],4,0,400)
    {
    Name = "S2"
    }
   0.export([612,270],4,0,390)
    {
    Name = "S1"
    }
   0.export([594,432],4,0,350)
    {
    Name = "S4"
    }
   0.export([594,504],4,0,340)
    {
    Name = "S3"
    }
   0.export([666,756],4,0,300)
    {
    Name = "S6"
    }
   0.export([666,810],4,0,290)
    {
    Name = "S5"
    }
   0.compar([414,216],0,0,180)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.modulator([324,180],0,0,170)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([324,36],1,0,10)
    {
    Name = ""
    Value = "0"
    }
   -Wire-([360,180],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([414,180],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   -Wire-([486,216],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([504,270],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.inv([504,270],0,0,190)
    {
    INTR = "0"
    }
   0.pgb([414,288],0,59726760,60)
    {
    Name = "G1tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([414,288],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([540,216],0,0,-1)
    {
    Name = "S2"
    }
   0.datalabel([540,270],0,0,-1)
    {
    Name = "S1"
    }
   -Wire-([414,180],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.pgb([486,180],0,59727984,420)
    {
    Name = "G1sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([540,216],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([540,198],0,59728392,410)
    {
    Name = "S2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([540,324],0,59728800,380)
    {
    Name = "S2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([540,324],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.compar([360,432],0,0,220)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.signalgen([288,486],0,0,80)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   -Wire-([360,396],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   -Wire-([324,468],0,0,-1)
    {
    Vertex="0,0;0,18"
    }
   -Wire-([324,468],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([432,432],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([450,504],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([450,504],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.inv([468,504],0,0,230)
    {
    INTR = "0"
    }
   -Wire-([324,486],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([360,396],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([504,432],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([486,432],0,0,-1)
    {
    Name = "S4"
    }
   0.datalabel([504,504],0,0,-1)
    {
    Name = "S3"
    }
   -Wire-([522,540],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.pgb([414,396],0,59731248,370)
    {
    Name = "G2Sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([360,504],0,59731656,90)
    {
    Name = "G2Tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([360,504],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([504,396],0,59732064,360)
    {
    Name = "S3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.pgb([522,540],0,59732472,330)
    {
    Name = "S4"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.compar([414,756],0,0,260)
    {
    Pulse = "0"
    INTR = "0"
    OPos = "1"
    ONone = "0"
    ONeg = "1"
    OHi = "1"
    OLo = "0"
    }
   0.modulator([342,684],0,0,250)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([342,540],1,0,100)
    {
    Name = ""
    Value = "-4.189"
    }
   -Wire-([378,684],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([414,756],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([486,756],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([522,810],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([522,810],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.inv([540,810],0,0,270)
    {
    INTR = "0"
    }
   -Wire-([414,810],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([414,810],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.pgb([468,810],0,59734512,150)
    {
    Name = "G3tri"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   -Wire-([414,684],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.pgb([486,684],0,59734920,320)
    {
    Name = "G3Sin"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.signalgen([378,792],0,0,140)
    {
    F = "5000.0 [Hz]"
    P = "0.0 [deg]"
    Type = "1"
    Duty = "50 [%]"
    Max = "1"
    Min = "-1"
    INTR = "0"
    }
   0.datalabel([558,756],0,0,-1)
    {
    Name = "S6"
    }
   0.datalabel([576,810],0,0,-1)
    {
    Name = "S5"
    }
   0.pgb([576,738],0,59794024,310)
    {
    Name = "S6"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   0.pgb([576,846],0,59794432,280)
    {
    Name = "S6"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "0"
    Max = "1.0"
    }
   -Wire-([576,846],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([558,756],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([558,738],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.modulator([324,396],0,0,210)
    {
    Type = "1"
    FMod = "1"
    PMod = "0"
    }
   0.const([324,252],1,0,40)
    {
    Name = ""
    Value = "-2.094"
    }
   0.sumjct([324,108],7,0,160)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.sumjct([324,324],7,0,200)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.sumjct([342,612],7,0,240)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.const([216,756],3,0,130)
    {
    Name = "Magnitude"
    Value = "1"
    }
   -Wire-([216,720],0,0,-1)
    {
    Vertex="0,0;0,-540"
    }
   -Wire-([450,342],0,0,-1)
    {
    Vertex="0,0;252,0"
    }
   -Wire-([360,324],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([702,612],0,0,-1)
    {
    Vertex="0,0;0,-504"
    }
   -Wire-([450,342],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([378,612],0,0,-1)
    {
    Vertex="0,0;324,0"
    }
   0.const([738,612],2,0,110)
    {
    Name = "Phi(in radians)"
    Value = "3.5"
    }
   -Wire-([360,108],0,0,-1)
    {
    Vertex="0,0;342,0"
    }
   -Wire-([216,180],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([216,396],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([216,684],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([504,504],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([486,432],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([558,756],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([576,810],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([522,504],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([504,432],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([540,216],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([540,270],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.sig_gen([378,252],0,0,50)
    {
    Max = "1"
    Min = "-1"
    }
   0.const([828,108],0,0,20)
    {
    Name = ""
    Value = "5000"
    }
   0.datalabel([864,108],0,0,-1)
    {
    Name = "f"
    }
   0.datalabel([342,252],0,0,-1)
    {
    Name = "f"
    }
   0.const([306,720],0,0,120)
    {
    Name = ""
    Value = "60"
    }
   0.const([288,432],0,0,70)
    {
    Name = ""
    Value = "60"
    }
   0.const([288,216],0,0,30)
    {
    Name = ""
    Value = "60"
    }
   }
  }
 Module("Inverter1")
  {
  Desc = ""
  FileDate = 0
  Nodes = 
   {
   Input("Pcommand",-54,-54)
    {
    Type = Real
    }
   Input("Qcommand",-54,-18)
    {
    Type = Real
    }
   Input("VD1",-54,18)
    {
    Type = Real
    }
   Input("VQ1",-54,54)
    {
    Type = Real
    }
   Output("S13",54,-90)
    {
    Type = Real
    }
   Output("S14",54,-54)
    {
    Type = Real
    }
   Output("S15",54,-18)
    {
    Type = Real
    }
   Output("S16",54,18)
    {
    Type = Real
    }
   Output("S17",54,54)
    {
    Type = Real
    }
   Output("S18",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,6,"1")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,Pcommand)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"Pcomman")
   Pen(,ByNode,,Qcommand)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"Qcomman")
   Pen(,ByNode,,VD1)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"VD1")
   Pen(,ByNode,,VQ1)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"VQ1")
   Pen(,ByNode,,S13)
   Line(54,-90,39,-90)
   Text(46,-94,"S13")
   Pen(,,,S14)
   Line(54,-54,39,-54)
   Text(46,-58,"S14")
   Pen(,,,S15)
   Line(54,-18,39,-18)
   Text(46,-22,"S15")
   Pen(,,,S16)
   Line(54,18,39,18)
   Text(46,14,"S16")
   Pen(,,,S17)
   Line(54,54,39,54)
   Text(46,50,"S17")
   Pen(,,,S18)
   Line(54,90,39,90)
   Text(46,86,"S18")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([54,108],0,0,-1)
    {
    Name = "Pcommand"
    }
   0.import([54,144],0,0,-1)
    {
    Name = "Qcommand"
    }
   0.import([54,180],0,0,-1)
    {
    Name = "VD1"
    }
   0.import([54,216],0,0,-1)
    {
    Name = "VQ1"
    }
   0.export([198,108],4,0,-1)
    {
    Name = "S13"
    }
   0.export([198,144],4,0,-1)
    {
    Name = "S14"
    }
   0.export([198,180],4,0,-1)
    {
    Name = "S15"
    }
   0.export([198,216],4,0,-1)
    {
    Name = "S16"
    }
   0.export([198,252],4,0,-1)
    {
    Name = "S17"
    }
   0.export([198,288],4,0,-1)
    {
    Name = "S18"
    }
   }
  }
 Module("Inverter2")
  {
  Desc = ""
  FileDate = 0
  Nodes = 
   {
   Input("Pcommand2",-54,-54)
    {
    Type = Real
    }
   Input("Qcommand2",-54,-18)
    {
    Type = Real
    }
   Input("VD2",-54,18)
    {
    Type = Real
    }
   Input("VQ2",-54,54)
    {
    Type = Real
    }
   Output("S7",54,-90)
    {
    Type = Real
    }
   Output("S8",54,-54)
    {
    Type = Real
    }
   Output("S9",54,-18)
    {
    Type = Real
    }
   Output("S10",54,18)
    {
    Type = Real
    }
   Output("S11",54,54)
    {
    Type = Real
    }
   Output("S12",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,6,"2")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,Pcommand2)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"Pcomman")
   Pen(,ByNode,,Qcommand2)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"Pcomman")
   Pen(,ByNode,,VD2)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"VD2")
   Pen(,ByNode,,VQ2)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"VQ2")
   Pen(,ByNode,,S7)
   Line(54,-90,39,-90)
   Text(46,-94,"S7")
   Pen(,,,S8)
   Line(54,-54,39,-54)
   Text(46,-58,"S8")
   Pen(,,,S9)
   Line(54,-18,39,-18)
   Text(46,-22,"S9")
   Pen(,,,S10)
   Line(54,18,39,18)
   Text(46,14,"S10")
   Pen(,,,S11)
   Line(54,54,39,54)
   Text(46,50,"S11")
   Pen(,,,S12)
   Line(54,90,39,90)
   Text(46,86,"S12")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([450,126],0,0,-1)
    {
    Name = "Pcommand2"
    }
   0.import([450,234],0,0,-1)
    {
    Name = "Qcommand2"
    }
   0.import([486,270],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([486,162],0,0,-1)
    {
    Name = "VQ2"
    }
   0.export([198,108],4,0,-1)
    {
    Name = "S7"
    }
   0.export([198,144],4,0,-1)
    {
    Name = "S8"
    }
   0.export([198,180],4,0,-1)
    {
    Name = "S9"
    }
   0.export([198,216],4,0,-1)
    {
    Name = "S10"
    }
   0.export([198,252],4,0,-1)
    {
    Name = "S11"
    }
   0.export([198,288],4,0,-1)
    {
    Name = "S12"
    }
   0.div([738,126],0,0,390)
    {
    }
   0.sumjct([594,126],0,0,370)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   -Wire-([594,234],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([558,234],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([666,126],0,0,380)
    {
    }
   0.const([666,198],3,0,70)
    {
    Name = ""
    Value = "2"
    }
   0.datalabel([738,162],0,0,-1)
    {
    Name = "Z2"
    }
   0.datalabel([774,126],0,0,-1)
    {
    Name = "IQc2"
    }
   -Wire-([558,162],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([558,162],0,0,-1)
    {
    Name = "S2"
    }
   0.mult([522,126],0,0,300)
    {
    }
   0.mult([522,234],0,0,310)
    {
    }
   -Wire-([558,270],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([558,270],0,0,-1)
    {
    Name = "T2"
    }
   -Wire-([702,144],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.datalabel([702,144],0,0,-1)
    {
    Name = "N2"
    }
   0.div([1278,126],0,0,360)
    {
    }
   0.sumjct([1134,126],0,0,340)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   -Wire-([1134,234],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([1098,234],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([1206,126],0,0,350)
    {
    }
   0.const([1206,198],3,0,40)
    {
    Name = ""
    Value = "2"
    }
   0.datalabel([1278,162],0,0,-1)
    {
    Name = "Z2"
    }
   0.datalabel([1314,126],0,0,-1)
    {
    Name = "IDc2"
    }
   -Wire-([1242,180],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1242,180],0,0,-1)
    {
    Name = "C2"
    }
   0.mult([1062,126],0,0,280)
    {
    }
   0.mult([1062,234],0,0,290)
    {
    }
   0.import([990,126],0,0,-1)
    {
    Name = "Pcommand2"
    }
   0.import([990,234],0,0,-1)
    {
    Name = "Qcommand2"
    }
   0.import([1026,162],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([1026,270],0,0,-1)
    {
    Name = "VD2"
    }
   0.datalabel([504,378],0,0,-1)
    {
    Name = "VD1"
    }
   0.datalabel([540,414],0,0,-1)
    {
    Name = "ID1"
    }
   0.sumjct([612,378],0,0,510)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.datalabel([540,468],0,0,-1)
    {
    Name = "VQ1"
    }
   0.datalabel([576,504],0,0,-1)
    {
    Name = "IQ1"
    }
   0.mult([540,378],0,0,440)
    {
    }
   0.mult([576,468],0,0,500)
    {
    }
   -Wire-([612,468],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([684,378],0,0,660)
    {
    }
   0.const([684,450],3,0,130)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([720,378],0,0,-1)
    {
    Name = "P"
    }
   -Wire-([648,414],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([648,414],0,0,-1)
    {
    Name = "A"
    }
   }
  }
 Module("Inverter3")
  {
  Desc = ""
  FileDate = 0
  Nodes = 
   {
   Input("Pcommand3",-54,-54)
    {
    Type = Real
    }
   Input("Qcommand3",-54,-18)
    {
    Type = Real
    }
   Input("VD3",-54,18)
    {
    Type = Real
    }
   Input("VQ3",-54,54)
    {
    Type = Real
    }
   Output("S1",54,-90)
    {
    Type = Real
    }
   Output("S2",54,-54)
    {
    Type = Real
    }
   Output("S3",54,-18)
    {
    Type = Real
    }
   Output("S4",54,18)
    {
    Type = Real
    }
   Output("S5",54,54)
    {
    Type = Real
    }
   Output("S6",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,6,"3")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,Pcommand3)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"Pcomman")
   Pen(,ByNode,,Qcommand3)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"Qcomman")
   Pen(,ByNode,,VD3)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"VD3")
   Pen(,ByNode,,VQ3)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"VQ3")
   Pen(,ByNode,,S1)
   Line(54,-90,39,-90)
   Text(46,-94,"S1")
   Pen(,,,S2)
   Line(54,-54,39,-54)
   Text(46,-58,"S2")
   Pen(,,,S3)
   Line(54,-18,39,-18)
   Text(46,-22,"S3")
   Pen(,,,S4)
   Line(54,18,39,18)
   Text(46,14,"S4")
   Pen(,,,S5)
   Line(54,54,39,54)
   Text(46,50,"S5")
   Pen(,,,S6)
   Line(54,90,39,90)
   Text(46,86,"S6")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([54,108],0,0,-1)
    {
    Name = "Pcommand3"
    }
   0.import([54,144],0,0,-1)
    {
    Name = "Qcommand3"
    }
   0.import([54,180],0,0,-1)
    {
    Name = "VD3"
    }
   0.import([54,216],0,0,-1)
    {
    Name = "VQ3"
    }
   0.export([198,108],4,0,-1)
    {
    Name = "S1"
    }
   0.export([198,144],4,0,-1)
    {
    Name = "S2"
    }
   0.export([198,180],4,0,-1)
    {
    Name = "S3"
    }
   0.export([198,216],4,0,-1)
    {
    Name = "S4"
    }
   0.export([198,252],4,0,-1)
    {
    Name = "S5"
    }
   0.export([198,288],4,0,-1)
    {
    Name = "S6"
    }
   }
  }
 Module("Inverter1_1")
  {
  Desc = ""
  FileDate = 0
  Nodes = 
   {
   Input("Pcommand1",-54,-90)
    {
    Type = Real
    }
   Input("Qcommand1",-54,-54)
    {
    Type = Real
    }
   Input("VD1",-54,-18)
    {
    Type = Real
    }
   Input("VQ1",-54,18)
    {
    Type = Real
    }
   Input("ID1",-54,54)
    {
    Type = Real
    }
   Input("IQ1",-54,90)
    {
    Type = Real
    }
   Output("S13",54,-90)
    {
    Type = Real
    }
   Output("S14",54,-54)
    {
    Type = Real
    }
   Output("S15",54,-18)
    {
    Type = Real
    }
   Output("S16",54,18)
    {
    Type = Real
    }
   Output("S17",54,54)
    {
    Type = Real
    }
   Output("S18",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,6,"1")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,Pcommand1)
   Line(-54,-90,-39,-90)
   Pen(,Solid)
   Line(-39,-90,-44,-95)
   Line(-39,-90,-44,-85)
   Text(-46,-94,"Pcomman")
   Pen(,ByNode,,Qcommand1)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"Qcomman")
   Pen(,ByNode,,VD1)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"VD1")
   Pen(,ByNode,,VQ1)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"VQ1")
   Pen(,ByNode,,ID1)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"ID1")
   Pen(,ByNode,,IQ1)
   Line(-54,90,-39,90)
   Pen(,Solid)
   Line(-39,90,-44,85)
   Line(-39,90,-44,95)
   Text(-46,86,"IQ1")
   Pen(,ByNode,,S13)
   Line(54,-90,39,-90)
   Text(46,-94,"S13")
   Pen(,,,S14)
   Line(54,-54,39,-54)
   Text(46,-58,"S14")
   Pen(,,,S15)
   Line(54,-18,39,-18)
   Text(46,-22,"S15")
   Pen(,,,S16)
   Line(54,18,39,18)
   Text(46,14,"S16")
   Pen(,,,S17)
   Line(54,54,39,54)
   Text(46,50,"S17")
   Pen(,,,S18)
   Line(54,90,39,90)
   Text(46,86,"S18")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([54,108],0,0,-1)
    {
    Name = "Pcommand1"
    }
   0.import([54,144],0,0,-1)
    {
    Name = "Qcommand1"
    }
   0.import([54,180],0,0,-1)
    {
    Name = "VD1"
    }
   0.import([54,216],0,0,-1)
    {
    Name = "VQ1"
    }
   0.import([54,252],0,0,-1)
    {
    Name = "ID1"
    }
   0.import([54,288],0,0,-1)
    {
    Name = "IQ1"
    }
   0.export([198,108],4,0,-1)
    {
    Name = "S13"
    }
   0.export([198,144],4,0,-1)
    {
    Name = "S14"
    }
   0.export([198,180],4,0,-1)
    {
    Name = "S15"
    }
   0.export([198,216],4,0,-1)
    {
    Name = "S16"
    }
   0.export([198,252],4,0,-1)
    {
    Name = "S17"
    }
   0.export([198,288],4,0,-1)
    {
    Name = "S18"
    }
   }
  }
 Module("Inverter2_1")
  {
  Desc = ""
  FileDate = 0
  Nodes = 
   {
   Input("Pcommand2",-54,-90)
    {
    Type = Real
    }
   Input("Qcommand2",-54,-54)
    {
    Type = Real
    }
   Input("VD2",-54,-18)
    {
    Type = Real
    }
   Input("VQ2",-54,18)
    {
    Type = Real
    }
   Input("ID2",-54,54)
    {
    Type = Real
    }
   Input("IQ2",-54,90)
    {
    Type = Real
    }
   Output("S7",54,-90)
    {
    Type = Real
    }
   Output("S8",54,-54)
    {
    Type = Real
    }
   Output("S9",54,-18)
    {
    Type = Real
    }
   Output("S10",54,18)
    {
    Type = Real
    }
   Output("S11",54,54)
    {
    Type = Real
    }
   Output("S12",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,6,"2")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,Pcommand2)
   Line(-54,-90,-39,-90)
   Pen(,Solid)
   Line(-39,-90,-44,-95)
   Line(-39,-90,-44,-85)
   Text(-46,-94,"Pcomman")
   Pen(,ByNode,,Qcommand2)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"Qcomman")
   Pen(,ByNode,,VD2)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"VD2")
   Pen(,ByNode,,VQ2)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"VQ2")
   Pen(,ByNode,,ID2)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"ID2")
   Pen(,ByNode,,IQ2)
   Line(-54,90,-39,90)
   Pen(,Solid)
   Line(-39,90,-44,85)
   Line(-39,90,-44,95)
   Text(-46,86,"IQ2")
   Pen(,ByNode,,S7)
   Line(54,-90,39,-90)
   Text(46,-94,"S7")
   Pen(,,,S8)
   Line(54,-54,39,-54)
   Text(46,-58,"S8")
   Pen(,,,S9)
   Line(54,-18,39,-18)
   Text(46,-22,"S9")
   Pen(,,,S10)
   Line(54,18,39,18)
   Text(46,14,"S10")
   Pen(,,,S11)
   Line(54,54,39,54)
   Text(46,50,"S11")
   Pen(,,,S12)
   Line(54,90,39,90)
   Text(46,86,"S12")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([486,198],0,0,-1)
    {
    Name = "Pcommand2"
    }
   0.import([486,306],0,0,-1)
    {
    Name = "Qcommand2"
    }
   0.import([522,342],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([522,234],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([54,252],0,0,-1)
    {
    Name = "ID2"
    }
   0.import([846,468],0,0,-1)
    {
    Name = "IQ2"
    }
   0.export([198,108],4,0,-1)
    {
    Name = "S7"
    }
   0.export([198,144],4,0,-1)
    {
    Name = "S8"
    }
   0.export([198,180],4,0,-1)
    {
    Name = "S9"
    }
   0.export([198,216],4,0,-1)
    {
    Name = "S10"
    }
   0.export([198,252],4,0,-1)
    {
    Name = "S11"
    }
   0.export([198,288],4,0,-1)
    {
    Name = "S12"
    }
   0.sumjct([918,342],0,0,510)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.mult([846,342],0,0,440)
    {
    }
   0.mult([882,432],0,0,500)
    {
    }
   -Wire-([918,432],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([990,342],0,0,660)
    {
    }
   0.const([990,414],3,0,130)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1026,342],0,0,-1)
    {
    Name = "P2"
    }
   0.sumjct([1062,522],0,0,490)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.mult([990,522],0,0,250)
    {
    }
   0.mult([1026,612],0,0,480)
    {
    }
   -Wire-([1062,612],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([1134,522],0,0,550)
    {
    }
   0.const([1134,594],3,0,200)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1170,522],0,0,-1)
    {
    Name = "Q2"
    }
   0.div([1188,36],0,0,360)
    {
    }
   0.datalabel([612,378],0,0,-1)
    {
    Name = "VD1"
    }
   0.datalabel([540,378],0,0,-1)
    {
    Name = "VQ1"
    }
   0.pgb([612,378],0,59892752,670)
    {
    Name = "VD1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([540,378],0,59893160,680)
    {
    Name = "VQ1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([792,72],0,0,-1)
    {
    Name = "IDc2"
    }
   0.pgb([792,72],0,59893976,820)
    {
    Name = "IDc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([630,450],0,0,-1)
    {
    Name = "I2Ac2"
    }
   0.datalabel([630,486],0,0,-1)
    {
    Name = "I2Bc2"
    }
   0.datalabel([630,522],0,0,-1)
    {
    Name = "I2Cc2"
    }
   0.pgb([630,450],0,59895608,600)
    {
    Name = "I1Ac"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([630,486],0,59896016,580)
    {
    Name = "I1Bc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([630,522],0,59896424,560)
    {
    Name = "I1Cc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1098,306],0,0,-1)
    {
    Name = "P2"
    }
   0.pgb([1098,306],0,59897240,690)
    {
    Name = "P"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([666,342],0,0,-1)
    {
    Name = "Q2"
    }
   0.pgb([666,342],0,59898056,700)
    {
    Name = "Q"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.sumjct([1044,36],0,0,340)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   -Wire-([1044,144],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([1008,144],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([1116,36],0,0,350)
    {
    }
   0.const([1116,108],3,0,40)
    {
    Name = ""
    Value = "2"
    }
   0.datalabel([1188,72],0,0,-1)
    {
    Name = "Z2"
    }
   0.datalabel([1224,36],0,0,-1)
    {
    Name = "IDc2"
    }
   0.div([774,198],0,0,390)
    {
    }
   0.sumjct([630,198],0,0,370)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   -Wire-([630,306],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([594,306],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([702,198],0,0,380)
    {
    }
   0.const([702,270],3,0,70)
    {
    Name = ""
    Value = "2"
    }
   0.datalabel([774,234],0,0,-1)
    {
    Name = "Z2"
    }
   0.datalabel([810,198],0,0,-1)
    {
    Name = "IQc2"
    }
   -Wire-([594,234],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([594,234],0,0,-1)
    {
    Name = "S2"
    }
   0.mult([558,198],0,0,300)
    {
    }
   0.datalabel([900,216],0,0,-1)
    {
    Name = "S2"
    }
   0.pgb([900,216],0,59904176,770)
    {
    Name = "S"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([558,306],0,0,310)
    {
    }
   -Wire-([594,342],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([594,342],0,0,-1)
    {
    Name = "T2"
    }
   0.datalabel([900,270],0,0,-1)
    {
    Name = "T2"
    }
   0.pgb([900,270],0,59905808,720)
    {
    Name = "T"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([738,216],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.datalabel([738,216],0,0,-1)
    {
    Name = "N2"
    }
   0.datalabel([1098,342],0,0,-1)
    {
    Name = "N2"
    }
   0.pgb([1098,342],0,59907032,640)
    {
    Name = "N"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1026,252],0,0,-1)
    {
    Name = "Z2"
    }
   0.pgb([1026,252],0,59932552,730)
    {
    Name = "Z"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([954,378],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([954,378],0,0,-1)
    {
    Name = "A2"
    }
   0.datalabel([1116,432],0,0,-1)
    {
    Name = "A2"
    }
   0.pgb([1116,432],0,59933776,590)
    {
    Name = "A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([738,504],0,59934184,160)
    {
    Name = "I1B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([738,558],0,59934592,190)
    {
    Name = "I1C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([738,504],0,0,-1)
    {
    Name = "I1B"
    }
   0.datalabel([738,558],0,0,-1)
    {
    Name = "I1C"
    }
   -Wire-([1098,576],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1098,576],0,0,-1)
    {
    Name = "B2"
    }
   0.datalabel([1134,468],0,0,-1)
    {
    Name = "B2"
    }
   0.pgb([1134,468],0,59936632,570)
    {
    Name = "B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1152,90],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1152,90],0,0,-1)
    {
    Name = "C2"
    }
   0.datalabel([1134,180],0,0,-1)
    {
    Name = "C2"
    }
   0.pgb([1134,180],0,59937856,780)
    {
    Name = "C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([972,36],0,0,280)
    {
    }
   0.mult([972,144],0,0,290)
    {
    }
   0.import([900,36],0,0,-1)
    {
    Name = "Qcommand2"
    }
   0.import([900,144],0,0,-1)
    {
    Name = "Pcommand2"
    }
   0.import([936,72],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([936,180],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([774,342],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([810,378],0,0,-1)
    {
    Name = "ID2"
    }
   0.import([810,432],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([918,522],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([954,558],0,0,-1)
    {
    Name = "ID2"
    }
   0.import([954,612],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([990,648],0,0,-1)
    {
    Name = "IQ2"
    }
   0.sumjct([576,774],0,0,430)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([540,774],2,0,-1)
    {
    Name = "I2Ac2"
    }
   0.datalabel([576,810],0,0,-1)
    {
    Name = "I2A"
    }
   -Wire-([648,774],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([738,774],0,0,-1)
    {
    Name = "S8"
    }
   0.inv([702,810],0,0,540)
    {
    INTR = "0"
    }
   -Wire-([702,774],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([738,810],0,0,-1)
    {
    Name = "S7"
    }
   0.buffer([612,774],0,0,470)
    {
    HI = "0.001"
    LO = "-0.001"
    Inv = "0"
    INTR = "0"
    }
   0.sumjct([828,756],0,0,420)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([792,756],2,0,-1)
    {
    Name = "I2Bc"
    }
   0.datalabel([828,792],0,0,-1)
    {
    Name = "I2B"
    }
   -Wire-([900,756],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([990,756],0,0,-1)
    {
    Name = "S10"
    }
   0.inv([954,792],0,0,530)
    {
    INTR = "0"
    }
   -Wire-([954,756],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([990,792],0,0,-1)
    {
    Name = "S9"
    }
   0.buffer([864,756],0,0,460)
    {
    HI = "0.001"
    LO = "-0.001"
    Inv = "0"
    INTR = "0"
    }
   0.sumjct([684,918],0,0,410)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([648,918],2,0,-1)
    {
    Name = "I2Cc"
    }
   0.datalabel([684,954],0,0,-1)
    {
    Name = "I2C"
    }
   -Wire-([756,918],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([846,918],0,0,-1)
    {
    Name = "S12"
    }
   0.inv([810,954],0,0,520)
    {
    INTR = "0"
    }
   -Wire-([810,918],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([846,954],0,0,-1)
    {
    Name = "S11"
    }
   0.buffer([720,918],0,0,450)
    {
    HI = "0.001"
    LO = "-0.001"
    Inv = "0"
    INTR = "0"
    }
   0.abcdq0([666,666],0,0,400)
    {
    IDir = "0"
    Theta = "Qe"
    }
   0.datalabel([630,648],0,0,-1)
    {
    Name = "IDc2"
    }
   0.datalabel([630,666],0,0,-1)
    {
    Name = "IQc2"
    }
   0.datalabel([630,684],0,0,-1)
    {
    Name = "I02"
    }
   0.datalabel([702,648],0,0,-1)
    {
    Name = "I2Ac2"
    }
   0.datalabel([702,666],0,0,-1)
    {
    Name = "I2Bc2"
    }
   0.datalabel([702,684],0,0,-1)
    {
    Name = "I2Cc2"
    }
   }
  }
 Module("Inverter3_1")
  {
  Desc = ""
  FileDate = 0
  Nodes = 
   {
   Input("Pcommand3",-54,-90)
    {
    Type = Real
    }
   Input("Qcommand3",-54,-54)
    {
    Type = Real
    }
   Input("VD3",-54,-18)
    {
    Type = Real
    }
   Input("VQ3",-54,18)
    {
    Type = Real
    }
   Input("ID3",-54,54)
    {
    Type = Real
    }
   Input("IQ3",-54,90)
    {
    Type = Real
    }
   Output("S1",54,-90)
    {
    Type = Real
    }
   Output("S2",54,-54)
    {
    Type = Real
    }
   Output("S3",54,-18)
    {
    Type = Real
    }
   Output("S4",54,18)
    {
    Type = Real
    }
   Output("S5",54,54)
    {
    Type = Real
    }
   Output("S6",54,90)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-111,39,111)
   Text(0,-49,"Inverter")
   Text(0,6,"3")
   Text(0,61,"Control")
   Pen(ByNode,ByNode,ByNode,Pcommand3)
   Line(-54,-90,-39,-90)
   Pen(,Solid)
   Line(-39,-90,-44,-95)
   Line(-39,-90,-44,-85)
   Text(-46,-94,"Pcomman")
   Pen(,ByNode,,Qcommand3)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"Qcomman")
   Pen(,ByNode,,VD3)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"VD3")
   Pen(,ByNode,,VQ3)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"VQ3")
   Pen(,ByNode,,ID3)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"ID3")
   Pen(,ByNode,,IQ3)
   Line(-54,90,-39,90)
   Pen(,Solid)
   Line(-39,90,-44,85)
   Line(-39,90,-44,95)
   Text(-46,86,"IQ3")
   Pen(,ByNode,,S1)
   Line(54,-90,39,-90)
   Text(46,-94,"S1")
   Pen(,,,S2)
   Line(54,-54,39,-54)
   Text(46,-58,"S2")
   Pen(,,,S3)
   Line(54,-18,39,-18)
   Text(46,-22,"S3")
   Pen(,,,S4)
   Line(54,18,39,18)
   Text(46,14,"S4")
   Pen(,,,S5)
   Line(54,54,39,54)
   Text(46,50,"S5")
   Pen(,,,S6)
   Line(54,90,39,90)
   Text(46,86,"S6")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([54,108],0,0,-1)
    {
    Name = "Pcommand3"
    }
   0.import([54,144],0,0,-1)
    {
    Name = "Qcommand3"
    }
   0.import([54,180],0,0,-1)
    {
    Name = "VD3"
    }
   0.import([54,216],0,0,-1)
    {
    Name = "VQ3"
    }
   0.import([54,252],0,0,-1)
    {
    Name = "ID3"
    }
   0.import([54,288],0,0,-1)
    {
    Name = "IQ3"
    }
   0.export([198,108],4,0,-1)
    {
    Name = "S1"
    }
   0.export([198,144],4,0,-1)
    {
    Name = "S2"
    }
   0.export([198,180],4,0,-1)
    {
    Name = "S3"
    }
   0.export([198,216],4,0,-1)
    {
    Name = "S4"
    }
   0.export([198,252],4,0,-1)
    {
    Name = "S5"
    }
   0.export([198,288],4,0,-1)
    {
    Name = "S6"
    }
   }
  }
 Component("pscad_recv")
  {
  Desc = ""
  FileDate = 0
  Parameters = 
   {
   Category("Configuration")
    {
    Input("ndata",6,4,180,12,2)
     {
     Desc = "Number of Outputs"
     Def = "1"
     Data = Integer
     Len = 15
     Min = 1
     }
    Input("IP1",6,18,180,12,2)
     {
     Desc = "IP Address (1)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("IP2",6,32,180,12,2)
     {
     Desc = "IP Address (2)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("IP3",6,46,180,12,2)
     {
     Desc = "IP Address (3)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("IP4",6,60,180,12,2)
     {
     Desc = "IP Address (4)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("port",6,74,180,12,2)
     {
     Desc = "Port Number"
     Data = Integer
     Len = 15
     Min = 0
     Max = 65535
     }
    Input("dt",6,88,180,12,2)
     {
     Desc = "Time Step"
     Data = Real
     Len = 15
     Min = 0
     }
    Input("delay",6,102,180,12,2)
     {
     Desc = "Time Delay"
     Data = Real
     Len = 15
     Min = 0
     }
    }
   }

  Nodes = 
   {
   Output("data:ndata",54,-18)
    {
    Type = Real
    Dim  = [0]
    }
   Output("status",54,18)
    {
    Type = Integer
    }
   Input("init:ndata",-54,0)
    {
    Type = Real
    Dim  = [0]
    }
   }

  Graphics = 
   {
   Rectangle(-39,-39,39,39)
   Pen(ByNode,ByNode,ByNode,status)
   Line(54,-18,39,-18)
   Text(52,-22,"data")
   Line(54,18,39,18)
   Text(57,13,"status")
   Text(-1,3,"pscad_recv")
   Line(-54,0,-39,0)
   Pen(,Solid)
   Line(-39,0,-44,-5)
   Line(-39,0,-44,5)
   Text(-55,-5,"initial")
   }

  Section(EMTDC)
   {
   Fortran = "\
      IF (TIMEZERO) THEN
            DO 10, I = 1, $NDATA
                  $DATA(I) = $INIT(I)
10          CONTINUE
      ELSE IF (NINT(TIME/DELT) == NINT($DELAY/DELT)) THEN
            CALL PSCAD_RECV_INIT($IP1,$IP2,$IP3,$IP4,$PORT,$STATUS)
      ELSE IF (TIME > $DELAY) THEN
            IF (LASTSTEP) THEN
                  CALL PSCAD_RECV_CLOSE($STATUS)
            ELSE IF (MOD(NINT(TIME/DELT),NINT($dt/DELT)) == 0) THEN
                  CALL PSCAD_RECV($IP1,$IP2,$IP3,$IP4,$PORT,$DATA,$NDATA,$STATUS)
            END IF
      END IF"
   Branch = ""
   Computations = ""
   }
  }
 Component("pscad_send")
  {
  Desc = ""
  FileDate = 0
  Parameters = 
   {
   Category("Configuration")
    {
    Input("ndata",6,4,180,12,2)
     {
     Desc = "Number of Inputs"
     Def = "1"
     Data = Integer
     Len = 15
     Min = 1
     }
    Input("IP1",6,18,180,12,2)
     {
     Desc = "IP Address (1)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("IP2",6,32,180,12,2)
     {
     Desc = "IP Address (2)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("IP3",6,46,180,12,2)
     {
     Desc = "IP Address (3)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("IP4",6,60,180,12,2)
     {
     Desc = "IP Address (4)"
     Data = Integer
     Len = 15
     Min = 0
     Max = 255
     }
    Input("port",6,74,180,12,2)
     {
     Desc = "Port Number"
     Data = Integer
     Len = 15
     Min = 0
     Max = 65535
     }
    Input("dt",6,88,180,12,2)
     {
     Desc = "Time Step"
     Data = Real
     Len = 15
     Min = 0
     }
    Input("delay",6,102,180,12,2)
     {
     Desc = "Time Delay"
     Data = Real
     Len = 15
     Min = 0
     }
    }
   }

  Nodes = 
   {
   Output("status",54,0)
    {
    Type = Integer
    }
   Input("data:ndata",-54,0)
    {
    Type = Real
    Dim  = [0]
    }
   }

  Graphics = 
   {
   Rectangle(-39,-39,39,39)
   Pen(ByNode,ByNode,ByNode,data:ndata)
   Line(54,0,39,0)
   Text(57,-5,"status")
   Text(0,3,"pscad_send")
   Line(-54,0,-39,0)
   Pen(,Solid)
   Line(-39,0,-44,-5)
   Line(-39,0,-44,5)
   Text(-52,-5,"data")
   }

  Section(EMTDC)
   {
   Fortran = "\
      IF (NINT(TIME/DELT) == NINT($DELAY/DELT)) THEN
            CALL PSCAD_SEND_INIT($IP1,$IP2,$IP3,$IP4,$PORT,$DATA,$NDATA,$STATUS)
      ELSE IF (TIME > $DELAY) THEN
            IF (LASTSTEP) THEN
                  CALL PSCAD_SEND_CLOSE($STATUS)
            ELSE IF (MOD(NINT(TIME/DELT),NINT($dt/DELT)) == 0) THEN
                  CALL PSCAD_SEND($IP1,$IP2,$IP3,$IP4,$PORT,$DATA,$NDATA,$STATUS)
            END IF
      END IF"
   Branch = ""
   Computations = ""
   }
  }
 Module("INVERTER3CONTROL_1")
  {
  Desc = ""
  FileDate = 1323119314
  Nodes = 
   {
   Input("Pcommand3",-54,-162)
    {
    Type = Real
    }
   Input("Qcommand3",-54,-126)
    {
    Type = Real
    }
   Input("VD3",-54,-90)
    {
    Type = Real
    }
   Input("VQ3",-54,-54)
    {
    Type = Real
    }
   Input("ID3",-54,-18)
    {
    Type = Real
    }
   Input("IQ3",-54,18)
    {
    Type = Real
    }
   Input("I03",-54,54)
    {
    Type = Real
    }
   Input("I3A",-54,90)
    {
    Type = Real
    }
   Input("I3B",-54,126)
    {
    Type = Real
    }
   Input("I3C",-54,162)
    {
    Type = Real
    }
   Output("S1",54,-90)
    {
    Type = Real
    }
   Output("S2",54,-54)
    {
    Type = Real
    }
   Output("S3",54,-18)
    {
    Type = Real
    }
   Output("S4",54,18)
    {
    Type = Real
    }
   Output("S5",54,54)
    {
    Type = Real
    }
   Output("S6",54,90)
    {
    Type = Real
    }
   Output("P3",54,126)
    {
    Type = Real
    }
   Output("Q3",54,162)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-183,39,183)
   Text(0,-85,"INVERTER")
   Text(0,6,"3 ")
   Text(0,97,"CONTROL")
   Pen(ByNode,ByNode,ByNode,Pcommand3)
   Line(-54,-162,-39,-162)
   Pen(,Solid)
   Line(-39,-162,-44,-167)
   Line(-39,-162,-44,-157)
   Text(-46,-166,"Pcomm3")
   Pen(,ByNode,,Qcommand3)
   Line(-54,-126,-39,-126)
   Pen(,Solid)
   Line(-39,-126,-44,-131)
   Line(-39,-126,-44,-121)
   Text(-46,-130,"Qcomm3")
   Pen(,ByNode,,VD3)
   Line(-54,-90,-39,-90)
   Pen(,Solid)
   Line(-39,-90,-44,-95)
   Line(-39,-90,-44,-85)
   Text(-46,-94,"VD3")
   Pen(,ByNode,,VQ3)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"VQ3")
   Pen(,ByNode,,ID3)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"ID3")
   Pen(,ByNode,,IQ3)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"IQ3")
   Pen(,ByNode,,I03)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"I03")
   Pen(,ByNode,,I3A)
   Line(-54,90,-39,90)
   Pen(,Solid)
   Line(-39,90,-44,85)
   Line(-39,90,-44,95)
   Text(-46,86,"I3A")
   Pen(,ByNode,,I3B)
   Line(-54,126,-39,126)
   Pen(,Solid)
   Line(-39,126,-44,121)
   Line(-39,126,-44,131)
   Text(-46,122,"I3B")
   Pen(,ByNode,,I3C)
   Line(-54,162,-39,162)
   Pen(,Solid)
   Line(-39,162,-44,157)
   Line(-39,162,-44,167)
   Text(-46,158,"I3C")
   Pen(,ByNode,,S1)
   Line(54,-90,39,-90)
   Text(46,-94,"S1")
   Pen(,,,S2)
   Line(54,-54,39,-54)
   Text(46,-58,"S2")
   Pen(,,,S3)
   Line(54,-18,39,-18)
   Text(46,-22,"S3")
   Pen(,,,S4)
   Line(54,18,39,18)
   Text(46,14,"S4")
   Pen(,,,S5)
   Line(54,54,39,54)
   Text(46,50,"S5")
   Pen(Real,Real,0.2)
   Line(54,90,39,90)
   Text(46,86,"S6")
   Line(54,126,39,126)
   Text(46,122,"P3")
   Line(54,162,39,162)
   Text(46,158,"Q3")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([54,108],0,0,-1)
    {
    Name = "Pcommand3"
    }
   0.import([54,144],0,0,-1)
    {
    Name = "Qcommand3"
    }
   0.import([54,180],0,0,-1)
    {
    Name = "VD3"
    }
   0.import([54,216],0,0,-1)
    {
    Name = "VQ3"
    }
   0.import([54,252],0,0,-1)
    {
    Name = "ID3"
    }
   0.import([54,288],0,0,-1)
    {
    Name = "IQ3"
    }
   0.import([54,324],0,0,20)
    {
    Name = "I03"
    }
   0.import([54,360],0,0,60)
    {
    Name = "I3A"
    }
   0.import([54,396],0,0,70)
    {
    Name = "I3B"
    }
   0.import([54,432],0,0,100)
    {
    Name = "I3C"
    }
   0.export([198,108],4,0,850)
    {
    Name = "S1"
    }
   0.export([198,144],4,0,830)
    {
    Name = "S2"
    }
   0.export([198,180],4,0,820)
    {
    Name = "S3"
    }
   0.export([198,216],4,0,810)
    {
    Name = "S4"
    }
   0.export([198,252],4,0,800)
    {
    Name = "S5"
    }
   0.export([198,288],4,0,770)
    {
    Name = "S6"
    }
   0.import([576,180],0,0,-1)
    {
    Name = "Pcommand3"
    }
   0.import([612,324],0,0,-1)
    {
    Name = "VD3"
    }
   0.import([612,216],0,0,-1)
    {
    Name = "VQ3"
    }
   0.import([936,450],0,0,-1)
    {
    Name = "IQ3"
    }
   0.sumjct([1008,324],0,0,450)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.mult([936,324],0,0,310)
    {
    }
   0.mult([972,414],0,0,320)
    {
    }
   -Wire-([1008,414],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([1080,324],0,0,550)
    {
    }
   0.const([1080,396],3,0,110)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1116,324],0,0,-1)
    {
    Name = "P3"
    }
   0.sumjct([1152,504],0,0,460)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.mult([1080,504],0,0,330)
    {
    }
   0.mult([1116,594],0,0,360)
    {
    }
   -Wire-([1152,594],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([1224,504],0,0,540)
    {
    }
   0.const([1224,576],3,0,180)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1260,504],0,0,-1)
    {
    Name = "Q3"
    }
   0.datalabel([702,360],0,0,-1)
    {
    Name = "VD3"
    }
   0.datalabel([630,360],0,0,-1)
    {
    Name = "VQ3"
    }
   0.pgb([702,360],0,57732312,690)
    {
    Name = "VD1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([630,360],0,57732720,700)
    {
    Name = "VQ1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([882,54],0,0,-1)
    {
    Name = "IDc3"
    }
   0.pgb([882,54],0,57733536,840)
    {
    Name = "IDc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([720,432],0,0,-1)
    {
    Name = "I3Ac"
    }
   0.datalabel([720,468],0,0,-1)
    {
    Name = "I3Bc"
    }
   0.datalabel([720,504],0,0,-1)
    {
    Name = "I3Cc"
    }
   0.pgb([720,432],0,57735168,660)
    {
    Name = "I3Ac"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([720,468],0,57735576,630)
    {
    Name = "I3Bc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([720,504],0,57735984,610)
    {
    Name = "I3Cc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1188,288],0,0,-1)
    {
    Name = "P3"
    }
   0.pgb([1188,288],0,57736800,710)
    {
    Name = "P"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([756,324],0,0,-1)
    {
    Name = "Q3"
    }
   0.pgb([756,324],0,57737616,720)
    {
    Name = "Q"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1134,126],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([1098,126],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([1206,18],0,0,400)
    {
    }
   0.const([1206,90],3,0,10)
    {
    Name = ""
    Value = "2"
    }
   0.div([864,180],0,0,440)
    {
    }
   0.sumjct([720,180],0,0,420)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   -Wire-([720,288],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([684,288],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([792,180],0,0,430)
    {
    }
   0.const([792,252],3,0,30)
    {
    Name = ""
    Value = "2"
    }
   0.datalabel([864,216],0,0,-1)
    {
    Name = "Z3"
    }
   0.datalabel([900,180],0,0,-1)
    {
    Name = "IQc3"
    }
   -Wire-([684,216],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([684,216],0,0,-1)
    {
    Name = "M3"
    }
   0.mult([648,180],0,0,290)
    {
    }
   0.datalabel([990,198],0,0,-1)
    {
    Name = "M3"
    }
   0.pgb([990,198],0,60060560,780)
    {
    Name = "S"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([648,288],0,0,300)
    {
    }
   -Wire-([684,324],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([684,324],0,0,-1)
    {
    Name = "T3"
    }
   0.datalabel([990,252],0,0,-1)
    {
    Name = "T3"
    }
   0.pgb([990,252],0,60062192,740)
    {
    Name = "T"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([828,198],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.datalabel([828,198],0,0,-1)
    {
    Name = "N3"
    }
   0.datalabel([1188,324],0,0,-1)
    {
    Name = "N3"
    }
   0.pgb([1188,324],0,60063416,680)
    {
    Name = "N"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1116,234],0,0,-1)
    {
    Name = "Z3"
    }
   0.pgb([1116,234],0,60064232,750)
    {
    Name = "Z"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1044,360],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([1044,360],0,0,-1)
    {
    Name = "A3"
    }
   0.datalabel([1206,414],0,0,-1)
    {
    Name = "A3"
    }
   0.pgb([1206,414],0,60065456,640)
    {
    Name = "A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([828,486],0,60065864,130)
    {
    Name = "I3B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([828,540],0,60066272,150)
    {
    Name = "I3C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([828,486],0,0,-1)
    {
    Name = "I3B"
    }
   0.datalabel([828,540],0,0,-1)
    {
    Name = "I3C"
    }
   -Wire-([1188,558],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1188,558],0,0,-1)
    {
    Name = "B3"
    }
   0.datalabel([1224,450],0,0,-1)
    {
    Name = "B3"
    }
   0.pgb([1224,450],0,60068312,620)
    {
    Name = "B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1242,72],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1242,72],0,0,-1)
    {
    Name = "C3"
    }
   0.datalabel([1224,162],0,0,-1)
    {
    Name = "C3"
    }
   0.pgb([1224,162],0,60069536,790)
    {
    Name = "C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([1062,18],0,0,270)
    {
    }
   0.mult([1062,126],0,0,280)
    {
    }
   0.import([990,18],0,0,-1)
    {
    Name = "Qcommand3"
    }
   0.import([990,126],0,0,-1)
    {
    Name = "Pcommand3"
    }
   0.import([1026,54],0,0,-1)
    {
    Name = "VQ3"
    }
   0.import([1026,162],0,0,-1)
    {
    Name = "VD3"
    }
   0.import([864,324],0,0,-1)
    {
    Name = "VD3"
    }
   0.import([900,360],0,0,-1)
    {
    Name = "ID3"
    }
   0.import([900,414],0,0,-1)
    {
    Name = "VQ3"
    }
   0.import([1008,504],0,0,140)
    {
    Name = "VQ3"
    }
   0.import([1044,540],0,0,160)
    {
    Name = "ID3"
    }
   0.import([1044,594],0,0,190)
    {
    Name = "VD3"
    }
   0.import([1080,630],0,0,220)
    {
    Name = "IQ3"
    }
   0.sumjct([666,756],0,0,480)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([630,756],2,0,-1)
    {
    Name = "I3Ac"
    }
   0.datalabel([666,792],0,0,-1)
    {
    Name = "I3A"
    }
   -Wire-([738,756],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([828,756],0,0,-1)
    {
    Name = "S1"
    }
   0.inv([792,792],0,0,600)
    {
    INTR = "0"
    }
   -Wire-([792,756],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([828,792],0,0,-1)
    {
    Name = "S2"
    }
   0.buffer([702,756],0,0,530)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.sumjct([918,738],0,0,490)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([882,738],2,0,-1)
    {
    Name = "I3Bc"
    }
   0.datalabel([918,774],0,0,-1)
    {
    Name = "I3B"
    }
   -Wire-([990,738],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([1080,738],0,0,-1)
    {
    Name = "S3"
    }
   0.inv([1044,774],0,0,590)
    {
    INTR = "0"
    }
   -Wire-([1044,738],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([1080,774],0,0,-1)
    {
    Name = "S4"
    }
   0.buffer([954,738],0,0,520)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.abcdq0([756,648],0,0,470)
    {
    IDir = "0"
    Theta = "Qf"
    }
   0.datalabel([720,630],0,0,-1)
    {
    Name = "IDc3"
    }
   0.datalabel([720,648],0,0,-1)
    {
    Name = "IQc3"
    }
   0.datalabel([792,630],0,0,-1)
    {
    Name = "I3Ac"
    }
   0.datalabel([792,648],0,0,-1)
    {
    Name = "I3Bc"
    }
   0.datalabel([792,666],0,0,-1)
    {
    Name = "I3Cc"
    }
   0.div([1278,18],0,0,410)
    {
    }
   0.sumjct([1134,18],0,0,390)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.datalabel([1314,18],0,0,-1)
    {
    Name = "IDc3"
    }
   0.datalabel([1278,54],0,0,-1)
    {
    Name = "Z3"
    }
   0.import([576,288],0,0,-1)
    {
    Name = "Qcommand3"
    }
   0.sumjct([1116,864],0,0,500)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([1080,864],2,0,-1)
    {
    Name = "I3Cc"
    }
   0.datalabel([1116,900],0,0,-1)
    {
    Name = "I3C"
    }
   -Wire-([1188,864],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([1278,864],0,0,-1)
    {
    Name = "S5"
    }
   0.inv([1242,900],0,0,560)
    {
    INTR = "0"
    }
   -Wire-([1242,864],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([1278,900],0,0,-1)
    {
    Name = "S6"
    }
   0.buffer([1152,864],0,0,510)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.datalabel([1296,540],6,0,-1)
    {
    Name = "VQ3"
    }
   0.datalabel([1332,576],0,0,-1)
    {
    Name = "VQ3"
    }
   0.datalabel([1296,630],6,0,-1)
    {
    Name = "VD3"
    }
   0.datalabel([1332,666],0,0,-1)
    {
    Name = "VD3"
    }
   0.sumjct([1404,540],0,0,340)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   -Wire-([1404,630],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([1368,630],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([1476,540],0,0,350)
    {
    }
   0.const([1476,612],3,0,230)
    {
    Name = ""
    Value = "3"
    }
   0.datalabel([1512,540],0,0,-1)
    {
    Name = "Z3"
    }
   0.mult([1332,540],0,0,170)
    {
    }
   0.mult([1332,630],0,0,250)
    {
    }
   0.mult([432,684],0,0,370)
    {
    }
   0.mult([504,684],0,0,380)
    {
    }
   0.emtconst([396,720],0,0,240)
    {
    Name = ""
    Value = "1"
    }
   0.const([468,720],0,0,260)
    {
    Name = ""
    Value = "60"
    }
   0.datalabel([540,684],0,0,-1)
    {
    Name = "Qf"
    }
   0.time-sig([360,684],0,0,200)
    {
    }
   0.datalabel([558,900],0,0,-1)
    {
    Name = "P3"
    }
   0.datalabel([558,936],0,0,-1)
    {
    Name = "Q3"
    }
   0.pgb([558,900],0,60227880,580)
    {
    Name = "P3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([558,936],0,60228288,570)
    {
    Name = "Q3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([828,432],0,60228696,120)
    {
    Name = "I3A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([828,432],0,0,-1)
    {
    Name = "I3A"
    }
   0.datalabel([468,450],0,0,-1)
    {
    Name = "IDc3"
    }
   0.datalabel([576,450],0,0,-1)
    {
    Name = "IQc3"
    }
   0.pgb([468,450],0,60230328,670)
    {
    Name = "IDc3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([576,450],0,60230736,650)
    {
    Name = "IQc3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.const([684,666],0,0,210)
    {
    Name = ""
    Value = "0"
    }
   0.import([324,378],0,0,80)
    {
    Name = "Qcommand3"
    }
   0.import([324,324],0,0,40)
    {
    Name = "Pcommand3"
    }
   0.pgb([360,324],0,60232368,50)
    {
    Name = "Pcommand3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([360,378],0,60232776,90)
    {
    Name = "Qcommand3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.export([198,324],4,0,760)
    {
    Name = "P3"
    }
   0.export([198,360],4,0,730)
    {
    Name = "Q3"
    }
   -Plot-([324,540],0)
    {
    Title = "Inverter 3 Currents"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [630,270]
    Icon = [324,540]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(60228696,"I3A",0,,,)
     Curve(60065864,"I3B",0,,,)
     Curve(60066272,"I3C",0,,,)
     }
    }
   -Plot-([324,558],0)
    {
    Title = "Inverter 3 Compensated Currents"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [270,252]
    Icon = [324,558]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(57735168,"I3Ac",0,,,)
     Curve(57735576,"I3Bc",0,,,)
     Curve(57735984,"I3Cc",0,,,)
     }
    }
   -Plot-([324,522],0)
    {
    Title = "Compensated dq3"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,306]
    Icon = [324,522]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(60230328,"IDc3",0,,,)
     Curve(60230736,"IQc3",0,,,)
     }
    }
   -XYPlot-(54,864,378,1224)
    {
    Title = "P3 v/s Q3"
    Options = 37
    State = 1
    Icon = 36,630
    Posn = 54,864
    Extents = 0,0,324,360
    Curve(60228288,"Q3",0,,,)
    Curve(60227880,"P3",0,,,)
    }
   -Plot-([288,180],0)
    {
    Title = "PQ3Commands"
    Draw = 1
    Area = [0,0,576,288]
    Posn = [288,180]
    Icon = [324,504]
    Extents = 0,0,576,288
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,576,225],"y")
     {
     Options = 128
     Units = ""
     Curve(60232368,"Pcommand3",0,,,)
     Curve(60232776,"Qcommand3",0,,,)
     }
    }
   -Plot-([324,576],0)
    {
    Title = "P3"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,738]
    Icon = [324,576]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(60227880,"P3",0,,,)
     }
    }
   -Plot-([324,594],0)
    {
    Title = "Q3"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,792]
    Icon = [324,594]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(60228288,"Q3",0,,,)
     }
    }
   }
  }
 Module("INVERTER2CONTROL_1")
  {
  Desc = ""
  FileDate = 1323119314
  Nodes = 
   {
   Input("Pcommand2",-54,-162)
    {
    Type = Real
    }
   Input("Qcommand2",-54,-126)
    {
    Type = Real
    }
   Input("VD2",-54,-90)
    {
    Type = Real
    }
   Input("VQ2",-54,-54)
    {
    Type = Real
    }
   Input("ID2",-54,-18)
    {
    Type = Real
    }
   Input("IQ2",-54,18)
    {
    Type = Real
    }
   Input("I02",-54,54)
    {
    Type = Real
    }
   Input("I2A",-54,90)
    {
    Type = Real
    }
   Input("I2B",-54,126)
    {
    Type = Real
    }
   Input("I2C",-54,162)
    {
    Type = Real
    }
   Output("S7",54,-90)
    {
    Type = Real
    }
   Output("S8",54,-54)
    {
    Type = Real
    }
   Output("S9",54,-18)
    {
    Type = Real
    }
   Output("S10",54,18)
    {
    Type = Real
    }
   Output("S11",54,54)
    {
    Type = Real
    }
   Output("S12",54,90)
    {
    Type = Real
    }
   Output("P2",54,126)
    {
    Type = Real
    }
   Output("Q2",54,162)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-183,39,183)
   Text(0,-85,"INVERTER")
   Text(0,6,"2")
   Text(0,97,"CONTROL")
   Pen(ByNode,ByNode,ByNode,Pcommand2)
   Line(-54,-162,-39,-162)
   Pen(,Solid)
   Line(-39,-162,-44,-167)
   Line(-39,-162,-44,-157)
   Text(-46,-166,"Pcomm2")
   Pen(,ByNode,,Qcommand2)
   Line(-54,-126,-39,-126)
   Pen(,Solid)
   Line(-39,-126,-44,-131)
   Line(-39,-126,-44,-121)
   Text(-46,-130,"Qcomm2")
   Pen(,ByNode,,VD2)
   Line(-54,-90,-39,-90)
   Pen(,Solid)
   Line(-39,-90,-44,-95)
   Line(-39,-90,-44,-85)
   Text(-46,-94,"VD2")
   Pen(,ByNode,,VQ2)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"VQ2")
   Pen(,ByNode,,ID2)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"ID2")
   Pen(,ByNode,,IQ2)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"IQ2")
   Pen(,ByNode,,I02)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"I02")
   Pen(,ByNode,,I2A)
   Line(-54,90,-39,90)
   Pen(,Solid)
   Line(-39,90,-44,85)
   Line(-39,90,-44,95)
   Text(-46,86,"I2A")
   Pen(,ByNode,,I2B)
   Line(-54,126,-39,126)
   Pen(,Solid)
   Line(-39,126,-44,121)
   Line(-39,126,-44,131)
   Text(-46,122,"I2B")
   Pen(,ByNode,,I2C)
   Line(-54,162,-39,162)
   Pen(,Solid)
   Line(-39,162,-44,157)
   Line(-39,162,-44,167)
   Text(-46,158,"I2Cl")
   Pen(,ByNode,,S7)
   Line(54,-90,39,-90)
   Text(46,-94,"S7")
   Pen(,,,S8)
   Line(54,-54,39,-54)
   Text(46,-58,"S8")
   Pen(,,,S9)
   Line(54,-18,39,-18)
   Text(46,-22,"S9")
   Pen(,,,S10)
   Line(54,18,39,18)
   Text(46,14,"S10")
   Pen(,,,S11)
   Line(54,54,39,54)
   Text(46,50,"S11")
   Pen(Real,Real,0.2)
   Line(54,90,39,90)
   Text(46,86,"S12")
   Line(54,126,39,126)
   Text(46,122,"P2")
   Line(54,162,39,162)
   Text(46,158,"Q2")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([54,108],0,0,-1)
    {
    Name = "Pcommand2"
    }
   0.import([468,288],0,0,-1)
    {
    Name = "Qcommand2"
    }
   0.import([54,180],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([54,216],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([54,252],0,0,-1)
    {
    Name = "ID2"
    }
   0.import([54,288],0,0,-1)
    {
    Name = "IQ2"
    }
   0.import([54,324],0,0,50)
    {
    Name = "I02"
    }
   0.import([54,360],0,0,60)
    {
    Name = "I2A"
    }
   0.import([54,396],0,0,70)
    {
    Name = "I2B"
    }
   0.import([54,432],0,0,90)
    {
    Name = "I2C"
    }
   0.export([198,108],4,0,810)
    {
    Name = "S7"
    }
   0.export([198,144],4,0,790)
    {
    Name = "S8"
    }
   0.export([198,180],4,0,780)
    {
    Name = "S9"
    }
   0.export([198,216],4,0,770)
    {
    Name = "S10"
    }
   0.export([198,252],4,0,760)
    {
    Name = "S11"
    }
   0.export([198,288],4,0,730)
    {
    Name = "S12"
    }
   0.import([468,180],0,0,-1)
    {
    Name = "Pcommand2"
    }
   0.import([504,324],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([504,216],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([828,450],0,0,-1)
    {
    Name = "IQ2"
    }
   0.sumjct([900,324],0,0,410)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.mult([828,324],0,0,280)
    {
    }
   0.mult([864,414],0,0,300)
    {
    }
   -Wire-([900,414],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([972,324],0,0,520)
    {
    }
   0.const([972,396],3,0,100)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1008,324],0,0,-1)
    {
    Name = "P2"
    }
   0.sumjct([1044,504],0,0,510)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.mult([972,504],0,0,310)
    {
    }
   0.mult([1008,594],0,0,320)
    {
    }
   -Wire-([1044,594],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([1116,504],0,0,560)
    {
    }
   0.const([1116,576],3,0,170)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1152,504],0,0,-1)
    {
    Name = "Q2"
    }
   0.datalabel([594,360],0,0,-1)
    {
    Name = "VD2"
    }
   0.datalabel([522,360],0,0,-1)
    {
    Name = "VQ2"
    }
   0.pgb([594,360],0,60247872,650)
    {
    Name = "VD1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([522,360],0,60248280,660)
    {
    Name = "VQ1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([774,54],0,0,-1)
    {
    Name = "IDc2"
    }
   0.pgb([774,54],0,60249096,800)
    {
    Name = "IDc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([612,432],0,0,-1)
    {
    Name = "I2Ac"
    }
   0.datalabel([612,468],0,0,-1)
    {
    Name = "I2Bc"
    }
   0.datalabel([612,504],0,0,-1)
    {
    Name = "I2Cc"
    }
   0.pgb([612,432],0,60250728,620)
    {
    Name = "I2Ac"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([612,468],0,60251136,590)
    {
    Name = "I2Bc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([612,504],0,60251544,570)
    {
    Name = "I2Cc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1080,288],0,0,-1)
    {
    Name = "P2"
    }
   0.pgb([1080,288],0,60252360,670)
    {
    Name = "P"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([648,324],0,0,-1)
    {
    Name = "Q2"
    }
   0.pgb([648,324],0,60253176,680)
    {
    Name = "Q"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1026,126],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([990,126],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([1098,18],0,0,360)
    {
    }
   0.const([1098,90],3,0,20)
    {
    Name = ""
    Value = "2"
    }
   0.div([756,180],0,0,430)
    {
    }
   0.sumjct([612,180],0,0,370)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   -Wire-([612,288],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([576,288],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([684,180],0,0,380)
    {
    }
   0.const([684,252],3,0,40)
    {
    Name = ""
    Value = "2"
    }
   0.datalabel([756,216],0,0,-1)
    {
    Name = "Z2"
    }
   0.datalabel([792,180],0,0,-1)
    {
    Name = "IQc2"
    }
   -Wire-([576,216],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([576,216],0,0,-1)
    {
    Name = "M2"
    }
   0.mult([540,180],0,0,250)
    {
    }
   0.datalabel([882,198],0,0,-1)
    {
    Name = "M2"
    }
   0.pgb([882,198],0,60258072,740)
    {
    Name = "S"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([540,288],0,0,270)
    {
    }
   -Wire-([576,324],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([576,324],0,0,-1)
    {
    Name = "T2"
    }
   0.datalabel([882,252],0,0,-1)
    {
    Name = "T2"
    }
   0.pgb([882,252],0,60259704,710)
    {
    Name = "T"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([720,198],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.datalabel([720,198],0,0,-1)
    {
    Name = "N2"
    }
   0.datalabel([1080,324],0,0,-1)
    {
    Name = "N2"
    }
   0.pgb([1080,324],0,60260928,640)
    {
    Name = "N"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1008,234],0,0,-1)
    {
    Name = "Z2"
    }
   0.pgb([1008,234],0,60261744,720)
    {
    Name = "Z"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([936,360],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([936,360],0,0,-1)
    {
    Name = "A2"
    }
   0.datalabel([1098,414],0,0,-1)
    {
    Name = "A2"
    }
   0.pgb([1098,414],0,60262968,600)
    {
    Name = "A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([720,486],0,60263376,120)
    {
    Name = "I2B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([720,540],0,60263784,140)
    {
    Name = "I2C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([720,486],0,0,-1)
    {
    Name = "I2B"
    }
   0.datalabel([720,540],0,0,-1)
    {
    Name = "I2C"
    }
   -Wire-([1080,558],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1080,558],0,0,-1)
    {
    Name = "B2"
    }
   0.datalabel([1116,450],0,0,-1)
    {
    Name = "B2"
    }
   0.pgb([1116,450],0,60265824,580)
    {
    Name = "B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1134,72],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1134,72],0,0,-1)
    {
    Name = "C2"
    }
   0.datalabel([1116,162],0,0,-1)
    {
    Name = "C2"
    }
   0.pgb([1116,162],0,60267048,750)
    {
    Name = "C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([954,18],0,0,230)
    {
    }
   0.mult([954,126],0,0,240)
    {
    }
   0.import([882,18],0,0,10)
    {
    Name = "Qcommand2"
    }
   0.import([882,126],0,0,30)
    {
    Name = "Pcommand2"
    }
   0.import([918,54],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([918,162],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([756,324],0,0,-1)
    {
    Name = "VD2"
    }
   0.import([792,360],0,0,-1)
    {
    Name = "ID2"
    }
   0.import([792,414],0,0,-1)
    {
    Name = "VQ2"
    }
   0.import([900,504],0,0,130)
    {
    Name = "VQ2"
    }
   0.import([936,540],0,0,150)
    {
    Name = "ID2"
    }
   0.import([936,594],0,0,160)
    {
    Name = "VD2"
    }
   0.import([972,630],0,0,190)
    {
    Name = "IQ2"
    }
   0.sumjct([558,756],0,0,470)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([522,756],2,0,-1)
    {
    Name = "I2Ac"
    }
   0.datalabel([558,792],0,0,-1)
    {
    Name = "I2A"
    }
   -Wire-([630,756],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([720,756],0,0,-1)
    {
    Name = "S8"
    }
   0.inv([684,792],0,0,550)
    {
    INTR = "0"
    }
   -Wire-([684,756],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([720,792],0,0,-1)
    {
    Name = "S7"
    }
   0.buffer([594,756],0,0,500)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.sumjct([810,738],0,0,460)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([774,738],2,0,-1)
    {
    Name = "I2Bc"
    }
   0.datalabel([810,774],0,0,-1)
    {
    Name = "I2B"
    }
   -Wire-([882,738],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([972,738],0,0,-1)
    {
    Name = "S10"
    }
   0.inv([936,774],0,0,540)
    {
    INTR = "0"
    }
   -Wire-([936,738],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([972,774],0,0,-1)
    {
    Name = "S9"
    }
   0.buffer([846,738],0,0,490)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.abcdq0([648,648],0,0,440)
    {
    IDir = "0"
    Theta = "Qd"
    }
   0.datalabel([612,630],0,0,-1)
    {
    Name = "IDc2"
    }
   0.datalabel([612,648],0,0,-1)
    {
    Name = "IQc2"
    }
   0.datalabel([684,630],0,0,-1)
    {
    Name = "I2Ac"
    }
   0.datalabel([684,648],0,0,-1)
    {
    Name = "I2Bc"
    }
   0.datalabel([684,666],0,0,-1)
    {
    Name = "I2Cc"
    }
   0.div([1170,18],0,0,420)
    {
    }
   0.sumjct([1026,18],0,0,350)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.datalabel([1206,18],0,0,-1)
    {
    Name = "IDc2"
    }
   0.datalabel([1170,54],0,0,-1)
    {
    Name = "Z2"
    }
   0.sumjct([702,882],0,0,450)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([666,882],2,0,-1)
    {
    Name = "I2Cc"
    }
   0.datalabel([702,918],0,0,-1)
    {
    Name = "I2C"
    }
   -Wire-([774,882],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([864,882],0,0,-1)
    {
    Name = "S12"
    }
   0.inv([828,918],0,0,530)
    {
    INTR = "0"
    }
   -Wire-([828,882],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([864,918],0,0,-1)
    {
    Name = "S11"
    }
   0.buffer([738,882],0,0,480)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.datalabel([1296,198],6,0,-1)
    {
    Name = "VQ2"
    }
   0.datalabel([1332,234],0,0,-1)
    {
    Name = "VQ2"
    }
   0.datalabel([1296,288],6,0,-1)
    {
    Name = "VD2"
    }
   0.datalabel([1332,324],0,0,-1)
    {
    Name = "VD2"
    }
   0.sumjct([1404,198],0,0,390)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   -Wire-([1404,288],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([1368,288],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([1476,198],0,0,400)
    {
    }
   0.const([1476,270],3,0,80)
    {
    Name = ""
    Value = "3"
    }
   0.datalabel([1512,198],0,0,-1)
    {
    Name = "Z2"
    }
   0.mult([1332,198],0,0,260)
    {
    }
   0.mult([1332,288],0,0,290)
    {
    }
   0.mult([1134,684],0,0,330)
    {
    }
   0.mult([1206,684],0,0,340)
    {
    }
   0.emtconst([1098,720],0,0,210)
    {
    Name = ""
    Value = "1"
    }
   0.const([1170,720],0,0,220)
    {
    Name = ""
    Value = "60"
    }
   0.datalabel([1242,684],0,0,-1)
    {
    Name = "Qd"
    }
   0.time-sig([1062,684],0,0,200)
    {
    }
   0.datalabel([306,450],0,0,-1)
    {
    Name = "P2"
    }
   0.datalabel([306,486],0,0,-1)
    {
    Name = "Q2"
    }
   0.pgb([306,450],0,60595992,630)
    {
    Name = "P2"
    Group = ""
    Display = "0"
    Scale = "1"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1.0"
    }
   0.pgb([306,486],0,60596400,610)
    {
    Name = "Q2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([702,432],0,60596808,110)
    {
    Name = "I2A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([702,432],0,0,-1)
    {
    Name = "I2A"
    }
   0.const([576,666],0,0,180)
    {
    Name = ""
    Value = "0"
    }
   0.pgb([90,108],0,60598032,820)
    {
    Name = "Pcommand2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -XYPlot-(18,630,306,648)
    {
    Title = "P2 v/s Q2"
    Options = 37
    State = 0
    Icon = 18,630
    Posn = 0,648
    Extents = 0,0,324,360
    Curve(60596400,"Q2",0,,,)
    Curve(60595992,"P2",0,,,)
    }
   0.export([198,324],4,0,700)
    {
    Name = "P2"
    }
   0.export([198,360],4,0,690)
    {
    Name = "Q2"
    }
   -Plot-([288,108],0)
    {
    Title = "Inverter 2 Currents"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [468,54]
    Icon = [288,108]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(60596808,"I2A",0,,,)
     Curve(60263376,"I2B",0,,,)
     Curve(60263784,"I2C",0,,,)
     }
    }
   -Plot-([288,126],0)
    {
    Title = "Inverter 2 Compensated Currents"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [324,306]
    Icon = [288,126]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(60250728,"I2Ac",0,,,)
     Curve(60251136,"I2Bc",0,,,)
     Curve(60251544,"I2Cc",0,,,)
     }
    }
   -Plot-([288,144],0)
    {
    Title = "Pcommand2"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,144]
    Icon = [288,144]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(60598032,"Pcommand2",0,,,)
     }
    }
   -Plot-([288,72],0)
    {
    Title = "P2"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,306]
    Icon = [288,72]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(60595992,"P2",0,,,)
     }
    }
   -Plot-([288,90],0)
    {
    Title = "Q2"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,342]
    Icon = [288,90]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(60596400,"Q2",0,,,)
     }
    }
   }
  }
 Module("INVERTER1CONTROL_1")
  {
  Desc = ""
  FileDate = 1323119314
  Nodes = 
   {
   Input("Pcommand1",-54,-162)
    {
    Type = Real
    }
   Input("Qcommand1",-54,-126)
    {
    Type = Real
    }
   Input("VD1",-54,-90)
    {
    Type = Real
    }
   Input("VQ1",-54,-54)
    {
    Type = Real
    }
   Input("I01",-54,-18)
    {
    Type = Real
    }
   Input("I1A",-54,18)
    {
    Type = Real
    }
   Input("I1B",-54,54)
    {
    Type = Real
    }
   Input("I1C",-54,90)
    {
    Type = Real
    }
   Input("ID1",-54,126)
    {
    Type = Real
    }
   Input("IQ1",-54,162)
    {
    Type = Real
    }
   Output("S13",54,-90)
    {
    Type = Real
    }
   Output("S14",54,-54)
    {
    Type = Real
    }
   Output("S15",54,-18)
    {
    Type = Real
    }
   Output("S16",54,18)
    {
    Type = Real
    }
   Output("S17",54,54)
    {
    Type = Real
    }
   Output("S18",54,90)
    {
    Type = Real
    }
   Output("P1",54,126)
    {
    Type = Real
    }
   Output("Q1",54,162)
    {
    Type = Real
    }
   }

  Graphics = 
   {
   Rectangle(-39,-183,39,183)
   Text(0,-85,"INVERTER")
   Text(0,6,"1 ")
   Text(0,97,"CONTROL")
   Pen(ByNode,ByNode,ByNode,Pcommand1)
   Line(-54,-162,-39,-162)
   Pen(,Solid)
   Line(-39,-162,-44,-167)
   Line(-39,-162,-44,-157)
   Text(-46,-166,"Pcomm1")
   Pen(,ByNode,,Qcommand1)
   Line(-54,-126,-39,-126)
   Pen(,Solid)
   Line(-39,-126,-44,-131)
   Line(-39,-126,-44,-121)
   Text(-46,-130,"Qcomm1")
   Pen(,ByNode,,VD1)
   Line(-54,-90,-39,-90)
   Pen(,Solid)
   Line(-39,-90,-44,-95)
   Line(-39,-90,-44,-85)
   Text(-46,-94,"VD1")
   Pen(,ByNode,,VQ1)
   Line(-54,-54,-39,-54)
   Pen(,Solid)
   Line(-39,-54,-44,-59)
   Line(-39,-54,-44,-49)
   Text(-46,-58,"VQ1")
   Pen(,ByNode,,I01)
   Line(-54,-18,-39,-18)
   Pen(,Solid)
   Line(-39,-18,-44,-23)
   Line(-39,-18,-44,-13)
   Text(-46,-22,"I01")
   Pen(,ByNode,,I1A)
   Line(-54,18,-39,18)
   Pen(,Solid)
   Line(-39,18,-44,13)
   Line(-39,18,-44,23)
   Text(-46,14,"I1A")
   Pen(,ByNode,,I1B)
   Line(-54,54,-39,54)
   Pen(,Solid)
   Line(-39,54,-44,49)
   Line(-39,54,-44,59)
   Text(-46,50,"I1B")
   Pen(,ByNode,,I1C)
   Line(-54,90,-39,90)
   Pen(,Solid)
   Line(-39,90,-44,85)
   Line(-39,90,-44,95)
   Text(-46,86,"I1C")
   Pen(,ByNode,,ID1)
   Line(-54,126,-39,126)
   Pen(,Solid)
   Line(-39,126,-44,121)
   Line(-39,126,-44,131)
   Text(-46,122,"ID1")
   Pen(,ByNode,,IQ1)
   Line(-54,162,-39,162)
   Pen(,Solid)
   Line(-39,162,-44,157)
   Line(-39,162,-44,167)
   Text(-46,158,"IQ1")
   Pen(,ByNode,,S13)
   Line(54,-90,39,-90)
   Text(46,-94,"S13")
   Pen(,,,S14)
   Line(54,-54,39,-54)
   Text(46,-58,"S14")
   Pen(,,,S15)
   Line(54,-18,39,-18)
   Text(46,-22,"S15")
   Pen(,,,S16)
   Line(54,18,39,18)
   Text(46,14,"S16")
   Pen(,,,S17)
   Line(54,54,39,54)
   Text(46,50,"S17")
   Pen(Real,Real,0.2)
   Line(54,90,39,90)
   Text(46,86,"S18")
   Pen(,Solid)
   Line(39,126,53,126)
   Line(39,162,53,162)
   Text(46,121,"P1")
   Text(47,157,"Q1")
   }


  Page(A/A4,Landscape,16,[840,484],5)
   {
   0.import([54,108],0,0,-1)
    {
    Name = "Pcommand1"
    }
   0.import([54,144],0,0,-1)
    {
    Name = "Qcommand1"
    }
   0.import([54,180],0,0,-1)
    {
    Name = "VD1"
    }
   0.import([54,216],0,0,-1)
    {
    Name = "VQ1"
    }
   0.import([54,252],0,0,30)
    {
    Name = "I01"
    }
   0.import([54,288],0,0,40)
    {
    Name = "I1A"
    }
   0.import([54,324],0,0,50)
    {
    Name = "I1B"
    }
   0.import([54,360],0,0,80)
    {
    Name = "I1C"
    }
   0.import([54,396],0,0,-1)
    {
    Name = "ID1"
    }
   0.import([54,432],0,0,-1)
    {
    Name = "IQ1"
    }
   0.export([198,108],4,0,810)
    {
    Name = "S13"
    }
   0.export([198,144],4,0,790)
    {
    Name = "S14"
    }
   0.export([198,180],4,0,780)
    {
    Name = "S15"
    }
   0.export([198,216],4,0,770)
    {
    Name = "S16"
    }
   0.export([198,252],4,0,760)
    {
    Name = "S17"
    }
   0.export([198,288],4,0,730)
    {
    Name = "S18"
    }
   0.import([684,180],0,0,-1)
    {
    Name = "Pcommand1"
    }
   0.import([720,324],0,0,-1)
    {
    Name = "VD1"
    }
   0.import([720,216],0,0,-1)
    {
    Name = "VQ1"
    }
   0.import([1044,450],0,0,-1)
    {
    Name = "IQ1"
    }
   0.sumjct([1116,324],0,0,510)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.mult([1044,324],0,0,290)
    {
    }
   0.mult([1080,414],0,0,300)
    {
    }
   -Wire-([1116,414],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([1188,324],0,0,620)
    {
    }
   0.const([1188,396],3,0,90)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1224,324],0,0,-1)
    {
    Name = "P1"
    }
   0.sumjct([1260,504],0,0,500)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.mult([1188,504],0,0,330)
    {
    }
   0.mult([1224,594],0,0,340)
    {
    }
   -Wire-([1260,594],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.mult([1332,504],0,0,550)
    {
    }
   0.const([1332,576],3,0,180)
    {
    Name = ""
    Value = "1.5"
    }
   0.datalabel([1368,504],0,0,-1)
    {
    Name = "Q1"
    }
   0.datalabel([810,360],0,0,-1)
    {
    Name = "VD1"
    }
   0.datalabel([738,360],0,0,-1)
    {
    Name = "VQ1"
    }
   0.pgb([810,360],0,60613128,630)
    {
    Name = "VD1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([738,360],0,60613536,640)
    {
    Name = "VQ1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([990,54],0,0,-1)
    {
    Name = "IDc1"
    }
   0.pgb([990,54],0,60614352,800)
    {
    Name = "IDc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([828,432],0,0,-1)
    {
    Name = "I1Ac"
    }
   0.datalabel([828,468],0,0,-1)
    {
    Name = "I1Bc"
    }
   0.datalabel([828,504],0,0,-1)
    {
    Name = "I1Cc"
    }
   0.pgb([828,432],0,60615984,600)
    {
    Name = "I1Ac"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([828,468],0,60616392,580)
    {
    Name = "I1Bc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([828,504],0,60616800,560)
    {
    Name = "I1Cc"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1296,288],0,0,-1)
    {
    Name = "P1"
    }
   0.pgb([1296,288],0,60617616,650)
    {
    Name = "P"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([864,324],0,0,-1)
    {
    Name = "Q1"
    }
   0.pgb([864,324],0,60618432,660)
    {
    Name = "Q"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1242,126],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([1206,126],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([1314,18],0,0,380)
    {
    }
   0.const([1314,90],3,0,10)
    {
    Name = ""
    Value = "2"
    }
   0.div([972,180],0,0,420)
    {
    }
   0.sumjct([828,180],0,0,400)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   -Wire-([828,288],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([792,288],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([900,180],0,0,410)
    {
    }
   0.const([900,252],3,0,60)
    {
    Name = ""
    Value = "2"
    }
   0.datalabel([972,216],0,0,-1)
    {
    Name = "Z1"
    }
   0.datalabel([1008,180],0,0,-1)
    {
    Name = "IQc1"
    }
   -Wire-([792,216],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([792,216],0,0,-1)
    {
    Name = "M1"
    }
   0.mult([756,180],0,0,270)
    {
    }
   0.datalabel([1098,198],0,0,-1)
    {
    Name = "M1"
    }
   0.pgb([1098,198],0,60623328,720)
    {
    Name = "S"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([756,288],0,0,280)
    {
    }
   -Wire-([792,324],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([792,324],0,0,-1)
    {
    Name = "T1"
    }
   0.datalabel([1098,252],0,0,-1)
    {
    Name = "T1"
    }
   0.pgb([1098,252],0,60624960,680)
    {
    Name = "T"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([936,198],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.datalabel([936,198],0,0,-1)
    {
    Name = "N1"
    }
   0.datalabel([1296,324],0,0,-1)
    {
    Name = "N1"
    }
   0.pgb([1296,324],0,60626184,610)
    {
    Name = "N"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1224,234],0,0,-1)
    {
    Name = "Z1"
    }
   0.pgb([1224,234],0,60627000,690)
    {
    Name = "Z"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1152,360],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([1152,360],0,0,-1)
    {
    Name = "A1"
    }
   0.datalabel([1314,414],0,0,-1)
    {
    Name = "A1"
    }
   0.pgb([1314,414],0,60628224,590)
    {
    Name = "A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([936,486],0,60628632,110)
    {
    Name = "I1B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([936,540],0,60629040,140)
    {
    Name = "I1C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([936,486],0,0,-1)
    {
    Name = "I1B"
    }
   0.datalabel([936,540],0,0,-1)
    {
    Name = "I1C"
    }
   -Wire-([1296,558],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1296,558],0,0,-1)
    {
    Name = "B1"
    }
   0.datalabel([1332,450],0,0,-1)
    {
    Name = "B1"
    }
   0.pgb([1332,450],0,60631080,570)
    {
    Name = "B"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([1350,72],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1350,72],0,0,-1)
    {
    Name = "C1"
    }
   0.datalabel([1332,162],0,0,-1)
    {
    Name = "C1"
    }
   0.pgb([1332,162],0,60632304,750)
    {
    Name = "C"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([1170,18],0,0,250)
    {
    }
   0.mult([1170,126],0,0,260)
    {
    }
   0.import([1098,18],0,0,-1)
    {
    Name = "Qcommand1"
    }
   0.import([1098,126],0,0,20)
    {
    Name = "Pcommand1"
    }
   0.import([1134,54],0,0,-1)
    {
    Name = "VQ1"
    }
   0.import([1134,162],0,0,-1)
    {
    Name = "VD1"
    }
   0.import([972,324],0,0,-1)
    {
    Name = "VD1"
    }
   0.import([1008,360],0,0,-1)
    {
    Name = "ID1"
    }
   0.import([1008,414],0,0,-1)
    {
    Name = "VQ1"
    }
   0.import([1116,504],0,0,130)
    {
    Name = "VQ1"
    }
   0.import([1152,540],0,0,170)
    {
    Name = "ID1"
    }
   0.import([1152,594],0,0,190)
    {
    Name = "VD1"
    }
   0.import([1188,630],0,0,210)
    {
    Name = "IQ1"
    }
   0.sumjct([774,756],0,0,440)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([738,756],2,0,-1)
    {
    Name = "I1Ac"
    }
   0.datalabel([774,792],0,0,-1)
    {
    Name = "I1A"
    }
   -Wire-([846,756],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([936,756],0,0,-1)
    {
    Name = "S18"
    }
   0.inv([900,792],0,0,540)
    {
    INTR = "0"
    }
   -Wire-([900,756],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([936,792],0,0,-1)
    {
    Name = "S15"
    }
   0.buffer([810,756],0,0,490)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.sumjct([1026,738],0,0,450)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([990,738],2,0,-1)
    {
    Name = "I1Bc"
    }
   0.datalabel([1026,774],0,0,-1)
    {
    Name = "I1B"
    }
   -Wire-([1098,738],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([1188,738],0,0,-1)
    {
    Name = "S17"
    }
   0.inv([1152,774],0,0,530)
    {
    INTR = "0"
    }
   -Wire-([1152,738],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([1188,774],0,0,-1)
    {
    Name = "S14"
    }
   0.buffer([1062,738],0,0,480)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.abcdq0([864,648],0,0,430)
    {
    IDir = "0"
    Theta = "Qg"
    }
   0.datalabel([828,630],0,0,-1)
    {
    Name = "IDc1"
    }
   0.datalabel([828,648],0,0,-1)
    {
    Name = "IQc1"
    }
   0.datalabel([900,630],0,0,-1)
    {
    Name = "I1Ac"
    }
   0.datalabel([900,648],0,0,-1)
    {
    Name = "I1Bc"
    }
   0.datalabel([900,666],0,0,-1)
    {
    Name = "I1Cc"
    }
   0.div([1386,18],0,0,390)
    {
    }
   0.sumjct([1242,18],0,0,370)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   0.datalabel([1422,18],0,0,-1)
    {
    Name = "IDc1"
    }
   0.datalabel([1386,54],0,0,-1)
    {
    Name = "Z1"
    }
   0.sumjct([936,918],0,0,460)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([900,918],2,0,-1)
    {
    Name = "I1Cc"
    }
   0.datalabel([936,954],0,0,-1)
    {
    Name = "I1C"
    }
   -Wire-([1008,918],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([1098,918],0,0,-1)
    {
    Name = "S16"
    }
   0.inv([1062,954],0,0,520)
    {
    INTR = "0"
    }
   -Wire-([1062,918],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   0.datalabel([1098,954],0,0,-1)
    {
    Name = "S13"
    }
   0.buffer([972,918],0,0,470)
    {
    HI = "0.0001"
    LO = "-0.0001"
    Inv = "0"
    INTR = "0"
    }
   0.import([684,288],0,0,70)
    {
    Name = "Qcommand1"
    }
   0.datalabel([504,918],6,0,-1)
    {
    Name = "VQ1"
    }
   0.datalabel([540,954],0,0,-1)
    {
    Name = "VQ1"
    }
   0.datalabel([504,1008],6,0,-1)
    {
    Name = "VD1"
    }
   0.datalabel([540,1044],0,0,-1)
    {
    Name = "VD1"
    }
   0.sumjct([612,918],0,0,350)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "1"
    G = "0"
    }
   -Wire-([612,1008],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([576,1008],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.mult([684,918],0,0,360)
    {
    }
   0.const([684,990],3,0,230)
    {
    Name = ""
    Value = "3"
    }
   0.datalabel([720,918],0,0,-1)
    {
    Name = "Z1"
    }
   0.mult([540,918],0,0,220)
    {
    }
   0.mult([540,1008],0,0,240)
    {
    }
   0.mult([612,540],0,0,310)
    {
    }
   0.mult([684,540],0,0,320)
    {
    }
   0.emtconst([576,576],0,0,150)
    {
    Name = ""
    Value = "1"
    }
   0.const([648,576],0,0,160)
    {
    Name = ""
    Value = "60"
    }
   0.datalabel([720,540],0,0,-1)
    {
    Name = "Qg"
    }
   0.time-sig([540,540],0,0,120)
    {
    }
   0.datalabel([522,252],0,0,-1)
    {
    Name = "P1"
    }
   0.datalabel([522,288],0,0,-1)
    {
    Name = "Q1"
    }
   0.pgb([522,252],0,61000288,740)
    {
    Name = "P1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1.0"
    Max = "1.0"
    }
   0.pgb([522,288],0,61000696,710)
    {
    Name = "Q1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([918,450],0,61001104,100)
    {
    Name = "I1A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([918,450],0,0,-1)
    {
    Name = "I1A"
    }
   0.const([792,666],0,0,200)
    {
    Name = ""
    Value = "0"
    }
   0.export([198,324],2,0,700)
    {
    Name = "P1"
    }
   0.export([198,360],2,0,670)
    {
    Name = "Q1"
    }
   -Plot-([360,90],0)
    {
    Title = "Inverter 1 Currents"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [630,324]
    Icon = [360,90]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61001104,"I1A",0,,,)
     Curve(60628632,"I1B",0,,,)
     Curve(60629040,"I1C",0,,,)
     }
    }
   -Plot-([360,72],0)
    {
    Title = "Inverter 1 Compensated Currents"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [396,288]
    Icon = [360,72]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(60615984,"I1Ac",0,,,)
     Curve(60616392,"I1Bc",0,,,)
     Curve(60616800,"I1Cc",0,,,)
     }
    }
   -XYPlot-(360,108,648,126)
    {
    Title = "P1 v/s Q1"
    Options = 37
    State = 0
    Icon = 360,108
    Posn = 18,738
    Extents = 0,0,324,360
    Curve(61000696,"Q1",0,,,)
    Curve(61000288,"P1",0,,,)
    }
   -Plot-([360,36],0)
    {
    Title = "P1"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [288,144]
    Icon = [360,36]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 161
     Units = ""
     Curve(61000288,"P1",0,,,)
     }
    }
   -Plot-([288,144],0)
    {
    Title = "Q1"
    Draw = 1
    Area = [0,0,576,288]
    Posn = [288,144]
    Icon = [360,54]
    Extents = 0,0,576,288
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,576,225],"y")
     {
     Options = 128
     Units = ""
     Curve(61000696,"Q1",0,,,)
     }
    }
   }
  }
 Module("Main")
  {
  Desc = ""
  FileDate = 0
  Nodes = 
   {
   }

  Graphics = 
   {
   Rectangle(-18,-18,18,18)
   }


  Page(C/A2,Landscape,16,[840,484],5)
   {
   0.inductor([306,234],6,0,-1)
    {
    L = "0.1 [H]"
    }
   -Wire-([540,234],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   -Wire-([540,270],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   0.inductor([1422,270],0,0,-1)
    {
    L = "1 [H]"
    }
   0.inductor([1422,252],6,0,-1)
    {
    L = "1 [H]"
    }
   0.inductor([1422,630],6,0,-1)
    {
    L = "1 [H]"
    }
   0.source_1([234,792],1,0,320)
    {
    Name = "Source2"
    Type = "6"
    Grnd = "0"
    Spec = "0"
    Cntrl = "1"
    AC = "0"
    Vm = "132.79 [kV]"
    Tc = "0.05 [s]"
    Ph = "0.0 [deg]"
    f = "60.0 [Hz]"
    P = "0.0 [MW]"
    Q = "0.0 [MVAR]"
    R = "1.0 [ohm]"
    Rs = "1.0 [ohm]"
    Rp = "1.0 [ohm]"
    Lp = "0.1 [H]"
    R' = "1.0 [ohm]"
    L = "0.1 [H]"
    C = "1.0 [uF]"
    L' = "0.1 [H]"
    C' = "1.0 [uF]"
    CUR = ""
    }
   0.inductor([1422,306],0,0,-1)
    {
    L = "1 [H]"
    }
   0.datalabel([270,810],0,0,-1)
    {
    Name = "S1"
    }
   0.datalabel([270,972],0,0,-1)
    {
    Name = "S2"
    }
   0.datalabel([360,810],0,0,-1)
    {
    Name = "S3"
    }
   0.datalabel([360,972],0,0,-1)
    {
    Name = "S4"
    }
   0.datalabel([450,810],0,0,-1)
    {
    Name = "S5"
    }
   0.datalabel([450,972],0,0,-1)
    {
    Name = "S6"
    }
   0.datalabel([1512,342],0,0,-1)
    {
    Name = "S16"
    }
   0.datalabel([1584,342],0,0,-1)
    {
    Name = "S17"
    }
   0.datalabel([1674,342],0,0,-1)
    {
    Name = "S18"
    }
   0.var([1710,432],0,61009264,210)
    {
    Name = "Slider1"
    Group = ""
    Display = "1"
    Max = "1000"
    Min = "0"
    Value = "452.063492063"
    Units = "kV"
    Collect = "1"
    }
   0.var([234,666],4,61009672,250)
    {
    Name = "Slider3"
    Group = ""
    Display = "1"
    Max = "1000"
    Min = "0.0"
    Value = "492.063492063"
    Units = "kV"
    Collect = "1"
    }
   -Wire-([198,972],0,0,-1)
    {
    Vertex="0,0;0,-306"
    }
   0.peswitch([306,774],2,0,1520)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([306,936],2,0,1450)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([396,774],2,0,1510)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([396,936],2,0,1440)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([486,774],2,0,1500)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([486,936],2,0,1430)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1476,234],0,0,1630)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1476,378],0,0,1590)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1638,234],0,0,1610)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1638,378],0,0,1570)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.ammeter([1386,234],2,0,110)
    {
    Name = "I1C"
    }
   0.ammeter([1386,270],2,0,140)
    {
    Name = "I1B"
    }
   0.ammeter([1386,306],2,0,170)
    {
    Name = "I1A"
    }
   -Wire-([234,1008],0,0,-1)
    {
    Vertex="0,0;252,0"
    }
   -Wire-([1476,396],0,0,-1)
    {
    Vertex="0,0;234,0"
    }
   -Wire-([1476,144],0,0,-1)
    {
    Vertex="0,0;234,0"
    }
   -Wire-([1152,684],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([540,306],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   -Wire-([1008,648],0,0,-1)
    {
    Vertex="0,0;144,0"
    }
   0.ammeter([972,648],0,0,260)
    {
    Name = "I4C"
    }
   0.inductor([936,648],6,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([954,234],0,0,70)
    {
    Name = "I5C"
    }
   0.inductor([864,234],6,0,-1)
    {
    L = "0.1 [H]"
    }
   -Wire-([612,648],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   0.ammeter([1062,504],3,0,220)
    {
    Name = "I7A"
    }
   0.inductor([1062,504],1,0,-1)
    {
    L = "0.1 [H]"
    }
   -Wire-([1062,720],0,0,-1)
    {
    Vertex="0,0;0,-180"
    }
   -Wire-([612,774],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([648,684],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.inductor([936,684],6,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([972,684],0,0,290)
    {
    Name = "I4B"
    }
   -Wire-([1008,684],0,0,-1)
    {
    Vertex="0,0;144,0"
    }
   0.inductor([936,720],6,0,-1)
    {
    L = "0.1 [H]"
    }
   -Wire-([1008,720],0,0,-1)
    {
    Vertex="0,0;144,0"
    }
   0.ammeter([972,720],0,0,330)
    {
    Name = "I4A"
    }
   0.inductor([864,270],6,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([954,270],0,0,90)
    {
    Name = "I5B"
    }
   0.inductor([864,306],6,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([954,306],0,0,120)
    {
    Name = "I5A"
    }
   -Wire-([684,738],0,0,-1)
    {
    Vertex="0,0;0,-216"
    }
   0.inductor([1098,504],1,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([1098,504],3,0,230)
    {
    Name = "I7B"
    }
   0.inductor([1134,504],1,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([1134,504],3,0,240)
    {
    Name = "I7C"
    }
   -Wire-([1098,468],0,0,-1)
    {
    Vertex="0,0;0,-198"
    }
   -Wire-([1098,684],0,0,-1)
    {
    Vertex="0,0;0,-144"
    }
   -Wire-([1134,648],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1638,198],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([1458,270],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([1512,198],0,0,-1)
    {
    Name = "S13"
    }
   0.datalabel([1584,198],0,0,-1)
    {
    Name = "S14"
    }
   0.datalabel([1674,198],0,0,-1)
    {
    Name = "S15"
    }
   0.resistor([702,234],0,0,-1)
    {
    R = "0.1[ohm]"
    }
   -Wire-([738,234],0,0,-1)
    {
    Vertex="0,0;126,0"
    }
   0.resistor([702,270],0,0,-1)
    {
    R = "0.1 [ohm]"
    }
   -Wire-([738,270],0,0,-1)
    {
    Vertex="0,0;126,0"
    }
   0.resistor([702,306],0,0,-1)
    {
    R = "0.1 [ohm]"
    }
   -Wire-([738,306],0,0,-1)
    {
    Vertex="0,0;126,0"
    }
   0.resistor([738,648],6,0,-1)
    {
    R = "0.1 [ohm]"
    }
   -Wire-([774,648],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   0.resistor([738,684],6,0,-1)
    {
    R = "0.1[ohm]"
    }
   -Wire-([774,684],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   0.resistor([738,720],6,0,-1)
    {
    R = "0.1 [ohm]"
    }
   -Wire-([774,720],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   0.peswitch([324,810],0,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([414,810],0,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([504,810],0,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([324,972],0,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([414,972],0,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([504,972],0,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1458,198],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1458,342],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1530,342],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.ammeter([612,486],3,0,180)
    {
    Name = "I6A"
    }
   0.inductor([612,522],5,0,-1)
    {
    L = "0.1 [H]"
    }
   0.inductor([648,522],5,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([648,486],3,0,190)
    {
    Name = "I6B"
    }
   0.inductor([684,522],5,0,-1)
    {
    L = "0.1 [H]"
    }
   0.ammeter([684,486],3,0,200)
    {
    Name = "I6C"
    }
   -Wire-([612,306],0,0,-1)
    {
    Vertex="0,0;0,108"
    }
   -Wire-([684,234],0,0,-1)
    {
    Vertex="0,0;0,180"
    }
   -Wire-([900,234],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([990,234],0,0,-1)
    {
    Vertex="0,0;270,0"
    }
   -Wire-([900,270],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([990,270],0,0,-1)
    {
    Vertex="0,0;270,0"
    }
   -Wire-([900,306],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([612,450],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([684,450],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([648,756],0,0,-1)
    {
    Vertex="0,0;0,-234"
    }
   0.peswitch([1458,576],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1476,612],0,0,1560)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1458,720],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1476,756],0,0,1490)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   -Wire-([1458,576],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1458,612],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1458,720],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.peswitch([1530,576],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1548,612],0,0,1550)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1530,720],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1548,756],0,0,1470)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1602,576],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1602,720],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1620,612],0,0,1540)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1620,756],0,0,1460)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.source_1([1710,630],5,0,1530)
    {
    Name = "Source1"
    Type = "6"
    Grnd = "0"
    Spec = "0"
    Cntrl = "1"
    AC = "0"
    Vm = "132.79 [kV]"
    Tc = "0.05 [s]"
    Ph = "0.0 [deg]"
    f = "60.0 [Hz]"
    P = "0.0 [MW]"
    Q = "0.0 [MVAR]"
    R = "1.0 [ohm]"
    Rs = "1.0 [ohm]"
    Rp = "1.0 [ohm]"
    Lp = "0.1 [H]"
    R' = "1.0 [ohm]"
    L = "0.1 [H]"
    C = "1.0 [uF]"
    L' = "0.1 [H]"
    C' = "1.0 [uF]"
    CUR = ""
    }
   -Wire-([1530,576],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1530,612],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1530,720],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1602,720],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1602,612],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1602,576],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1476,558],0,0,-1)
    {
    Vertex="0,0;234,0"
    }
   -Wire-([1458,756],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1476,720],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1476,558],0,0,-1)
    {
    Vertex="0,0;0,18"
    }
   -Wire-([1476,774],0,0,-1)
    {
    Vertex="0,0;234,0"
    }
   -Wire-([1548,720],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1620,720],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1710,594],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([1512,576],0,0,-1)
    {
    Name = "S11"
    }
   0.datalabel([1512,720],0,0,-1)
    {
    Name = "S12"
    }
   0.datalabel([1584,576],0,0,-1)
    {
    Name = "S9"
    }
   0.datalabel([1584,720],0,0,-1)
    {
    Name = "S10"
    }
   0.datalabel([1656,720],0,0,-1)
    {
    Name = "S8"
    }
   0.datalabel([1656,576],0,0,-1)
    {
    Name = "S7"
    }
   0.inductor([1422,666],6,0,-1)
    {
    L = "1 [H]"
    }
   0.inductor([1422,702],6,0,-1)
    {
    L = "1 [H]"
    }
   -Wire-([1152,648],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1458,666],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,720],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1548,576],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1620,576],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1476,774],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1548,774],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1530,756],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1620,774],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1602,756],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.source_1([1710,738],3,0,1480)
    {
    Name = "Source1"
    Type = "6"
    Grnd = "0"
    Spec = "0"
    Cntrl = "1"
    AC = "0"
    Vm = "132.79 [kV]"
    Tc = "0.05 [s]"
    Ph = "0.0 [deg]"
    f = "60.0 [Hz]"
    P = "0.0 [MW]"
    Q = "0.0 [MVAR]"
    R = "1.0 [ohm]"
    Rs = "1.0 [ohm]"
    Rp = "1.0 [ohm]"
    Lp = "0.1 [H]"
    R' = "1.0 [ohm]"
    L = "0.1 [H]"
    C = "1.0 [uF]"
    L' = "0.1 [H]"
    C' = "1.0 [uF]"
    CUR = ""
    }
   -Wire-([1746,810],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   0.var([1710,810],0,61041496,410)
    {
    Name = "Slider 2"
    Group = ""
    Display = "1"
    Max = "1000"
    Min = "0.0"
    Value = "492.063492063"
    Units = "kV"
    Collect = "1"
    }
   0.ground([1746,684],7,0,-1)
    {
    }
   -Wire-([1674,792],0,0,-1)
    {
    Vertex="0,0;0,-162"
    }
   -Wire-([1674,792],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([1710,702],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([1710,684],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([306,936],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([306,972],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([504,774],0,0,-1)
    {
    Vertex="0,0;-18,0"
    }
   -Wire-([504,810],0,0,-1)
    {
    Vertex="0,0;-18,0"
    }
   -Wire-([396,936],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([396,972],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([504,936],0,0,-1)
    {
    Vertex="0,0;-18,0"
    }
   -Wire-([504,972],0,0,-1)
    {
    Vertex="0,0;-18,0"
    }
   -Wire-([306,936],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([396,936],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([486,936],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([306,774],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([306,810],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([396,774],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([396,810],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.source_1([234,972],1,0,430)
    {
    Name = "Source2"
    Type = "6"
    Grnd = "0"
    Spec = "0"
    Cntrl = "1"
    AC = "0"
    Vm = "132.79 [kV]"
    Tc = "0.05 [s]"
    Ph = "0.0 [deg]"
    f = "60.0 [Hz]"
    P = "0.0 [MW]"
    Q = "0.0 [MVAR]"
    R = "1.0 [ohm]"
    Rs = "1.0 [ohm]"
    Rp = "1.0 [ohm]"
    Lp = "0.1 [H]"
    R' = "1.0 [ohm]"
    L = "0.1 [H]"
    C = "1.0 [uF]"
    L' = "0.1 [H]"
    C' = "1.0 [uF]"
    CUR = ""
    }
   -Wire-([234,954],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([234,756],0,0,-1)
    {
    Vertex="0,0;252,0"
    }
   -Wire-([486,774],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([396,774],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([306,774],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.ground([216,882],1,0,-1)
    {
    }
   -Wire-([216,882],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([486,1008],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([396,1008],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([306,1008],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([306,846],0,0,-1)
    {
    Vertex="0,0;198,0"
    }
   -Wire-([396,882],0,0,-1)
    {
    Vertex="0,0;108,0"
    }
   -Wire-([486,918],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.ammeter([576,882],0,0,380)
    {
    Name = "I3B"
    }
   0.ammeter([576,918],0,0,400)
    {
    Name = "I3C"
    }
   0.inductor([504,846],0,0,-1)
    {
    L = "1 [H]"
    }
   0.inductor([504,882],0,0,-1)
    {
    L = "1 [H]"
    }
   0.inductor([504,918],0,0,-1)
    {
    L = "1 [H]"
    }
   -Wire-([540,882],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([540,918],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([774,882],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([810,918],0,0,-1)
    {
    Vertex="0,0;0,-180"
    }
   0.source_1([1710,342],3,0,1600)
    {
    Name = "Source1"
    Type = "6"
    Grnd = "0"
    Spec = "0"
    Cntrl = "1"
    AC = "0"
    Vm = "132.79 [kV]"
    Tc = "0.05 [s]"
    Ph = "0.0 [deg]"
    f = "60.0 [Hz]"
    P = "0.0 [MW]"
    Q = "0.0 [MVAR]"
    R = "1.0 [ohm]"
    Rs = "1.0 [ohm]"
    Rp = "1.0 [ohm]"
    Lp = "0.1 [H]"
    R' = "1.0 [ohm]"
    L = "0.1 [H]"
    C = "1.0 [uF]"
    L' = "0.1 [H]"
    C' = "1.0 [uF]"
    CUR = ""
    }
   -Wire-([1710,306],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   0.ground([1728,270],7,0,-1)
    {
    }
   -Wire-([1746,432],0,0,-1)
    {
    Vertex="0,0;0,-90"
    }
   0.source_1([1710,198],3,0,1640)
    {
    Name = "Source1"
    Type = "6"
    Grnd = "0"
    Spec = "0"
    Cntrl = "1"
    AC = "0"
    Vm = "132.79 [kV]"
    Tc = "0.05 [s]"
    Ph = "0.0 [deg]"
    f = "60.0 [Hz]"
    P = "0.0 [MW]"
    Q = "0.0 [MVAR]"
    R = "1.0 [ohm]"
    Rs = "1.0 [ohm]"
    Rp = "1.0 [ohm]"
    Lp = "0.1 [H]"
    R' = "1.0 [ohm]"
    L = "0.1 [H]"
    C = "1.0 [uF]"
    L' = "0.1 [H]"
    C' = "1.0 [uF]"
    CUR = ""
    }
   -Wire-([1746,342],0,0,-1)
    {
    Vertex="0,0;0,-144"
    }
   -Wire-([342,234],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([342,270],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([396,306],0,0,-1)
    {
    Vertex="0,0;-54,0"
    }
   -Wire-([684,648],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([1134,486],0,0,-1)
    {
    Vertex="0,0;0,-252"
    }
   -Wire-([648,450],0,0,-1)
    {
    Vertex="0,0;0,-180"
    }
   -Wire-([612,720],0,0,-1)
    {
    Vertex="0,0;126,0"
    }
   -Wire-([1458,306],0,0,-1)
    {
    Vertex="0,0;180,0"
    }
   0.ammeter([1386,630],4,0,280)
    {
    Name = "I2C"
    }
   -Wire-([1458,630],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.ammeter([1386,666],4,0,310)
    {
    Name = "I2B"
    }
   0.ammeter([1386,702],4,0,350)
    {
    Name = "I2A"
    }
   -Wire-([1458,702],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   -Wire-([612,882],0,0,-1)
    {
    Vertex="0,0;162,0"
    }
   -Wire-([720,918],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([540,846],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([1386,630],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([1386,666],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([1386,702],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([1710,270],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1710,162],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1620,198],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1620,234],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1620,342],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.peswitch([1620,342],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   -Wire-([1620,378],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1638,342],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1638,396],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1710,396],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.peswitch([1548,378],0,0,1580)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1548,234],0,0,1620)
    {
    L = "I"
    Type = "3"
    SNUB = "0"
    INTR = "0"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1620,198],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   0.peswitch([1530,198],2,0,-1)
    {
    L = "D"
    Type = "0"
    SNUB = "0"
    INTR = "1"
    RON = "0.01 [ohm]"
    ROFF = "1.0E6 [ohm]"
    EFVD = "0.0 [kV]"
    EBO = "1.0E5 [kV]"
    Erw = "1.0E5 [kV]"
    TEXT = "0.0 [us]"
    RD = "5000.0 [ohm]"
    CD = "0.05 [uF]"
    PFB = "0"
    I = ""
    It = ""
    V = ""
    Ton = ""
    Toff = ""
    Alpha = ""
    Gamma = ""
    }
   -Wire-([1530,198],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1530,234],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1530,342],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1530,378],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1548,342],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1548,198],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([1548,396],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1458,198],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1458,234],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1458,342],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1458,378],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1476,342],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1476,396],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1476,198],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([1458,252],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([1422,252],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([1386,234],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.voltmetergnd([630,882],0,0,390)
    {
    Name = "V3B"
    }
   0.voltmetergnd([630,918],0,0,420)
    {
    Name = "V3C"
    }
   0.mult([1206,1494],0,0,920)
    {
    }
   0.mult([1278,1494],0,0,1190)
    {
    }
   0.emtconst([1170,1530],0,0,830)
    {
    Name = ""
    Value = "1"
    }
   0.const([1242,1530],0,0,850)
    {
    Name = ""
    Value = "60"
    }
   0.datalabel([1314,1494],0,0,-1)
    {
    Name = "Qc"
    }
   0.time-sig([1134,1494],0,0,820)
    {
    }
   0.voltmetergnd([1332,702],0,0,340)
    {
    Name = "V2A"
    }
   0.voltmetergnd([1332,666],0,0,300)
    {
    Name = "V2B"
    }
   0.voltmetergnd([1332,630],0,0,270)
    {
    Name = "V2C"
    }
   0.voltmetergnd([1332,234],0,0,100)
    {
    Name = "V1C"
    }
   0.voltmetergnd([1332,270],0,0,130)
    {
    Name = "V1B"
    }
   0.voltmetergnd([1332,306],0,0,160)
    {
    Name = "V1A"
    }
   -Wire-([1152,630],0,0,-1)
    {
    Vertex="0,0;198,0"
    }
   -Wire-([1152,666],0,0,-1)
    {
    Vertex="0,0;198,0"
    }
   -Wire-([1152,702],0,0,-1)
    {
    Vertex="0,0;198,0"
    }
   .INVERTER1CONTROL_1([360,1152],3,0,970)
    {
    }
   0.pgb([1530,1152],0,61055776,610)
    {
    Name = "I3A"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.datalabel([1530,1152],0,0,-1)
    {
    Name = "I3A"
    }
   0.datalabel([1080,990],0,0,-1)
    {
    Name = "V1A"
    }
   0.realpole([1116,990],0,0,470)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   .INVERTER2CONTROL_1([360,1332],3,0,1120)
    {
    }
   .INVERTER3CONTROL_1([360,1512],3,0,1110)
    {
    }
   0.datalabel([198,1206],3,0,-1)
    {
    Name = "Pcommand1"
    }
   0.datalabel([234,1206],3,0,-1)
    {
    Name = "Qcommand1"
    }
   0.datalabel([198,1386],3,0,-1)
    {
    Name = "Pcommand2"
    }
   0.datalabel([234,1386],3,0,-1)
    {
    Name = "Qcommand2"
    }
   0.datalabel([198,1566],3,0,-1)
    {
    Name = "Pcommand3"
    }
   0.datalabel([234,1566],3,0,-1)
    {
    Name = "Qcommand3"
    }
   0.datalabel([270,1206],0,0,-1)
    {
    Name = "VD1"
    }
   0.datalabel([306,1206],0,0,-1)
    {
    Name = "VQ1"
    }
   0.datalabel([270,1386],0,0,-1)
    {
    Name = "VD2"
    }
   0.datalabel([306,1386],0,0,-1)
    {
    Name = "VQ2"
    }
   0.datalabel([270,1566],0,0,-1)
    {
    Name = "VD3"
    }
   0.datalabel([306,1566],0,0,-1)
    {
    Name = "VQ3"
    }
   0.datalabel([342,1206],0,0,-1)
    {
    Name = "I01"
    }
   0.datalabel([414,1386],0,0,-1)
    {
    Name = "I02"
    }
   0.datalabel([414,1566],0,0,-1)
    {
    Name = "I03"
    }
   0.datalabel([486,1206],0,0,-1)
    {
    Name = "ID1"
    }
   0.datalabel([522,1206],0,0,-1)
    {
    Name = "IQ1"
    }
   0.datalabel([342,1386],0,0,-1)
    {
    Name = "ID2"
    }
   0.datalabel([378,1386],0,0,-1)
    {
    Name = "IQ2"
    }
   0.datalabel([342,1566],0,0,-1)
    {
    Name = "ID3"
    }
   0.datalabel([378,1566],0,0,-1)
    {
    Name = "IQ3"
    }
   0.datalabel([378,1206],0,0,-1)
    {
    Name = "I1A"
    }
   0.datalabel([414,1206],0,0,-1)
    {
    Name = "I1B"
    }
   0.datalabel([450,1206],0,0,-1)
    {
    Name = "I1C"
    }
   0.datalabel([450,1386],0,0,-1)
    {
    Name = "I2A"
    }
   0.datalabel([486,1386],0,0,-1)
    {
    Name = "I2B"
    }
   0.datalabel([522,1386],0,0,-1)
    {
    Name = "I2C"
    }
   0.datalabel([450,1566],0,0,-1)
    {
    Name = "I3A"
    }
   0.datalabel([486,1566],0,0,-1)
    {
    Name = "I3B"
    }
   0.datalabel([522,1566],0,0,-1)
    {
    Name = "I3C"
    }
   0.datalabel([270,1098],3,0,-1)
    {
    Name = "S13"
    }
   0.datalabel([306,1098],3,0,-1)
    {
    Name = "S14"
    }
   0.datalabel([342,1098],3,0,-1)
    {
    Name = "S15"
    }
   0.datalabel([378,1098],3,0,-1)
    {
    Name = "S16"
    }
   0.datalabel([414,1098],3,0,-1)
    {
    Name = "S17"
    }
   0.datalabel([450,1098],3,0,-1)
    {
    Name = "S18"
    }
   0.datalabel([270,1278],3,0,-1)
    {
    Name = "S7"
    }
   0.datalabel([306,1278],3,0,-1)
    {
    Name = "S8"
    }
   0.datalabel([342,1278],3,0,-1)
    {
    Name = "S9"
    }
   0.datalabel([378,1278],3,0,-1)
    {
    Name = "S10"
    }
   0.datalabel([414,1278],3,0,-1)
    {
    Name = "S11"
    }
   0.datalabel([450,1278],3,0,-1)
    {
    Name = "S12"
    }
   0.datalabel([270,1458],3,0,-1)
    {
    Name = "S1"
    }
   0.datalabel([306,1458],3,0,-1)
    {
    Name = "S2"
    }
   0.datalabel([342,1458],3,0,-1)
    {
    Name = "S3"
    }
   0.datalabel([378,1458],3,0,-1)
    {
    Name = "S4"
    }
   0.datalabel([414,1458],3,0,-1)
    {
    Name = "S5"
    }
   0.datalabel([450,1458],3,0,-1)
    {
    Name = "S6"
    }
   0.ammeter([576,846],0,0,360)
    {
    Name = "I3A"
    }
   -Wire-([612,846],0,0,-1)
    {
    Vertex="0,0;144,0"
    }
   -Wire-([612,918],0,0,-1)
    {
    Vertex="0,0;108,0"
    }
   -Wire-([1260,234],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1386,270],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([1260,270],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1386,306],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([756,846],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([612,774],0,0,-1)
    {
    Vertex="0,0;144,0"
    }
   -Wire-([774,756],0,0,-1)
    {
    Vertex="0,0;-126,0"
    }
   -Wire-([684,738],0,0,-1)
    {
    Vertex="0,0;126,0"
    }
   0.voltmetergnd([630,846],0,0,370)
    {
    Name = "V3A"
    }
   0.resistor([648,828],5,0,-1)
    {
    R = "10000[ohm]"
    }
   0.resistor([720,828],5,0,-1)
    {
    R = "10000 [ohm]"
    }
   0.resistor([684,828],5,0,-1)
    {
    R = "10000 [ohm]"
    }
   -Wire-([648,792],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   -Wire-([648,846],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([684,882],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([720,918],0,0,-1)
    {
    Vertex="0,0;0,-90"
    }
   0.capacitor([648,972],3,0,-1)
    {
    C = "4.2[uF]"
    }
   0.capacitor([702,972],3,0,-1)
    {
    C = "4.2 [uF]"
    }
   0.capacitor([738,972],3,0,-1)
    {
    C = "4.2 [uF]"
    }
   -Wire-([648,972],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([648,936],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([702,936],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([738,936],0,0,-1)
    {
    Vertex="0,0;0,-90"
    }
   -Wire-([990,306],0,0,-1)
    {
    Vertex="0,0;360,0"
    }
   -Wire-([1062,468],0,0,-1)
    {
    Vertex="0,0;0,-162"
    }
   0.resistor([1206,396],5,0,-1)
    {
    R = "10000.0 [ohm]"
    }
   0.resistor([1224,396],3,0,-1)
    {
    R = "10000.0 [ohm]"
    }
   0.resistor([1260,360],7,0,-1)
    {
    R = "10000.0 [ohm]"
    }
   -Wire-([1206,396],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   -Wire-([1260,360],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   -Wire-([1224,360],0,0,-1)
    {
    Vertex="0,0;0,-90"
    }
   -Wire-([1206,360],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   0.capacitor([1170,180],5,0,-1)
    {
    C = "4.2[uF]"
    }
   0.capacitor([1296,180],5,0,-1)
    {
    C = "4.2[uF]"
    }
   0.capacitor([1242,180],5,0,-1)
    {
    C = "4.2[uF]"
    }
   -Wire-([1170,144],0,0,-1)
    {
    Vertex="0,0;126,0"
    }
   -Wire-([1170,306],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([1242,270],0,0,-1)
    {
    Vertex="0,0;0,-90"
    }
   -Wire-([1296,234],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.resistor([1206,792],5,0,-1)
    {
    R = "10000.0 [ohm]"
    }
   0.resistor([1242,792],5,0,-1)
    {
    R = "10000.0 [ohm]"
    }
   0.resistor([1260,756],7,0,-1)
    {
    R = "10000.0 [ohm]"
    }
   -Wire-([1206,792],0,0,-1)
    {
    Vertex="0,0;54,0"
    }
   0.capacitor([1170,594],3,0,-1)
    {
    C = "4.2[uF]"
    }
   0.capacitor([1278,594],5,0,-1)
    {
    C = "4.2 [uF]"
    }
   0.capacitor([1224,594],5,0,-1)
    {
    C = "4.2[uF]"
    }
   -Wire-([1170,558],0,0,-1)
    {
    Vertex="0,0;108,0"
    }
   -Wire-([1170,702],0,0,-1)
    {
    Vertex="0,0;0,-108"
    }
   -Wire-([1224,666],0,0,-1)
    {
    Vertex="0,0;0,-72"
    }
   -Wire-([1278,630],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([1206,756],0,0,-1)
    {
    Vertex="0,0;0,-126"
    }
   -Wire-([1242,756],0,0,-1)
    {
    Vertex="0,0;0,-90"
    }
   -Wire-([1260,756],0,0,-1)
    {
    Vertex="0,0;0,-54"
    }
   0.datalabel([1530,1188],0,0,-1)
    {
    Name = "I02"
    }
   0.pgb([1530,1188],0,61488144,1350)
    {
    Name = "I02"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1530,1224],0,0,-1)
    {
    Name = "V02"
    }
   0.pgb([1530,1224],0,61488960,1320)
    {
    Name = "V02"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.inductor([306,270],6,0,-1)
    {
    L = "0.1 [H]"
    }
   0.inductor([306,306],6,0,-1)
    {
    L = "0.1 [H]"
    }
   0.voltmetergnd([576,234],0,0,30)
    {
    Name = "VTC"
    }
   0.voltmetergnd([576,270],0,0,60)
    {
    Name = "VTB"
    }
   0.voltmetergnd([576,306],0,0,80)
    {
    Name = "VTA"
    }
   -ControlPanel-([2034,522],0)
    {
    Name = "$(GROUP) : Controls"
    Flags = 0
    State = 1
    Icon = 576,306
    Posn = 2034,522
    Extents = 0,0,288,126
    Slider(61041496)
    Slider(61009672)
    Slider(61009264)
    }
   0.datalabel([1080,1026],0,0,-1)
    {
    Name = "V1B"
    }
   0.realpole([1116,1026],0,0,480)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.datalabel([1080,1062],0,0,-1)
    {
    Name = "V1C"
    }
   0.realpole([1116,1062],0,0,520)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.realpole([1116,1170],0,0,600)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.realpole([1116,1206],0,0,650)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.realpole([1116,1242],0,0,710)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.datalabel([1080,1170],0,0,-1)
    {
    Name = "V2A"
    }
   0.datalabel([1080,1206],0,0,-1)
    {
    Name = "V2B"
    }
   0.datalabel([1080,1242],0,0,-1)
    {
    Name = "V2C"
    }
   0.pgb([1458,1170],0,61495488,630)
    {
    Name = "V2A_filtered"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([1458,1206],0,61495896,700)
    {
    Name = "V2B_filtered"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([1458,1242],0,61496304,730)
    {
    Name = "V2C_filtered"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1080,1350],0,0,-1)
    {
    Name = "V3A"
    }
   0.datalabel([1080,1386],0,0,-1)
    {
    Name = "V3B"
    }
   0.datalabel([1080,1422],0,0,-1)
    {
    Name = "V3C"
    }
   0.realpole([1116,1386],0,0,790)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.realpole([1116,1422],0,0,800)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.realpole([1116,1350],0,0,780)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "400 [us]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.datalabel([486,1098],3,0,-1)
    {
    Name = "P1"
    }
   0.datalabel([522,1098],3,0,-1)
    {
    Name = "Q1"
    }
   0.datalabel([486,1458],3,0,-1)
    {
    Name = "P3"
    }
   0.datalabel([522,1458],3,0,-1)
    {
    Name = "Q3"
    }
   0.datalabel([486,1278],3,0,-1)
    {
    Name = "P2"
    }
   0.datalabel([522,1278],3,0,-1)
    {
    Name = "Q2"
    }
   0.xfmr-3p2w([468,270],6,0,-1)
    {
    Name = "T1"
    Tmva = "10.0 [MVA]"
    f = "60.0 [Hz]"
    YD1 = "0"
    YD2 = "0"
    Lead = "1"
    Xl = "0.1 [pu]"
    Ideal = "0"
    NLL = "0.0 [pu]"
    CuL = "0.0 [pu]"
    Tap = "0"
    View = "0"
    Dtls = "0"
    V1 = "230.0 [kV]"
    V2 = "100 [kV]"
    Enab = "0"
    Sat = "1"
    Xair = "0.2 [pu]"
    Tdc = "1.0 [s]"
    Xknee = "1.25 [pu]"
    Txk = "0.1 [s]"
    Im1 = "1 [%]"
    ILA1 = ""
    ILB1 = ""
    ILC1 = ""
    IAB1 = ""
    IBC1 = ""
    ICA1 = ""
    ILA2 = ""
    ILB2 = ""
    ILC2 = ""
    IAB2 = ""
    IBC2 = ""
    ICA2 = ""
    IMA = ""
    IMB = ""
    IMC = ""
    FLXA = ""
    FLXB = ""
    FLXC = ""
    IMAB = ""
    IMBC = ""
    IMCA = ""
    FLXAB = ""
    FLXBC = ""
    FLXCA = ""
    }
   0.datalabel([1260,918],0,0,-1)
    {
    Name = "I1A"
    }
   0.datalabel([1260,936],0,0,-1)
    {
    Name = "I1B"
    }
   0.datalabel([1260,954],0,0,-1)
    {
    Name = "I1C"
    }
   0.datalabel([1332,918],0,0,-1)
    {
    Name = "ID1"
    }
   0.datalabel([1332,936],0,0,-1)
    {
    Name = "IQ1"
    }
   0.datalabel([1332,954],0,0,-1)
    {
    Name = "I01"
    }
   0.abcdq0([1296,936],0,0,960)
    {
    IDir = "1"
    Theta = "F1"
    }
   0.datalabel([1260,1278],0,0,-1)
    {
    Name = "I3A"
    }
   0.datalabel([1260,1296],0,0,-1)
    {
    Name = "I3B"
    }
   0.datalabel([1260,1314],0,0,-1)
    {
    Name = "I3C"
    }
   0.datalabel([1332,1278],0,0,-1)
    {
    Name = "ID3"
    }
   0.datalabel([1332,1296],0,0,-1)
    {
    Name = "IQ3"
    }
   0.datalabel([1332,1314],0,0,-1)
    {
    Name = "I03"
    }
   0.abcdq0([1296,1296],0,0,1090)
    {
    IDir = "1"
    Theta = "Phi"
    }
   0.compare([1548,936],0,0,450)
    {
    X = "10"
    OL = "1"
    OH = "0"
    INTR = "0"
    }
   0.datalabel([1620,936],3,0,-1)
    {
    Name = "DIST"
    }
   0.time-sig([1476,936],0,0,440)
    {
    }
   0.unity([1620,936],0,0,460)
    {
    IType = "2"
    OType = "1"
    }
   0.datalabel([1620,1008],7,0,-1)
    {
    Name = "S2M"
    }
   0.compare([1548,1008],0,0,500)
    {
    X = "0.15"
    OL = "0"
    OH = "1"
    INTR = "0"
    }
   0.time-sig([1476,1008],0,0,490)
    {
    }
   0.unity([1620,1008],0,0,510)
    {
    IType = "2"
    OType = "1"
    }
   0.datalabel([1620,1080],3,0,-1)
    {
    Name = "ENAB"
    }
   0.compare([1548,1080],0,0,540)
    {
    X = "0.7"
    OL = "0"
    OH = "1"
    INTR = "0"
    }
   0.time-sig([1476,1080],0,0,530)
    {
    }
   0.unity([1620,1080],0,0,550)
    {
    IType = "2"
    OType = "1"
    }
   -Wire-([396,198],0,0,-1)
    {
    Vertex="0,0;108,0"
    }
   0.ground([396,198],1,0,-1)
    {
    }
   0.abcdq0([1116,1296],0,0,1000)
    {
    IDir = "1"
    Theta = "F3"
    }
   0.datalabel([1080,1278],0,0,-1)
    {
    Name = "V3A"
    }
   0.datalabel([1080,1296],0,0,-1)
    {
    Name = "V3B"
    }
   0.datalabel([1080,1314],0,0,-1)
    {
    Name = "V3C"
    }
   0.datalabel([1152,1278],0,0,-1)
    {
    Name = "VD3"
    }
   0.datalabel([1152,1296],0,0,-1)
    {
    Name = "VQ3"
    }
   0.datalabel([1152,1314],0,0,-1)
    {
    Name = "V03"
    }
   0.abcdq0([1116,1116],0,0,980)
    {
    IDir = "1"
    Theta = "F2"
    }
   0.datalabel([1080,1098],0,0,-1)
    {
    Name = "V2A"
    }
   0.datalabel([1080,1116],0,0,-1)
    {
    Name = "V2B"
    }
   0.datalabel([1080,1134],0,0,-1)
    {
    Name = "V2C"
    }
   0.datalabel([1152,1098],0,0,-1)
    {
    Name = "VD2"
    }
   0.datalabel([1152,1116],0,0,-1)
    {
    Name = "VQ2"
    }
   0.datalabel([1152,1134],0,0,-1)
    {
    Name = "V02"
    }
   0.abcdq0([1296,1116],0,0,990)
    {
    IDir = "1"
    Theta = "F2"
    }
   0.datalabel([1260,1098],0,0,-1)
    {
    Name = "I2A"
    }
   0.datalabel([1260,1116],0,0,-1)
    {
    Name = "I2B"
    }
   0.datalabel([1260,1134],0,0,-1)
    {
    Name = "I2C"
    }
   0.datalabel([1332,1098],0,0,-1)
    {
    Name = "ID2"
    }
   0.datalabel([1332,1116],0,0,-1)
    {
    Name = "IQ2"
    }
   0.datalabel([1332,1134],0,0,-1)
    {
    Name = "I02"
    }
   0.tvekta([1278,1026],0,0,880)
    {
    Gp = "5"
    Gi = "90"
    Vbas = "230.0 [kV]"
    Fbas = "60.0 [Hz]"
    Mode = "0"
    PMode = "0"
    THOFF = "0.0"
    TREL = "0.15"
    dlead = "1"
    FName = "fPLL1"
    Err = ""
    }
   0.datalabel([1350,1026],0,0,-1)
    {
    Name = "F1"
    }
   0.pgb([1350,1026],0,61522416,1400)
    {
    Name = "F1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.abcdq0([1116,936],0,0,950)
    {
    IDir = "1"
    Theta = "F1"
    }
   0.datalabel([1080,918],0,0,-1)
    {
    Name = "V1A"
    }
   0.datalabel([1080,936],0,0,-1)
    {
    Name = "V1B"
    }
   0.datalabel([1080,954],0,0,-1)
    {
    Name = "V1C"
    }
   0.datalabel([1152,918],0,0,-1)
    {
    Name = "VD1"
    }
   0.datalabel([1152,936],0,0,-1)
    {
    Name = "VQ1"
    }
   0.datalabel([1152,954],0,0,-1)
    {
    Name = "V01"
    }
   0.tvekta([1278,1206],0,0,890)
    {
    Gp = "5"
    Gi = "90"
    Vbas = "230.0 [kV]"
    Fbas = "60.0 [Hz]"
    Mode = "0"
    PMode = "0"
    THOFF = "0.0"
    TREL = "0.25"
    dlead = "1"
    FName = "fPLL2"
    Err = ""
    }
   0.datalabel([1458,1170],0,0,-1)
    {
    Name = "V2A"
    }
   0.datalabel([1458,1206],0,0,-1)
    {
    Name = "V2B"
    }
   0.datalabel([1458,1242],0,0,-1)
    {
    Name = "V2C"
    }
   0.datalabel([1350,1206],0,0,-1)
    {
    Name = "F2"
    }
   0.pgb([1350,1206],0,61527720,1340)
    {
    Name = "F2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.tvekta([1278,1386],0,0,900)
    {
    Gp = "5"
    Gi = "90"
    Vbas = "230.0 [kV]"
    Fbas = "60.0 [Hz]"
    Mode = "0"
    PMode = "0"
    THOFF = "0.0"
    TREL = "0.35"
    dlead = "1"
    FName = "fPLL3"
    Err = ""
    }
   0.datalabel([1350,1386],0,0,-1)
    {
    Name = "F3"
    }
   0.pgb([1350,1386],0,61528944,1230)
    {
    Name = "F3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([738,1602],0,0,-1)
    {
    Name = "fPLL3"
    }
   0.pgb([738,1602],0,61529760,1170)
    {
    Name = "fPLL3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([738,1548],0,0,-1)
    {
    Name = "fPLL2"
    }
   0.pgb([738,1548],0,61530576,1180)
    {
    Name = "fPLL2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.mult([864,1530],0,0,930)
    {
    }
   0.mult([864,1602],0,0,940)
    {
    }
   0.mult([864,1458],0,0,910)
    {
    }
   0.datalabel([828,1458],0,0,-1)
    {
    Name = "fPLL1"
    }
   0.datalabel([828,1530],0,0,-1)
    {
    Name = "fPLL2"
    }
   0.datalabel([828,1602],0,0,-1)
    {
    Name = "fPLL3"
    }
   0.const([828,1494],0,0,810)
    {
    Name = ""
    Value = "6.283185307179586"
    }
   0.datalabel([900,1458],0,0,-1)
    {
    Name = "W1"
    }
   0.datalabel([900,1530],0,0,-1)
    {
    Name = "W2"
    }
   0.datalabel([900,1602],0,0,-1)
    {
    Name = "W3"
    }
   0.const([828,1566],0,0,840)
    {
    Name = ""
    Value = "6.283185307179586"
    }
   0.const([828,1638],0,0,860)
    {
    Name = ""
    Value = "6.283185307179586"
    }
   0.pgb([900,1458],0,61577096,1220)
    {
    Name = "W1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([900,1530],0,61577504,1200)
    {
    Name = "W2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([900,1602],0,61577912,1160)
    {
    Name = "W3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([738,1494],0,0,-1)
    {
    Name = "fPLL1"
    }
   0.pgb([738,1494],0,61578728,1210)
    {
    Name = "fPLL1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([198,216],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([198,252],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([198,288],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   0.sync_machine([162,252],6,0,1060)
    {
    Name = "G"
    Nqaxw = "1"
    Cnfg = "0"
    MM = "1"
    CfRa = "1"
    MSat = "0"
    icTyp = "0"
    Iscl = "1"
    View = "0"
    itrfa = "0"
    izro = "0"
    Icsp = "0"
    Icmp = "0"
    Immd = "0"
    Ifmlt = "0"
    Term = "3"
    Ts = "0.02 [s]"
    Iexc = "1"
    Igov = "1"
    Ospd = "0"
    machsw = "S2M"
    Enab = "ENAB"
    npadjs = "0"
    pset = "0.0 [MW]"
    nftrsw = "0"
    hmult = "1.0"
    sdmftr = "1.0"
    sdmspd = "1.0"
    npadjm = "0"
    fldmlt = "1.0"
    nfldsw = "0"
    Vbase = "132.79[kV]"
    Ibase = "0.025 [kA]"
    OMO = "376.991118 [rad/s]"
    H = "31.17 [s]"
    D = "0.04 [pu]"
    RNeut = "1.0E5 [pu]"
    XNeut = "0 [pu]"
    Ri = "300.0 [pu]"
    NOM = "1.0"
    Rs1 = "0.0025 [pu]"
    XS1 = "0.14 [pu]"
    XMD0 = "1.66 [pu]"
    R2D = "0.00043 [pu]"
    X2D = "0.2004 [pu]"
    R3D = "0.0051 [pu]"
    X3D = "0.0437 [pu]"
    X230 = "0.0 [pu]"
    XMQ = "0.91 [pu]"
    R2Q = "0.00842 [pu]"
    X2Q = "0.106 [pu]"
    R3Q = "8.1942E-03 [pu]"
    X3Q = "9.4199E-02 [pu]"
    X231 = "0.0 [pu]"
    Ra = "0.0051716 [pu]"
    Ta = "0.278 [s]"
    Xp = "0.163 [pu]"
    Xd = "1.014 [pu]"
    Xd' = "0.314 [pu]"
    Tdo' = "6.55 [s]"
    Xd'' = "0.280 [pu]"
    Tdo'' = "0.039 [s]"
    Gfld = "1.0E+2 [pu]"
    Xkf = "0.0 [pu]"
    Xq = "0.770 [pu]"
    Xq' = "0.228 [pu]"
    Tqo' = "0.85 [s]"
    Xq'' = "0.375 [pu]"
    Tqo'' = "0.071 [s]"
    AGFC = "1.0"
    X1 = "0.0"
    Y1 = "0.0 [pu]"
    X2 = "0.5"
    Y2 = "0.5 [pu]"
    X3 = "0.8"
    Y3 = "0.79 [pu]"
    X4 = "1.0"
    Y4 = "0.947 [pu]"
    X5 = "1.2"
    Y5 = "1.076 [pu]"
    X6 = "1.5"
    Y6 = "1.2 [pu]"
    X7 = "1.8"
    Y7 = "1.26 [pu]"
    X8 = "2.2"
    Y8 = "1.32 [pu]"
    X9 = "3.2"
    Y9 = "1.53 [pu]"
    X10 = "4.2"
    Y10 = "1.74 [pu]"
    VT = "1.0[pu]"
    Pheta = "0 [rad]"
    Trmpv = "0 [s]"
    Sysfl = "100.0 [pu]"
    Ptcon = "0.2 [s]"
    P0 = "60.0 [MW]"
    Q0 = "0.0 [MVAR]"
    Theti = "3.141592 [rad]"
    Idi = "0.0 [pu]"
    Iqi = "0.0 [pu]"
    Ifi = "0.0 [pu]"
    Spdi = "1.0 [pu]"
    POut = "POUT"
    QOut = "QOUT"
    Vneut = ""
    Cneut = ""
    Lang = "Lang"
    Theta = "Theta"
    Wang = "Wang"
    Tesmt = ""
    PQscl = "0"
    InExc = "InitEx"
    InGov = "InitGv"
    Mon1 = "1"
    Chn1 = ""
    Mon2 = "1"
    Chn2 = ""
    Mon3 = "1"
    Chn3 = ""
    Mon4 = "1"
    Chn4 = ""
    Mon5 = "1"
    Chn5 = ""
    Mon6 = "1"
    Chn6 = ""
    }
   0.sandhdefn([180,108],4,0,1020)
    {
    Iand = "1"
    }
   -Wire-([162,162],0,0,-1)
    {
    Vertex="0,0;-108,0"
    }
   -Wire-([162,180],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.datalabel([180,144],0,0,-1)
    {
    Name = "ENAB"
    }
   0.datalabel([126,324],1,0,-1)
    {
    Name = "EF"
    }
   0.datalabel([162,324],3,0,-1)
    {
    Name = "IF"
    }
   -Wire-([198,324],0,0,-1)
    {
    Vertex="0,0;0,36"
    }
   -Wire-([90,324],0,0,-1)
    {
    Vertex="0,0;0,72"
    }
   0.sandhdefn([126,396],6,0,1010)
    {
    Iand = "1"
    }
   -Wire-([126,342],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   0.datalabel([126,360],0,0,-1)
    {
    Name = "S2M"
    }
   -Wire-([162,396],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([216,234],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([216,270],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([216,306],0,0,-1)
    {
    Vertex="0,0;0,-18"
    }
   -Wire-([216,234],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([216,270],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([216,306],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.datalabel([126,180],0,0,-1)
    {
    Name = "S"
    }
   0.datalabel([1530,1260],0,0,-1)
    {
    Name = "S"
    }
   0.pgb([1530,1260],0,61582808,1300)
    {
    Name = "S"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.datalabel([1620,1152],0,0,-1)
    {
    Name = "VD1"
    }
   0.datalabel([1620,1188],0,0,-1)
    {
    Name = "VQ1"
    }
   0.datalabel([1620,1224],0,0,-1)
    {
    Name = "VD2"
    }
   0.datalabel([1620,1260],0,0,-1)
    {
    Name = "VQ2"
    }
   0.datalabel([1620,1296],0,0,-1)
    {
    Name = "VD3"
    }
   0.datalabel([1620,1332],0,0,-1)
    {
    Name = "VQ3"
    }
   0.pgb([1620,1152],0,61585664,1360)
    {
    Name = "VD1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.pgb([1620,1188],0,61586072,1330)
    {
    Name = "VQ1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.pgb([1620,1224],0,61586480,1310)
    {
    Name = "VD2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.pgb([1620,1260],0,61586888,1290)
    {
    Name = "VQ2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.pgb([1620,1296],0,61587296,1270)
    {
    Name = "VD3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.pgb([1620,1332],0,61587704,1240)
    {
    Name = "VQ3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.datalabel([1530,1332],4,0,-1)
    {
    Name = "Tm"
    }
   0.pgb([1530,1332],0,61588520,1250)
    {
    Name = "Tm"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.datalabel([1692,1098],3,0,-1)
    {
    Name = "ENAB"
    }
   0.pgb([1692,1098],0,61589336,590)
    {
    Name = "ENAB"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.datalabel([1530,1296],4,0,-1)
    {
    Name = "Ef"
    }
   0.pgb([1530,1296],0,61590152,1280)
    {
    Name = "Vref"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-1"
    Max = "1"
    }
   0.datalabel([522,126],0,0,-1)
    {
    Name = "S2M"
    }
   -Wire-([1152,1170],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,1206],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,1242],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([216,108],0,0,-1)
    {
    Vertex="0,0;0,54"
    }
   0.nodeloop([360,270],0,0,50)
    {
    View = "0"
    }
   0.nodeloop([252,270],0,0,40)
    {
    View = "0"
    }
   -Wire-([252,342],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([324,342],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   0.power([324,378],1,0,150)
    {
    DIR = "1"
    P = "1"
    Q = "1"
    TS = "0.02 [s]"
    View = "0"
    }
   0.datalabel([324,414],0,0,-1)
    {
    Name = "PGEN"
    }
   0.datalabel([1458,1278],0,0,-1)
    {
    Name = "PGEN"
    }
   0.pgb([1458,1278],0,61593008,740)
    {
    Name = "PGEN"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Wire-([198,162],0,0,-1)
    {
    Vertex="0,0;0,18"
    }
   -Wire-([198,162],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([126,342],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([162,378],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Plot-([2052,918],0)
    {
    Title = "W3"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [720,792]
    Icon = [2052,918]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61577912,"W3",0,,,)
     }
    }
   -Plot-([2052,900],0)
    {
    Title = "W2"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [738,846]
    Icon = [2052,900]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61577504,"W2",0,,,)
     Curve(61582808,"S",0,,,)
     }
    }
   0.datalabel([1134,1602],0,0,-1)
    {
    Name = "S"
    }
   0.integral([1170,1602],0,0,1070)
    {
    Extrn = "0"
    Reset = "1"
    Mthd = "0"
    noname5 = ""
    INTR = "0"
    INTCLR = "0"
    T = "1 [s]"
    Yo = "0.0"
    YRst = "0.0"
    YHi = "100.0"
    YLo = "-100.0"
    }
   0.compare([1278,1602],0,0,1080)
    {
    X = "6.283185307179586"
    OL = "0"
    OH = "1"
    INTR = "0"
    }
   0.datalabel([1224,1602],0,0,-1)
    {
    Name = "Phi"
    }
   -Wire-([1206,1602],0,0,-1)
    {
    Vertex="0,0;36,0"
    }
   -Wire-([1170,1638],0,0,-1)
    {
    Vertex="0,0;144,0"
    }
   -Wire-([1314,1638],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   0.datalabel([1458,1332],0,0,-1)
    {
    Name = "Phi"
    }
   0.pgb([1458,1332],0,61595456,1260)
    {
    Name = "Phi"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Plot-([2052,972],0)
    {
    Title = "VD2 and VQ2"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1710,1098]
    Icon = [2052,972]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61586480,"VD2",0,,,)
     Curve(61586888,"VQ2",0,,,)
     }
    }
   -Plot-([2052,954],0)
    {
    Title = "VD1 and VQ1"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1692,1134]
    Icon = [2052,954]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 129
     Units = ""
     Curve(61585664,"VD1",0,,,)
     Curve(61586072,"VQ1",0,,,)
     }
    }
   -Plot-([2052,846],0)
    {
    Title = "fPLL2"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1944,1332]
    Icon = [2052,846]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(61530576,"fPLL2",0,,,)
     }
    }
   -Plot-([2052,864],0)
    {
    Title = "fPLL3"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1944,1422]
    Icon = [2052,864]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61529760,"fPLL3",0,,,)
     }
    }
   -Plot-([2052,792],0)
    {
    Title = "$(GROUP) : Machine Vref"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1278,1206]
    Icon = [2052,792]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61590152,"Vref",0,,,)
     }
    }
   -Plot-([2052,882],0)
    {
    Title = "W1"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [720,792]
    Icon = [2052,882]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(61577096,"W1",0,,,)
     Curve(61582808,"S",0,,,)
     }
    }
   -Wire-([1152,990],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,1026],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,1062],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,1350],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,1386],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   -Wire-([1152,1422],0,0,-1)
    {
    Vertex="0,0;90,0"
    }
   0.const([378,72],0,0,20)
    {
    Name = ""
    Value = "6.283185307179586"
    }
   0.consti([342,36],0,0,10)
    {
    Name = ""
    Value = "60"
    }
   0.mult([414,36],0,0,870)
    {
    }
   0.datalabel([450,36],0,0,-1)
    {
    Name = "S0"
    }
   0.datalabel([18,36],0,0,-1)
    {
    Name = "S"
    }
   0.sumjct([54,36],0,0,1030)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   0.datalabel([54,72],0,0,-1)
    {
    Name = "S0"
    }
   -Wire-([162,72],0,0,-1)
    {
    Vertex="0,0;0,-36"
    }
   -Wire-([90,72],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.sumjct([90,108],2,0,1050)
    {
    DPath = "1"
    A = "0"
    B = "0"
    C = "0"
    D = "1"
    E = "0"
    F = "-1"
    G = "0"
    }
   -Wire-([126,108],0,0,-1)
    {
    Vertex="0,0;18,0"
    }
   -Wire-([54,108],0,0,-1)
    {
    Vertex="0,0;0,54"
    }
   0.datalabel([54,162],0,0,-1)
    {
    Name = "Tm"
    }
   -Plot-([2052,990],0)
    {
    Title = "VD3 and VQ3"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1746,1098]
    Icon = [2052,990]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61587296,"VD3",0,,,)
     Curve(61587704,"VQ3",0,,,)
     }
    }
   0.realpole([126,36],0,0,1040)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1"
    T = "0.1 [s]"
    Max = "10.0"
    Min = "-10.0"
    }
   -Plot-([2052,810],0)
    {
    Title = "F1"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1944,1206]
    Icon = [2052,810]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61522416,"F1",0,,,)
     Curve(61595456,"Phi",0,,,)
     }
    }
   -Plot-([2052,774],0)
    {
    Title = "Enable"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1278,954]
    Icon = [2052,774]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61589336,"ENAB",0,,,)
     }
    }
   -Plot-([2052,828],0)
    {
    Title = "fPLL1"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1944,1260]
    Icon = [2052,828]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 0
     Units = ""
     Curve(61578728,"fPLL1",0,,,)
     }
    }
   -Plot-([2052,936],0)
    {
    Title = "Machine Torque"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1908,954]
    Icon = [2052,936]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 1
     Units = ""
     Curve(61588520,"Tm",0,,,)
     }
    }
   -Plot-([1890,90],0)
    {
    Title = "S"
    Draw = 1
    Area = [0,0,576,288]
    Posn = [1890,90]
    Icon = [2052,738]
    Extents = 0,0,576,288
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,576,225],"y")
     {
     Options = 1
     Units = ""
     Curve(61582808,"S",0,,,)
     }
    }
   .pscad_send([738,1080],0,0,1410)
    {
    ndata = "3"
    IP1 = "192"
    IP2 = "168"
    IP3 = "56"
    IP4 = "101"
    port = "4000"
    dt = "0.01"
    delay = "0.8"
    }
   .pscad_recv([738,1242],0,0,640)
    {
    ndata = "3"
    IP1 = "192"
    IP2 = "168"
    IP3 = "56"
    IP4 = "101"
    port = "4000"
    dt = "0.01"
    delay = "0.81"
    }
   -Wire-([792,1224],0,0,-1)
    {
    Vertex="0,0;72,0"
    }
   0.datamerge([684,1080],3,0,1150)
    {
    N = "3"
    Type = "2"
    Disp = "0"
    }
   0.datatap([846,1242],0,0,660)
    {
    Index = "1"
    Dim = "1"
    Type = "2"
    Style = "0"
    Disp = "1"
    }
   0.datatap([864,1242],0,0,670)
    {
    Index = "2"
    Dim = "1"
    Type = "2"
    Style = "0"
    Disp = "1"
    }
   0.datatap([882,1242],0,0,680)
    {
    Index = "3"
    Dim = "1"
    Type = "2"
    Style = "0"
    Disp = "1"
    }
   0.datalabel([846,1242],3,0,-1)
    {
    Name = "Pcommand1"
    }
   0.datalabel([864,1242],3,0,-1)
    {
    Name = "Pcommand2"
    }
   0.datalabel([882,1242],3,0,-1)
    {
    Name = "Pcommand3"
    }
   0.datalabel([612,1062],3,0,-1)
    {
    Name = "P1Stable"
    }
   0.datalabel([630,1062],3,0,-1)
    {
    Name = "P2Stable"
    }
   0.datalabel([648,1062],3,0,-1)
    {
    Name = "P3Stable"
    }
   0.const([738,1350],1,0,750)
    {
    Name = ""
    Value = "0"
    }
   0.datalabel([738,1386],0,0,-1)
    {
    Name = "Qcommand1"
    }
   0.const([846,1350],1,0,760)
    {
    Name = ""
    Value = "0"
    }
   0.datalabel([846,1386],0,0,-1)
    {
    Name = "Qcommand2"
    }
   0.const([954,1350],1,0,770)
    {
    Name = ""
    Value = "0"
    }
   0.datalabel([954,1386],0,0,-1)
    {
    Name = "Qcommand3"
    }
   0.datalabel([954,1206],0,0,-1)
    {
    Name = "Pcommand1"
    }
   0.datalabel([954,1242],0,0,-1)
    {
    Name = "Pcommand2"
    }
   0.datalabel([954,1278],0,0,-1)
    {
    Name = "Pcommand3"
    }
   0.pgb([954,1278],0,61608512,720)
    {
    Name = "P3Command"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([954,1242],0,61608920,690)
    {
    Name = "P2Command"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([954,1206],0,61609328,1370)
    {
    Name = "P1Command"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.realpole([882,1026],0,0,1100)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "20 [ms]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.realpole([882,1080],0,0,1140)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "20 [ms]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.realpole([882,1134],0,0,1130)
    {
    Limit = "0"
    COM = "Real_Pole"
    Reset = "0"
    YO = "0.0"
    G = "1.0"
    T = "20 [ms]"
    Max = "10.0"
    Min = "-10.0"
    }
   0.datalabel([918,1026],3,0,-1)
    {
    Name = "P1Stable"
    }
   0.datalabel([918,1080],3,0,-1)
    {
    Name = "P2Stable"
    }
   0.datalabel([918,1134],3,0,-1)
    {
    Name = "P3Stable"
    }
   0.datalabel([846,1026],0,0,-1)
    {
    Name = "P1"
    }
   0.datalabel([846,1080],0,0,-1)
    {
    Name = "P2"
    }
   0.datalabel([846,1134],0,0,-1)
    {
    Name = "P3"
    }
   0.datalabel([972,1026],0,0,-1)
    {
    Name = "P1Stable"
    }
   0.datalabel([972,1080],0,0,-1)
    {
    Name = "P2Stable"
    }
   0.datalabel([972,1134],0,0,-1)
    {
    Name = "P3Stable"
    }
   0.pgb([972,1026],0,61614632,1420)
    {
    Name = "P1"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([972,1080],0,61615040,1390)
    {
    Name = "P2"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   0.pgb([972,1134],0,61615448,1380)
    {
    Name = "P3"
    Group = ""
    Display = "0"
    Scale = "1.0"
    Units = ""
    mrun = "0"
    Pol = "0"
    Min = "-2.0"
    Max = "2.0"
    }
   -Plot-([1764,1350],0)
    {
    Title = "$(GROUP) : PCommand"
    Draw = 1
    Area = [0,0,576,288]
    Posn = [1764,1350]
    Icon = [2052,1026]
    Extents = 0,0,576,288
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,576,225],"y")
     {
     Options = 0
     Units = ""
     Curve(61609328,"P1Command",0,,,)
     Curve(61608920,"P2Command",0,,,)
     Curve(61608512,"P3Command",0,,,)
     }
    }
   0.datamerge([684,1242],3,0,620)
    {
    N = "3"
    Type = "2"
    Disp = "0"
    }
   0.const([612,1188],1,0,560)
    {
    Name = ""
    Value = "-20"
    }
   0.const([630,1188],1,0,570)
    {
    Name = ""
    Value = "20"
    }
   0.const([648,1188],1,0,580)
    {
    Name = ""
    Value = "0"
    }
   -Plot-([1764,1062],0)
    {
    Title = "$(GROUP) : POutput"
    Draw = 1
    Area = [0,0,576,288]
    Posn = [1764,1062]
    Icon = [2052,1044]
    Extents = 0,0,576,288
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,576,225],"y")
     {
     Options = 0
     Units = ""
     Curve(61614632,"P1",0,,,)
     Curve(61615040,"P2",0,,,)
     Curve(61615448,"P3",0,,,)
     }
    }
   -Plot-([2052,1008],0)
    {
    Title = "Phi"
    Draw = 0
    Area = [0,0,576,288]
    Posn = [1206,1188]
    Icon = [2052,1008]
    Extents = 0,0,288,18
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,288,90],"y")
     {
     Options = 128
     Units = ""
     Curve(61595456,"Phi",0,,,)
     }
    }
   -Plot-([1890,378],0)
    {
    Title = "$(GROUP) : Generator Power"
    Draw = 1
    Area = [0,0,576,288]
    Posn = [1890,378]
    Icon = [2052,756]
    Extents = 0,0,576,288
    XLabel = " "
    AutoPan = "false,75"
    Graph([0,0],[0,0,576,225],"y")
     {
     Options = 128
     Units = ""
     Curve(61593008,"PGEN",0,,,)
     }
    }
   }
  }
 }

