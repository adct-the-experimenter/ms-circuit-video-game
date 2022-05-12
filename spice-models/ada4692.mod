* ADA4692 SPICE Macro-model
* Description: Amplifier
* Generic Desc: 2.7/5V, CMOS, OP, Low Noise, SD, 4X
* Developed by: HH / ADSJ
* Revision History: 08/10/2012 - Updated to new header style
* 1.0 (07/2009)
* Copyright 2009, 2012 by Analog Devices
*
* Refer to http://www.analog.com/Analog_Root/static/techSupport/designTools/spiceModels/license/spice_general.html for License Statement.  Use of this model
* indicates your acceptance of the terms and provisions in the License Statement.
*
* BEGIN Notes:
* Not Modeled:
*    
* Parameters modeled include: 
*   VSY=5V, T=25°C
*
* END Notes
*
* Node Assignments
*                       noninverting input
*                       |   inverting input
*                       |   |    positive supply
*                       |   |    |   negative supply
*                       |   |    |   |   output
*                       |   |    |   |   |
*                       |   |    |   |   |
.SUBCKT ADA4692         1   2   99  50  45
*
* INPUT STAGE
*
M1   4  7  8 99 PIX L=1E-6 W=2.807E-04
M2   6  2  8 99 PIX L=1E-6 W=2.807E-04
RD1  4 50 1.333E+04
RD2  6 50 1.333E+04
C1   4  6 5.900E-13
I1  99  8 3.000E-05
V1  99  9 1.098E-00
D1   8  9 DX
EOS  7  1 POLY(4) (73,98) (22,98) (81,98) (83,98) 5.00E-04 1 1 1 1
IOS  1  2 5.00E-13
*

* INTERNAL VOLTAGE REFERENCE
*
EREF 98  0 POLY(2) (99,0) (50,0) 0 0.5 0.5
EVP  97 98 (99,50) 0.5
EVN  51 98 (50,99) 0.5
*
* GAIN STAGE
*
G1 98 30 POLY(1) (4,6) 0 4.599E-04 
R1 30 98 1.000E+06
RZ 45 31 3.429E+02
CF 30 31 1.115E-10
V3 32 30 0.2225E+00
V4 30 33 0.4087E+00
D3 32 97 DX
D4 51 33 DX
*
*CMRR 
*
E1  72 98 POLY(2) (1,98) (2,98) 0 2.667E-05 2.667E-05
R10 72 73 1.592E+00
R20 73 98 5.305E-03
C10 72 73 1.000E-06
*
* PSRR
*
EPSY 21 98 POLY(1) (99,50) -1.913E-00 3.826E-01 
RPS1 21 22 6.631E+02
RPS2 22 98 2.449E-02
CPS1 21 22 1.000E-06
*
* VOLTAGE NOISE REFERENCE OF 13nV/rt(Hz)
*
VN1 80 98 0
RN1 80 98 22.5E-03
HN  81 98 VN1 1.33E+01
RN2 81 98 1
*
* FLICKER NOISE CORNER = 1000 Hz
*
DFN 82 98 DNOISE
VFN 82 98 DC 0.6551
HFN 83 98 POLY(1) VFN 1.00E-03 1.00E+00
RFN 83 98 1
*
* OUTPUT STAGE
*
M5  45 46 99 99 POX L=1.00E-6 W=1.231E-03
M6  45 47 50 50 NOX L=1.00E-6 W=3.792E-04
EG1 99 46 POLY(1) (98,30) 7.964E-01 1
EG2 47 50 POLY(1) (30,98) 6.869E-01 1
*
GSY  99 50 POLY(1) (99,50) 63.95E-06 5.937E-06
*
* MODELS
*
.MODEL POX PMOS (LEVEL=2,KP=1.00E-05,VTO=-0.7,LAMBDA=0.05, RB=1E+00)
.MODEL NOX NMOS (LEVEL=2,KP=4.00E-05,VTO=+0.6,LAMBDA=0.035, RB=1E+00)
.MODEL PIX PMOS (LEVEL=2,KP=4.00E-05,VTO=-0.5,LAMBDA=0.03,RB=1E-02)
*.MODEL NIX NMOS (LEVEL=2,KP=4.00E-05,VTO=0.5, LAMBDA=0.03,RB=1E+00)
.MODEL DX D(IS=1E-14,RS=0.1)
.MODEL DNOISE D(IS=1E-14,RS=0,KF=3.6E-11)
*
.ENDS ADA4692
*
*$




