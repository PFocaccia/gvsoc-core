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

        # 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
#OPM     |  funct4   | 0  0  0  0|  ms2   |  ms1   | 0|  md | 0  0  0|  funct5   |     op           
#OPMMV   |  funct4   | 0  0  0  0  0  0  0|  ms1   |   md   | 0  0  0|  funct5   |     op      
#OPMZ    |  funct4   | 0  0  0| f| 0  0  0  0  0  0|   md   | 0  0  0|  funct5   |     op       
#OPMLS      | 0  0  0| m|  ldst  |      rs2     |     rs1      |     | size| mds3|     op
#OPMC    | 0| funct3 | 1  1  1  0  0         |     rs1      | 0  0  0|     rd    |     op 
#OPMCI   | 0| funct3 | 1  1  1  0  0|   imm  |     rs1      | 0  0  0|     rd    |     op 


Format_OPM = [ 
                OutMAcc     (0, Range(15, 2)),
                InMReg      (0, Range(18, 3)), #ms1
                InMReg      (1, Range(21, 3)), #ms2
]

Format_OPMF = [ 
                OutMAccF     (0, Range(15, 2)),
                InMRegF      (0, Range(18, 3)), #ms1
                InMRegF      (1, Range(21, 3)), #ms2
]

Format_OPMVMM = [ 
                OutMReg     (0, Range(15, 3)),
                InMReg      (0, Range(18, 3)),#ms1
]

Format_OPMVMA = [ 
                OutMAcc     (0, Range(15, 3)),
                InMReg      (0, Range(18, 3)), #ms1
]

Format_OPMVAM = [ 
                OutMReg     (0, Range(15, 3)),
                InMAcc      (0, Range(18, 3)),#ms1
]

Format_OPMVAA = [ 
                OutMAcc     (0, Range(15, 3)),
                InMAcc      (0, Range(18, 3)),#ms1
]

Format_OPMZM = [ OutMReg     (0, Range(15, 3))]

Format_OPMZA = [ OutMAcc     (0, Range(15, 3))]

Format_OPML = [ 
                OutMReg    (0, Range(7 , 3)),
                InReg      (0, Range(15, 5)),#rs1
                InReg      (1, Range(20, 5)),#rs2
]

Format_OPMS = [ 
                OutMReg    (0, Range(7 , 3)),
                InReg      (0, Range(15, 5)),#rs1
                InReg      (1, Range(20, 5)),#rs2
]


Format_OPMC = [ InReg      (0, Range(7, 5)),  #rd
                InReg      (1, Range(15, 5)), #rs
]

Format_OPMCI = [ InReg      (0, Range(7, 5)),  #rd
                 InReg      (1, Range(15, 5)), #rs
                 UnsignedImm(0, Range(20, 3)),
]

class Rv32m(IsaSubset):

    def __init__(self):
        super().__init__(name='M', instrs=[

            Instr('mcfgk.xi'     ,   Format_OPMCI,    '000011100--------000-----0001011'),
            Instr('mcfgm.x'      ,   Format_OPMC,     '000111100--------000-----0001011'),
            Instr('mcfgn.x'      ,   Format_OPMC,     '001011100--------000-----0001011'),
            
            Instr('fmmaccb.ma'   ,   Format_OPMF,     '0001 000 0 --- --- 0 -- 000 00000 0001011'),
            Instr('fmmacch.ma'   ,   Format_OPMF,     '0001 000 0 --- --- 0 -- 000 01000 0001011'),
            Instr('fmmaccw.ma'   ,   Format_OPMF,     '0001 000 0 --- --- 0 -- 000 10000 0001011'),

            Instr('mmaqab.ma'    ,   Format_OPM,      '00100000------0--000000000001011'),
            Instr('mmadah.ma'    ,   Format_OPM,      '00100000------0--000010000001011'),
            Instr('mmasaw.ma'    ,   Format_OPM,      '00100000------0--000100000001011'),

            Instr('mmov.mm'      ,   Format_OPMVMM,   '00000000000------000000010001011'),
            Instr('mmov.ma'      ,   Format_OPMVMA,   '00000000000------000010010001011'),
            Instr('mmov.am'      ,   Format_OPMVAM,   '00000000000------000100010001011'),
            Instr('mmov.aa'      ,   Format_OPMVAA,   '00000000000------000110010001011'),

            Instr('mzero.m'      ,   Format_OPMZM,    '10100000000000---000000000001011'),
            Instr('mzero.a'      ,   Format_OPMZA,    '10100001000000---000000000001011'),

            Instr('mldlhsb.m'    ,   Format_OPML,     '000 0 100 ----- ----- --- 00 --- 0001011'),
            Instr('mldlhsh.m'    ,   Format_OPML,     '000 0 100 ----- ----- --- 01 --- 0001011'),                                            
            Instr('mldlhsw.m'    ,   Format_OPML,     '000 0 100 ----- ----- --- 10 --- 0001011'),
            Instr('mldrhsb.m'    ,   Format_OPML,     '000 1 100 ----- ----- --- 00 --- 0001011'),
            Instr('mldrhsh.m'    ,   Format_OPML,     '000 1 100 ----- ----- --- 01 --- 0001011'),
            Instr('mldrhsw.m'    ,   Format_OPML,     '000 1 100 ----- ----- --- 10 --- 0001011'),

            Instr('mstb.m'       ,   Format_OPMS,     '000 0 101 ----- ----- --- 00 --- 0001011'),
            Instr('msth.m'       ,   Format_OPMS,     '000 0 101 ----- ----- --- 01 --- 0001011'),
            Instr('mstw.m'       ,   Format_OPMS,     '000 0 101 ----- ----- --- 10 --- 0001011'),
    ])
