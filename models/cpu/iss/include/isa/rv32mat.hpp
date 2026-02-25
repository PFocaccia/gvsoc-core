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

static inline iss_reg_t mcfgk_xi_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_MCFGK, insn->in_regs[0], REG_GET(1), UIM_GET(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mcfgn_x_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MCFGN, insn->in_regs[0], REG_GET(1));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mcfgm_x_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MCFGM, insn->in_regs[0], REG_GET(1));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t fmmaccb_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_FMMACCB,REG_OUT(0),REG_IN(1),REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t fmmacch_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_FMMACCH,REG_OUT(0),REG_IN(1),REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t fmmaccw_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_FMMACCW,REG_OUT(0),REG_IN(1),REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmaqab_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_MMAQAB,REG_OUT(0),REG_IN(1),REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmadah_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_MMADAH,REG_OUT(0),REG_IN(1),REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmasaw_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_MMASAW,REG_OUT(0),REG_IN(1),REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_mm_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MOVMM , REG_OUT(0), REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_ma_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MOVMA , REG_OUT(0), REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_am_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MOVAM , REG_OUT(0), REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mmov_aa_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MOVAA , REG_OUT(0), REG_IN(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mzero_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL1(lib_MZEROM , REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mzero_a_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL1(lib_MZEROA , REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mldlhsb_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MLDL, REG_GET(0), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mldlhsh_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MLDL, REG_GET(0), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mldlhsw_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MLDL, REG_GET(0), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mldrhsb_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MLD8R, REG_GET(0), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mldrhsh_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MLD16R, REG_GET(0), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mldrhsw_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL2(lib_MLD32R, REG_GET(0), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mstb_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_MST, REG_GET(0), REG_GET(1), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t msth_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_MST, REG_GET(0), REG_GET(1), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}

static inline iss_reg_t mstw_m_exec(Iss *iss, iss_insn_t *insn, iss_reg_t pc){
    LIB_CALL3(lib_MST, REG_GET(0), REG_GET(1), REG_OUT(0));
    return iss_insn_next(iss, insn, pc);
}
