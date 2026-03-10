#
# Copyright (C) 2020 GreenWaves Technologies, SAS, ETH Zurich and
#                    University of Bologna
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from cpu.iss.isa_gen.isa_gen import *
from cpu.iss.isa_gen.isa_riscv_gen import *

#
# RV32M
#

# Encodings for matrix instruction set

                # 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
#RVMBASE           0  0  1  -  -  -  0  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
#RVMMACC           0  0  1  0  1  0  0 | md |    ms2    |    ms1    | 0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
#RVZEROA           0  0  1  0  0  0  0 | md | 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1 
#RVZEROM           0  0  1  0  0  1  0  0  0  0  0  0  0|    ms1    | 0  0  0  0  0  0  0  0  0  0  0  1  0  1  1 

                # 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
#RVMMOVEBASE       0  1  0  -  -  -  0  -  -  -  -  0  0  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  1  0  1  1
#RVMMOVEAM         0  1  0  0  0  0  0 | ms1| 0  0  0  0  0  0  0  0|     md    | 0  0  0  0  0  0  0  1  0  1  1
#RVMMOVEAA         0  1  0  0  0  1  0 | ms1|  md | 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
#RVMMOVEMA         0  1  0  0  1  0  0  0  0|  md | 0  0 |   ms1    | 0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
#RVMMOVEMM         0  1  0  0  1  1  0  0  0  0  0  0  0 |   ms1    |     md    | 0  0  0  0  0  0  0  1  0  1  1

                # 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
#RVMCFGBASE        0  1  1  -  -  -  0  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  1  0  1  1
#RVMCFGK           0  1  1  0  0  0  0 |      rd     |      rs1     | 0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
#RVMCFGM           0  1  1  0  0  1  0 |      rd     |      rs1     |imm2 | 0  0  0  0  0  0  0  0  0  1  0  1  1
#RVMCFGN           0  1  1  0  1  0  0 |      rd     |      rs1     |imm2 | 0  0  0  0  0  0  0  0  0  1  0  1  1
#RVMCFGDT          0  1  1  0  1  1  0 |     imm5    |     imm5     |      imm5    | 0  0  0  0  0  0  1  0  1  1

                # 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
#RVMLOADSTORE      0  0  0|  ldst  | 0 |     rs2     |     rs1      |    mds3   | 0  0  0  0  0  0  0  1  0  1  1



                # 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
# mld.lhs          0  0  0  0  0  0  0  -  -  -  -  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  1  0  1  1
# mld.rhs          0  0  0  0  0  1  0  -  -  -  -  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  1  0  1  1
# mst              0  0  0  0  1  0  0  -  -  -  -  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  1  0  1  1

# mmac             0  0  1  0  1  0  0  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1

# mzerom           0  0  1  0  0  1  0  0  0  0  0  0  0  -  -  -  -  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
# mzeroa           0  0  1  0  0  0  0  -  -  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
  
# mmov.am          0  1  0  0  0  0  0  -  -  0  0  0  0  0  0  0  0  -  -  -  -  0  0  0  0  0  0  0  1  0  1  1
# mmov.aa          0  1  0  0  0  1  0  -  -  -  -  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
# mmov.ma          0  1  0  0  1  0  0  0  0  -  -  0  0  -  -  -  -  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
# mmov.mm          0  1  0  0  1  1  0  0  0  0  0  0  0  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  1  0  1  1

# mcfgk            0  1  1  0  0  0  0  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  0  0  0  0  1  0  1  1
# mcfgm            0  1  1  0  0  1  0  -  -  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  0  0  1  0  1  1
# mcfgn            0  1  1  0  1  0  0  -  -  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  0  0  0  1  0  1  1
# mmac.dt         0  1  1  0  1  1  0  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  0  0  0  0  0  0  1  0  1  1


Format_OPLOAD = [ 
                    InMReg      (0, Range(11, 4)), #md3
                    InReg       (1, Range(15, 5)), #rs1
                    InReg       (2, Range(20, 5)), #rs2
]

Format_OPSTORE = [ 
                    InMReg      (0, Range(11, 4)), #md3
                    InReg       (1, Range(15, 5)), #rs1
                    InReg       (2, Range(20, 5)), #rs2
]

Format_OPMAC = [ 
                    InMReg      (0, Range(15, 4)), #ms1 
                    InMReg      (1, Range(19, 4)), #ms2
                    InMReg      (2, Range(23, 2)), #md
]

Format_OPZEROM = [ 
                    InMReg      (0, Range(15, 4)), #md
]

Format_OPZEROA = [ 
                    InMReg      (0, Range(23, 2)), #md
]

Format_OPMVAM = [  
                    InMReg      (0, Range(11, 4)), #md
                    InMReg      (1, Range(23, 2)), #accs
]

Format_OPMVAA = [  
                    InMReg      (0, Range(21, 2)), #accd
                    InMReg      (1, Range(23, 2)), #accs
]

Format_OPMVMA = [  
                    InMReg      (0, Range(15, 4)), #ms1
                    InMReg      (1, Range(21, 2)), #accd
]

Format_OPMVMM = [ 
                    InMReg      (0, Range(11, 4)), #md 
                    InMReg      (1, Range(15, 4)), #ms1
]

Format_OPCFGK = [ 
                    InReg       (0, Range(15, 5)), #rs1 
                    InReg       (1, Range(20, 5)), #rd
]

Format_OPCFGMN = [  
                    UnsignedImm (0, Range(13, 2)), #imm2
                    InReg       (0, Range(15, 5)), #rs1 
                    InReg       (1, Range(20, 5)), #rd
]

Format_OPCFGDT = [  
                    UnsignedImm (0, Range(10, 5)), #imm5
                    UnsignedImm (1, Range(15, 5)), #imm5
                    UnsignedImm (2, Range(20, 5)), #imm5
]

class Rv32m(IsaSubset):

    def __init__(self):
        super().__init__(name='M', instrs=[

            Instr('mld.lhs'  ,   Format_OPLOAD , '0000 000- ---- ---- ---- -000 0000 1011'),
            Instr('mld.rhs'  ,   Format_OPLOAD , '0000 010- ---- ---- ---- -000 0000 1011'),
            Instr('mst'      ,   Format_OPSTORE, '0000 100- ---- ---- ---- -000 0000 1011'),

            Instr('mmacc'    ,   Format_OPMAC  , '0010 100- ---- ---- -000 0000 0000 1011'),

            Instr('mzero.m'  ,   Format_OPZEROM, '0010 0100 0000 0--- -000 0000 0000 1011'),
            Instr('mzero.a'  ,   Format_OPZEROA, '0010 000- -000 0000 0000 0000 0000 1011'),

            Instr('mmov.am'  ,   Format_OPMVAM , '0100 000- -000 0000 0--- -000 0000 1011'),
            Instr('mmov.aa'  ,   Format_OPMVAA , '0100 010- ---0 0000 0000 0000 0000 1011'),
            Instr('mmov.ma'  ,   Format_OPMVMA , '0100 1000 0--0 0--- -000 0000 0000 1011'),
            Instr('mmov.mm'  ,   Format_OPMVMM , '0100 1100 0000 0--- ---- -000 0000 1011'),

            Instr('mcfgk'    ,   Format_OPCFGK , '0110 000- ---- ---- -000 0000 0000 1011'),
            Instr('mcfgm'    ,   Format_OPCFGMN, '0110 010- ---- ---- ---0 0000 0000 1011'),
            Instr('mcfgn'    ,   Format_OPCFGMN, '0110 100- ---- ---- ---0 0000 0000 1011'),
            Instr('mmac.dt'  ,   Format_OPCFGDT, '0110 110- ---- ---- ---- --00 0000 1011'),


    ])
