/*
 * Copyright (C) 2020 GreenWaves Technologies, SAS, ETH Zurich and
 *                    University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "cpu/iss/include/isa_lib/mint.h"

static inline iss_reg_t mcfgk_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MCFGK is OK, rd=%d, rs1.value=%d\n", REG_IN(1), insn->in_regs[0]);
    LIB_CALL2(lib_MCFGK, REG_IN(1), REG_GET(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mcfgm_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MCFGM is OK, rd=%d, rs1.value=%d, imm2=%d\n", REG_IN(1), insn->in_regs[0], UIM_GET(0));
    LIB_CALL3(lib_MCFGM, REG_IN(1), REG_GET(0), UIM_GET(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mcfgn_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MCFGN is OK, rd=%d, rs1.value=%d, imm2=%d\n", REG_IN(1), insn->in_regs[0], UIM_GET(0));
    LIB_CALL3(lib_MCFGN, REG_IN(1), REG_GET(0), UIM_GET(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmac_dt_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MCFG_DT is OK, immA=%d, immB=%d, immC=%d\n", UIM_GET(1), UIM_GET(0), UIM_GET(2));
    LIB_CALL3(lib_MCFGDT, UIM_GET(1), UIM_GET(0), UIM_GET(2));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_am_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MMOV.AM is OK, accs=%d, md=%d\n", REG_IN(1), REG_IN(0));
    LIB_CALL2(lib_MOVAM, REG_IN(0), REG_IN(1));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_aa_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MMOV.AA is OK, accs=%d, accd=%d\n", REG_IN(1), REG_IN(0));
    LIB_CALL2(lib_MOVAA, REG_IN(0), REG_IN(1));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MMOV.MA is OK, ms=%d, accd=%d\n", REG_IN(0), REG_IN(1));
    LIB_CALL2(lib_MOVMA, REG_IN(0), REG_IN(1));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_mm_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MMOV.MM is OK, ms=%d, md=%d\n", REG_IN(1), REG_IN(0));
    LIB_CALL2(lib_MOVMM, REG_IN(0), REG_IN(1));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mzero_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MZERO.M is OK, md=%d\n", REG_IN(0));
    LIB_CALL1(lib_MZEROM, REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mzero_a_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MZERO.A is OK, accd=%d\n", REG_IN(0));
    LIB_CALL1(lib_MZEROA, REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmacc_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MMACC is OK, accd=%d, ms1=%d, ms2=%d\n", REG_IN(2), REG_IN(0), REG_IN(1));
    LIB_CALL3(lib_MAC, REG_IN(2), REG_IN(0), REG_IN(1));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mld_lhs_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MLD.LHS is OK, md=%d, rs1=%d, rs2=%d\n", REG_IN(0), REG_IN(1), REG_IN(2));
    LIB_CALL3(lib_MLDL, REG_IN(0), REG_GET(1), REG_GET(2));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mld_rhs_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MLD.RHS is OK, md=%d, rs1=%d, rs2=%d\n", REG_IN(0), REG_IN(1), REG_IN(2));
    LIB_CALL3(lib_MLDR, REG_IN(0), REG_GET(1), REG_GET(2));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mst_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    //printf("MST is OK, ms=%d, rs1=%d, rs2=%d\n", REG_IN(0), REG_IN(1), REG_IN(2));
    LIB_CALL3(lib_MST, REG_IN(0), REG_GET(1), REG_GET(2));
    return iss_insn_next(iss, insn, pc);
}