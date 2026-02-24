
#include "cpu/iss/include/quadrilatero.hpp"

#include "cpu/iss/flexfloat/flexfloat.h"
#include "int.h"
#include <stdint.h>
#include <math.h>
#include <fenv.h>
#include <limits.h>
#include "assert.h"

#define TYPE iss -> quadrilatero.mtype
#define COL  iss -> quadrilatero.active_cols
#define ROW  iss -> quadrilatero.active_rows


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX ZERO
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static inline void lib_MZEROM (Iss *iss, int md){
    
    for(int i=0; i<ROW; i++){
        for(int j=0; j<COL*4; j++){
            iss->quadrilatero.mregfile.mregs[md][i][j] = 0;
        }
    }

}

static inline void lib_MZEROA (Iss *iss, int md){
    
    for(int i=0; i<ROW; i++){
        for(int j=0; j<COL*4; j++){
            iss->quadrilatero.mregfile.maccregs[md][i][j] = 0;
        }
    }
    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX MOVE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static inline void lib_MOVMM (Iss *iss, int md, int ms){
   
    for(int i=0; i<ROW; i++){
        for(int j=0; j<COL*4; j++){
            iss->quadrilatero.mregfile.mregs[md][i][j] = iss->quadrilatero.mregfile.mregs[ms][i][j];
        }
    }
}

static inline void lib_MOVAM (Iss *iss, int md, int ms){
   
    for(int i=0; i<ROW; i++){
        for(int j=0; j<COL*4; j++){
            iss->quadrilatero.mregfile.maccregs[md][i][j] = iss->quadrilatero.mregfile.mregs[ms][i][j];
        }
    }
}

static inline void lib_MOVMA (Iss *iss, int md, int ms){
   
    for(int i=0; i<ROW; i++){
        for(int j=0; j<COL*4; j++){
            iss->quadrilatero.mregfile.mregs[md][i][j] = iss->quadrilatero.mregfile.maccregs[ms][i][j];
        }
    }
}

static inline void lib_MOVAA (Iss *iss, int md, int ms){
   
    for(int i=0; i<ROW; i++){
        for(int j=0; j<COL*4; j++){
            iss->quadrilatero.mregfile.maccregs[md][i][j] = iss->quadrilatero.mregfile.maccregs[ms][i][j];
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX FPU
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static inline void lib_FMMACCB(Iss *iss, int md, int ms1, int ms2) {
    
    int K = COL * 4;

    flexfloat_desc_t desc_fp8  = (flexfloat_desc_t){4, 3}; // E4M3
    flexfloat_desc_t desc_fp32 = (flexfloat_desc_t){8, 23}; 

    int old_frm = setFFRoundingMode(iss, iss->csr.fcsr.frm);

    for (int r = 0; r < ROW; r++){
        for (int c = 0; c < COL; c++){

            flexfloat_t ff_acc;
            ff_acc.desc = desc_fp32;
            flexfloat_set_bits(&ff_acc, 0x00000000);

            for(int k = 0; k < K; k += 4){

                uint8_t a0_bits = iss->quadrilatero.mregfile.mregs[ms1][r][k+0];
                uint8_t a1_bits = iss->quadrilatero.mregfile.mregs[ms1][r][k+1];
                uint8_t a2_bits = iss->quadrilatero.mregfile.mregs[ms1][r][k+2];
                uint8_t a3_bits = iss->quadrilatero.mregfile.mregs[ms1][r][k+3];
                                           
                uint8_t b0_bits = iss->quadrilatero.mregfile.mregs[ms2][c][k+0];
                uint8_t b1_bits = iss->quadrilatero.mregfile.mregs[ms2][c][k+1];
                uint8_t b2_bits = iss->quadrilatero.mregfile.mregs[ms2][c][k+2];
                uint8_t b3_bits = iss->quadrilatero.mregfile.mregs[ms2][c][k+3];

                flexfloat_t ff_a0, ff_a1, ff_a2, ff_a3;
                flexfloat_t ff_b0, ff_b1, ff_b2, ff_b3;

                ff_a0.desc = desc_fp8; flexfloat_set_bits(&ff_a0, a0_bits);
                ff_a1.desc = desc_fp8; flexfloat_set_bits(&ff_a1, a1_bits);
                ff_a2.desc = desc_fp8; flexfloat_set_bits(&ff_a2, a2_bits);
                ff_a3.desc = desc_fp8; flexfloat_set_bits(&ff_a3, a3_bits);

                ff_b0.desc = desc_fp8; flexfloat_set_bits(&ff_b0, b0_bits);
                ff_b1.desc = desc_fp8; flexfloat_set_bits(&ff_b1, b1_bits);
                ff_b2.desc = desc_fp8; flexfloat_set_bits(&ff_b2, b2_bits);
                ff_b3.desc = desc_fp8; flexfloat_set_bits(&ff_b3, b3_bits);

                flexfloat_t ff_a0_32, ff_b0_32, ff_a1_32, ff_b1_32;
                flexfloat_t ff_a2_32, ff_b2_32, ff_a3_32, ff_b3_32;

                ff_a0_32.desc = desc_fp32; ff_a0_32.value = ff_a0.value;
                ff_b0_32.desc = desc_fp32; ff_b0_32.value = ff_b0.value;
                
                ff_a1_32.desc = desc_fp32; ff_a1_32.value = ff_a1.value;
                ff_b1_32.desc = desc_fp32; ff_b1_32.value = ff_b1.value;
                
                ff_a2_32.desc = desc_fp32; ff_a2_32.value = ff_a2.value;
                ff_b2_32.desc = desc_fp32; ff_b2_32.value = ff_b2.value;
                
                ff_a3_32.desc = desc_fp32; ff_a3_32.value = ff_a3.value;
                ff_b3_32.desc = desc_fp32; ff_b3_32.value = ff_b3.value;

                feclearexcept(FE_ALL_EXCEPT);
                ff_fma(&ff_acc, &ff_a0_32, &ff_b0_32, &ff_acc);
                update_fflags_fenv(iss);

                feclearexcept(FE_ALL_EXCEPT);
                ff_fma(&ff_acc, &ff_a1_32, &ff_b1_32, &ff_acc);
                update_fflags_fenv(iss);

                feclearexcept(FE_ALL_EXCEPT);
                ff_fma(&ff_acc, &ff_a2_32, &ff_b2_32, &ff_acc);
                update_fflags_fenv(iss);

                feclearexcept(FE_ALL_EXCEPT);
                ff_fma(&ff_acc, &ff_a3_32, &ff_b3_32, &ff_acc);
                update_fflags_fenv(iss);
            }

            uint32_t prev_md_bits = (uint32_t)((iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] << 0)  |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] << 8)  |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] << 16) |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] << 24));

            flexfloat_t ff_prev;
            ff_prev.desc = desc_fp32;
            flexfloat_set_bits(&ff_prev, prev_md_bits);

            flexfloat_t ff_final;
            ff_final.desc = desc_fp32;
            
            feclearexcept(FE_ALL_EXCEPT);
            ff_add(&ff_final, &ff_acc, &ff_prev);
            update_fflags_fenv(iss);

            uint32_t final_bits = flexfloat_get_bits(&ff_final);

            iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] = (uint8_t)(final_bits & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] = (uint8_t)((final_bits >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] = (uint8_t)((final_bits >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] = (uint8_t)((final_bits >> 24) & 0xFF);                            
        }
    }

    restoreFFRoundingMode(old_frm);

    printf("\n--- MATRIX FMMACCB (md=%d)(ms1=%d)(ms2=%d) ---\n", md, ms1, ms2);
    for(int i = 0; i < ROW; i++){
        printf("Row %d: ", i);
        for(int w = 0; w < COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+3],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+0]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}

static inline void lib_FMMACCH(Iss *iss, int md, int ms1, int ms2) {
    
    int K = COL * 4;

    flexfloat_desc_t desc_fp16 = (flexfloat_desc_t){5, 10};
    flexfloat_desc_t desc_fp32 = (flexfloat_desc_t){8, 23}; 

    int old_frm = setFFRoundingMode(iss, iss->csr.fcsr.frm);

    for (int r = 0; r < ROW; r++){
        for (int c = 0; c < COL; c++){

            flexfloat_t ff_acc;
            ff_acc.desc = desc_fp32;
            flexfloat_set_bits(&ff_acc, 0x00000000);

            for(int k = 0; k < K; k += 4){

                uint16_t a0_bits = (uint16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+0] << 0) |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+1] << 8) );
                                           
                uint16_t a1_bits = (uint16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+2] << 0) |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+3] << 8) );

                uint16_t b0_bits = (uint16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+0] << 0) |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+1] << 8) );
                                           
                uint16_t b1_bits = (uint16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+2] << 0) |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+3] << 8) );

                flexfloat_t ff_a0, ff_a1, ff_b0, ff_b1;

                ff_a0.desc = desc_fp16;
                ff_a1.desc = desc_fp16;
                ff_b0.desc = desc_fp16;
                ff_b1.desc = desc_fp16;

                flexfloat_set_bits(&ff_a0, a0_bits);
                flexfloat_set_bits(&ff_a1, a1_bits);
                flexfloat_set_bits(&ff_b0, b0_bits);
                flexfloat_set_bits(&ff_b1, b1_bits);

                flexfloat_t ff_a0_32, ff_b0_32, ff_a1_32, ff_b1_32;

                ff_a0_32.desc = desc_fp32; ff_a0_32.value = ff_a0.value;
                ff_b0_32.desc = desc_fp32; ff_b0_32.value = ff_b0.value;
                ff_a1_32.desc = desc_fp32; ff_a1_32.value = ff_a1.value;
                ff_b1_32.desc = desc_fp32; ff_b1_32.value = ff_b1.value;

                feclearexcept(FE_ALL_EXCEPT);
                ff_fma(&ff_acc, &ff_a0_32, &ff_b0_32, &ff_acc);
                update_fflags_fenv(iss);

                feclearexcept(FE_ALL_EXCEPT);
                ff_fma(&ff_acc, &ff_a1_32, &ff_b1_32, &ff_acc);
                update_fflags_fenv(iss);
            }

            uint32_t prev_md_bits = (uint32_t)((iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] << 0)  |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] << 8)  |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] << 16) |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] << 24));

            flexfloat_t ff_prev;
            ff_prev.desc = desc_fp32;
            flexfloat_set_bits(&ff_prev, prev_md_bits);

            flexfloat_t ff_final;
            ff_final.desc = desc_fp32;
            
            feclearexcept(FE_ALL_EXCEPT);
            ff_add(&ff_final, &ff_acc, &ff_prev);
            update_fflags_fenv(iss);

            uint32_t final_bits = flexfloat_get_bits(&ff_final);

            iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] = (uint8_t)(final_bits & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] = (uint8_t)((final_bits >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] = (uint8_t)((final_bits >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] = (uint8_t)((final_bits >> 24) & 0xFF);                            
        }
    }

    restoreFFRoundingMode(old_frm);

    printf("\n--- MATRIX FMMACCH (md=%d)(ms1=%d)(ms2=%d) ---\n", md, ms1, ms2);
    for(int i = 0; i < ROW; i++){
        printf("Row %d: ", i);
        for(int w = 0; w < COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+3],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+0]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}

static inline void lib_FMMACCW(Iss *iss, int md, int ms1, int ms2) {
    
    int K = COL * 4;

    flexfloat_desc_t desc_fp32 = (flexfloat_desc_t){8, 23}; 

    int old_frm = setFFRoundingMode(iss, iss->csr.fcsr.frm);

    for (int r = 0; r < ROW; r++){
        for (int c = 0; c < COL; c++){

            flexfloat_t ff_acc;
            ff_acc.desc = desc_fp32;
            flexfloat_set_bits(&ff_acc, 0x00000000);

            for(int k = 0; k < K; k += 4){

                uint32_t a0_bits = (uint32_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+0] << 0)  |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+1] << 8)  |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+2] << 16) |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+3] << 24) );
                                           
                uint32_t b0_bits = (uint32_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+0] << 0)  |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+1] << 8)  |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+2] << 16) |
                                               ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+3] << 24) );

                flexfloat_t ff_a0, ff_b0;
                
                ff_a0.desc = desc_fp32;
                ff_b0.desc = desc_fp32;

                flexfloat_set_bits(&ff_a0, a0_bits);
                flexfloat_set_bits(&ff_b0, b0_bits);

                feclearexcept(FE_ALL_EXCEPT);
                ff_fma(&ff_acc, &ff_a0, &ff_b0, &ff_acc);
                update_fflags_fenv(iss);
            }

            uint32_t prev_md_bits = (uint32_t)((iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] << 0)  |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] << 8)  |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] << 16) |
                                               (iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] << 24));

            flexfloat_t ff_prev;
            ff_prev.desc = desc_fp32;
            flexfloat_set_bits(&ff_prev, prev_md_bits);

            flexfloat_t ff_final;
            ff_final.desc = desc_fp32;
            
            feclearexcept(FE_ALL_EXCEPT);
            ff_add(&ff_final, &ff_acc, &ff_prev);
            update_fflags_fenv(iss);

            uint32_t final_bits = flexfloat_get_bits(&ff_final);

            iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] = (uint8_t)(final_bits & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] = (uint8_t)((final_bits >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] = (uint8_t)((final_bits >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] = (uint8_t)((final_bits >> 24) & 0xFF);                            
        }
    }

    restoreFFRoundingMode(old_frm);

    // DEBUG
    printf("\n--- MATRIX FMMACCW (md=%d)(ms1=%d)(ms2=%d) ---\n", md, ms1, ms2);
    for(int i = 0; i < ROW; i++){
        printf("Row %d: ", i);
        for(int w = 0; w < COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+3],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+0]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX ALU
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static inline void lib_MMAQAB(Iss *iss, int md, int ms1, int ms2) {
    
    int K = COL*4;

    for (int r=0; r < ROW; r++){
        for (int c=0; c < COL; c++){

            int64_t accumulator = 0;

            for(int k=0; k < K; k+=4){

                int8_t a0 = (int8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+0];
                int8_t a1 = (int8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+1];
                int8_t a2 = (int8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+2];
                int8_t a3 = (int8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+3];

                int8_t b0 = (int8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+0];
                int8_t b1 = (int8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+1];
                int8_t b2 = (int8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+2];
                int8_t b3 = (int8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+3];

                accumulator += (int64_t)a0 * b0;
                accumulator += (int64_t)a1 * b1;
                accumulator += (int64_t)a2 * b2;
                accumulator += (int64_t)a3 * b3;
            }

            int32_t prev_md = (int32_t)((iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] << 0)  |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] << 8)  |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] << 16) |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] << 24));

            int64_t final_val = accumulator + prev_md;

            if (final_val > INT32_MAX) final_val = INT32_MAX;
            if (final_val < INT32_MIN) final_val = INT32_MIN;

            iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] = (uint8_t)(final_val & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] = (uint8_t)((final_val >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] = (uint8_t)((final_val >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] = (uint8_t)((final_val >> 24) & 0xFF);                            

        }
    }

    //DEBUG
    printf("\n--- MATRIX ACC MMAQAB (md=%d)(ms1=%d)(ms2=%d) ---\n", md, ms1, ms2);
    for(int i=0; i<ROW; i++){
        printf("Row %d: ", i);
        for(int w=0; w<COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+3],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+0]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}

static inline void lib_MMADAH(Iss *iss, int md, int ms1, int ms2) {
    
    int K = COL * 4;

    for (int r = 0; r < ROW; r++){
        for (int c = 0; c < COL; c++){

            int64_t accumulator = 0;

            for(int k = 0; k < K; k += 4){

                int16_t a0 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+0] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+1] << 8) );
                                           
                int16_t a1 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+2] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[ms1][r][k+3] << 8) );

                int16_t b0 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+0] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+1] << 8) );
                                           
                int16_t b1 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+2] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[ms2][c][k+3] << 8) );

                accumulator += (int64_t)a0 * b0;
                accumulator += (int64_t)a1 * b1;
            }

            int32_t prev_md = (int32_t)((iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] << 0)  |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] << 8)  |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] << 16) |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] << 24));

            int64_t final_val = accumulator + prev_md;

            if (final_val > INT32_MAX) final_val = INT32_MAX;
            if (final_val < INT32_MIN) final_val = INT32_MIN;

            iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] = (uint8_t)(final_val & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] = (uint8_t)((final_val >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] = (uint8_t)((final_val >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] = (uint8_t)((final_val >> 24) & 0xFF);                            

        }
    }

    // DEBUG
    printf("\n--- MATRIX ACC MMADAH (md=%d)(ms1=%d)(ms2=%d) ---\n", md, ms1, ms2);
    for(int i = 0; i < ROW; i++){
        printf("Row %d: ", i);
        for(int w = 0; w < COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+3],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+0]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}

static inline void lib_MMASAW(Iss *iss, int md, int ms1, int ms2) {
    
    int K = COL * 4;

    for (int r = 0; r < ROW; r++){
        for (int c = 0; c < COL; c++){

            int64_t accumulator = 0;

            for(int k = 0; k < K; k += 4){
                
                int32_t a0 = (int32_t)(  (iss->quadrilatero.mregfile.mregs[ms1][r][k+0] << 0)  |
                                         (iss->quadrilatero.mregfile.mregs[ms1][r][k+1] << 8)  |
                                         (iss->quadrilatero.mregfile.mregs[ms1][r][k+2] << 16) |
                                         (iss->quadrilatero.mregfile.mregs[ms1][r][k+3] << 24));

                int32_t b0 = (int32_t)(  (iss->quadrilatero.mregfile.mregs[ms2][c][k+0] << 0)  |
                                         (iss->quadrilatero.mregfile.mregs[ms2][c][k+1] << 8)  |
                                         (iss->quadrilatero.mregfile.mregs[ms2][c][k+2] << 16) |
                                         (iss->quadrilatero.mregfile.mregs[ms2][c][k+3] << 24));
                
                accumulator += (int64_t)a0 * b0;
            }

            int32_t prev_md = (int32_t)((iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] << 0)  |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] << 8)  |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] << 16) |
                                        (iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] << 24));

            int64_t final_val = accumulator + prev_md;

            if (final_val > INT32_MAX) final_val = INT32_MAX;
            if (final_val < INT32_MIN) final_val = INT32_MIN;

            iss->quadrilatero.mregfile.maccregs[md][r][c*4+0] = (uint8_t)(final_val & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+1] = (uint8_t)((final_val >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+2] = (uint8_t)((final_val >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[md][r][c*4+3] = (uint8_t)((final_val >> 24) & 0xFF);                            

        }
    }

    // DEBUG
    printf("\n--- MATRIX ACC MMASAW (md=%d)(ms1=%d)(ms2=%d) ---\n", md, ms1, ms2);
    for(int i = 0; i < ROW; i++){
        printf("Row %d: ", i);
        for(int w = 0; w < COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+3],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.maccregs[md][i][w*4+0]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX LOAD STORE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline int Mlsu::Mlsu_io_access(Iss *iss, uint64_t addr, int size, uint8_t *data, bool is_write)
{
    this->io_pending_addr = addr;
    this->io_pending_size = size;
    this->io_pending_data = data;
    this->io_pending_is_write = is_write;
    this->waiting_io_response = true;
    this->handle_pending_io_access(iss);

    return this->io_retval;
}

inline void Mlsu::handle_pending_io_access(Iss *iss)
{
    if (this->io_pending_size > 0){
        vp::IoReq *req = &this->io_req;

        uint32_t addr = this->io_pending_addr;        
        uint32_t addr_aligned = addr & ~(4 - 1);
        int size = addr_aligned + 4 - addr;
        // printf("size = %d\n" , size);    
        if (size > this->io_pending_size){
            size = this->io_pending_size;
        }

        req->init();
        req->set_addr(addr);
        req->set_size(size);
        req->set_is_write(this->io_pending_is_write);
        req->set_data(this->io_pending_data);


        this->io_pending_data += size;
        this->io_pending_size -= size;
        this->io_pending_addr += size;

        int err = this->io_itf[0].req(req);

        if (err == vp::IO_REQ_INVALID){
            this->waiting_io_response = false;
            this->io_retval = 1;
        }
    }
    else{
        this->waiting_io_response = false;
        this->io_retval = 0;
    }
}

static inline void lib_MLDL(Iss *iss, uint64_t rs1, int md){
    uint64_t add = rs1;

    for(int i=0; i<ROW; i++){     
        for(int w=0; w<COL; w++){ 
            
            uint8_t word_data[4];
            
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, add, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[md][i][w*4+0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[md][i][w*4+1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[md][i][w*4+2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[md][i][w*4+3] = word_data[3];
            
            add += 4; 
        }
    }

    //DEBUG
    printf("\n--- MATRIX m%d (LHS) ---\n", md);
    for(int i=0; i<ROW; i++){
        printf("Row %d: ", i);
        for(int w=0; w<COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.mregs[md][i][w*4+0],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+3]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}

static inline void lib_MLD8R(Iss *iss, uint64_t rs1, int md){
    
    uint64_t add = rs1;

    for(int c=0; c<COL*4; c+=1){ 
        for(int r=0; r < ROW; r+=4){
            
            uint8_t word_data[4];
            
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, add, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[md][r][c]   = word_data[0];
            iss->quadrilatero.mregfile.mregs[md][r+1][c] = word_data[1];
            iss->quadrilatero.mregfile.mregs[md][r+2][c] = word_data[2];
            iss->quadrilatero.mregfile.mregs[md][r+3][c] = word_data[3];
            
            add += 4;
        }
    }

    // DEBUG
    printf("\n--- MATRIX m%d (RHS - TRANSPOSED - 8-BIT) ---\n", md);
    for(int i=0; i<ROW; i++){
        printf("Row %d: ", i);
        for(int w=0; w<COL; w++){ 
            printf("[%02x] [%02x] [%02x] [%02x] ", 
                iss->quadrilatero.mregfile.mregs[md][i][w*4+0],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+3]);
        }
        printf("\n");
    }
    printf("--------------------------------------\n");
}

static inline void lib_MLD16R(Iss *iss, uint64_t rs1, int md){
    
    uint64_t add = rs1;

    for(int c=0; c<COL*4; c+=2){ 
        for(int r=0; r < ROW; r+=2){
            
            uint8_t word_data[4];
            
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, add, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[md][r][c]     = word_data[0];
            iss->quadrilatero.mregfile.mregs[md][r][c+1]   = word_data[1];
            
            iss->quadrilatero.mregfile.mregs[md][r+1][c]   = word_data[2];
            iss->quadrilatero.mregfile.mregs[md][r+1][c+1] = word_data[3];
            
            add += 4;
        }
    }

    // DEBUG
    printf("\n--- MATRIX m%d (RHS - TRANSPOSED - 16-BIT) ---\n", md);
    for(int i=0; i<ROW; i++){
        printf("Row %d: ", i);
        for(int w=0; w<COL; w++){ 
            printf("[%02x %02x] [%02x %02x] ", 
                iss->quadrilatero.mregfile.mregs[md][i][w*4+0],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+3]);
        }
        printf("\n");
    }
    printf("--------------------------------------\n");
}


static inline void lib_MLD32R(Iss *iss, uint64_t rs1, int md){
    uint64_t add = rs1;

    for(int c=0; c<COL; c++){ 
        for(int r=0; r<ROW; r++){
            
            uint8_t word_data[4];
            
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, add, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[md][r][c*4+0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[md][r][c*4+1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[md][r][c*4+2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[md][r][c*4+3] = word_data[3];
            
            add += 4;
        }
    }

    // DEBUG
    printf("\n--- MATRIX m%d (RHS - TRANSPOSED - 32-BIT) ---\n", md);
    for(int i=0; i<ROW; i++){
        printf("Row %d: ", i);
        for(int w=0; w<COL; w++){ 
            printf("[%02x %02x %02x %02x] ", 
                iss->quadrilatero.mregfile.mregs[md][i][w*4+0],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+1],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+2],
                iss->quadrilatero.mregfile.mregs[md][i][w*4+3]);
        }
        printf("\n");
    }
    printf("--------------------------------------\n");
}


static inline void lib_MST (Iss *iss, iss_reg_t rs1, iss_reg_t rs2, int ms){
    uint64_t start_add = rs1;
    uint64_t stride = rs2;

    uint64_t add = rs1;

    for(int r=0; r<ROW; r++){     
        for(int c=0; c<COL; c++){ 
            
            uint8_t word_data[4];
            
            word_data[0] = iss->quadrilatero.mregfile.mregs[ms][r][c*4+0];
            word_data[1] = iss->quadrilatero.mregfile.mregs[ms][r][c*4+1];
            word_data[2] = iss->quadrilatero.mregfile.mregs[ms][r][c*4+2];
            word_data[3] = iss->quadrilatero.mregfile.mregs[ms][r][c*4+3];
            
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, add, 4, word_data, true);

            add += 4; 
        }
    }
}
