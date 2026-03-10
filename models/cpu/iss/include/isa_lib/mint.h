
#include "cpu/iss/include/quadrilatero.hpp"

#include "cpu/iss/flexfloat/flexfloat.h"
#include "int.h"
#include <stdint.h>
#include <math.h>
#include <fenv.h>
#include <limits.h>
#include "assert.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX CFG
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void lib_MCFGK(Iss *iss, int rd, uint64_t requested_k) {

    uint64_t max_k = RLEN / 32;

    uint64_t granted_k = (requested_k < max_k) ? requested_k : max_k;
    
    iss->csr.matrix_k.value = granted_k;
    iss->regfile.set_reg(rd, granted_k);

    printf("MCFGK: K set to %lu (Req: %lu)\n", granted_k, requested_k);
}

static inline void lib_MCFGM(Iss *iss, int rd, uint64_t requested_m, uint8_t imm2) {
    
    iss->csr.matrix_rmul.value = imm2 + 1;

    uint64_t max_m = (RLEN/32)*iss->csr.matrix_rmul.value; 
    
    uint64_t granted_m = (requested_m < max_m) ? requested_m : max_m;
    
    iss->csr.matrix_m.value = granted_m;

    iss->regfile.set_reg(rd, granted_m);

    printf("MCFGM: M set to %lu (Requested: %lu) and RMUL to %lu\n", granted_m, requested_m, iss->csr.matrix_rmul.value);
}

static inline void lib_MCFGN(Iss *iss, int rd, uint64_t requested_n, uint8_t imm2) {
    
    iss->csr.matrix_cmul.value = imm2 + 1;

    uint64_t max_n = (RLEN/32)*iss->csr.matrix_cmul.value;;

    uint64_t granted_n = (requested_n < max_n) ? requested_n : max_n;
    
    iss->csr.matrix_n.value = granted_n;
    
    iss->regfile.set_reg(rd, granted_n);
    
    printf("MCFGN: N set to %lu (Requested: %lu) and CMUL to %lu\n", granted_n, requested_n, iss->csr.matrix_cmul.value);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX ZERO
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static inline void lib_MZEROM (Iss *iss, int md){
    
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;

    int dim = RLEN/32;

    if (md % (cmul*rmul) != 0){
        printf("(mzerom) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    for(int i=0; i<rmul; i++){
        for(int j=0; j<cmul; j++){
            
            int nreg = md + (cmul * i) + j;
            
            for(int x=0; x<dim; x++){
                for(int z=0; z<(dim*4); z++){
                    iss->quadrilatero.mregfile.mregs[nreg][x][z] = 0;
                }
            }
        }
    }

    /*
    printf("\n--- MZEROM (md=%d)---\n", md);
    
    for(int reg = 0; reg < ISS_NB_MREGS; reg++){
        
        printf("Physical Register m%d:\n", reg);
        
        for(int i = 0; i < 4; i++){
            printf("Row %d: ", i);
            for(int w = 0; w < 4; w++){ 
                printf("[%02x %02x %02x %02x] ", 
                    iss->quadrilatero.mregfile.mregs[reg][i][w*4+3],
                    iss->quadrilatero.mregfile.mregs[reg][i][w*4+2],
                    iss->quadrilatero.mregfile.mregs[reg][i][w*4+1],
                    iss->quadrilatero.mregfile.mregs[reg][i][w*4+0]);
            }
            printf("\n");
        }
    }
    printf("-----------------------------\n");
    */
}

static inline void lib_MZEROA (Iss *iss, int md){
    
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;

    int dim = RLEN/32;

    if (md % (cmul*rmul) != 0){
        printf("(mzeroa) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    for(int i=0; i<rmul; i++){
        for(int j=0; j<cmul; j++){
            
            int nreg = md + (cmul * i) + j;
            
            for(int x=0; x<dim; x++){
                for(int z=0; z<(dim*4); z++){
                    iss->quadrilatero.mregfile.maccregs[nreg][x][z] = 0;
                }
            }
        }
    }
    
    /*
    printf("\n--- MZEROA (accd=%d) ---\n", md);
    
    for(int reg = 0; reg < ISS_NB_ACCREGS; reg++){
        
        printf("Physical Register acc%d:\n", reg);
        
        for(int i = 0; i < 4; i++){
            printf("Row %d: ", i);
            for(int w = 0; w < 4; w++){ 
                printf("[%02x %02x %02x %02x] ", 
                    iss->quadrilatero.mregfile.maccregs[reg][i][w*4+3],
                    iss->quadrilatero.mregfile.maccregs[reg][i][w*4+2],
                    iss->quadrilatero.mregfile.maccregs[reg][i][w*4+1],
                    iss->quadrilatero.mregfile.maccregs[reg][i][w*4+0]);
            }
            printf("\n");
        }
    }
    printf("-----------------------------\n");
    */
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX MOVE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static inline void lib_MOVMM (Iss *iss, int md, int ms){
   
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;

    int dim = RLEN/32;

    if (md % (cmul*rmul) != 0){
        printf("(movmm) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    for(int i=0; i<rmul; i++){
        for(int j=0; j<cmul; j++){
            
            int nregd = md + (cmul * i) + j;
            int nregs = ms + (cmul * i) + j;

            for(int x=0; x<dim; x++){
                for(int z=0; z<(dim*4); z++){
                    iss->quadrilatero.mregfile.mregs[nregd][x][z] = iss->quadrilatero.mregfile.mregs[nregs][x][z];
                }
            }
        }
    }

}

static inline void lib_MOVAM (Iss *iss, int md, int ms){
   
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;

    int dim = RLEN/32;

    if (md % (cmul*rmul) != 0){
        printf("(movam) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    for(int i=0; i<rmul; i++){
        for(int j=0; j<cmul; j++){
            
            int nregd = md + (cmul * i) + j;
            int nregs = ms + (cmul * i) + j;

            for(int x=0; x<dim; x++){
                for(int z=0; z<(dim*4); z++){
                    iss->quadrilatero.mregfile.maccregs[nregd][x][z] = iss->quadrilatero.mregfile.mregs[nregs][x][z];
                }
            }
        }
    }

}

static inline void lib_MOVMA (Iss *iss, int md, int ms){
   
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;

    int dim = RLEN/32;

    if (md % (cmul*rmul) != 0){
        printf("(movma) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    for(int i=0; i<rmul; i++){
        for(int j=0; j<cmul; j++){
            
            int nregd = md + (cmul * i) + j;
            int nregs = ms + (cmul * i) + j;

            for(int x=0; x<dim; x++){
                for(int z=0; z<(dim*4); z++){
                    iss->quadrilatero.mregfile.mregs[nregd][x][z] = iss->quadrilatero.mregfile.maccregs[nregs][x][z];
                }
            }
        }
    }
}

static inline void lib_MOVAA (Iss *iss, int md, int ms){
   
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;

    int dim = RLEN/32;

    if (md % (cmul*rmul) != 0){
        printf("(movaa) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    for(int i=0; i<rmul; i++){
        for(int j=0; j<cmul; j++){
            
            int nregd = md + (cmul * i) + j;
            int nregs = ms + (cmul * i) + j;

            for(int x=0; x<dim; x++){
                for(int z=0; z<(dim*4); z++){
                    iss->quadrilatero.mregfile.maccregs[nregd][x][z] = iss->quadrilatero.mregfile.maccregs[nregs][x][z];
                }
            }
        }
    }
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

/*
static inline void lib_MLD8L(Iss *iss, int md, uint64_t rs1, uint64_t rs2) {
    
    int rmul = iss->csr.matrix_rmul.value;
    int M = iss->csr.matrix_m.value;
    int K = iss->csr.matrix_k.value;
    
    int dim = RLEN/32;

    if (md % rmul != 0){
        printf("(mld.lhs.b) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul );
        return;
    }

    uint64_t addr = rs1;
    uint64_t stride = rs2;   

    int n_row = dim; 
    int n_col = dim;
    
    int tiles_per_row = (M + n_col - 1) / n_col; 
    int tiles_per_col = (K + n_row - 1) / n_row; 

    int tot_reg = tiles_per_row * tiles_per_col;

    if (md + tot_reg > ISS_NB_MREGS) {             
        printf("(mld.lhs.b) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int k = 0; k < K; k++) {     
        
        uint64_t current_row_addr = addr + (k * stride);
        
        for(int m = 0; m < M; m++) { 
            
            uint64_t elem_addr = current_row_addr + (m * 4);
            
            int rowT = k / n_row;  
            int colT = m / n_col;  
            int mdT = md + (rowT * tiles_per_row) + colT;

            int row = k % n_row;           
            int col = (m % n_col) * 4;       

            uint8_t word_data[4];
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, elem_addr, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[mdT][row][col + 0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 3] = word_data[3];
        }
    }

    // DEBUG
    printf("\n--- (mld.lhs.b)(md=%d) - Logical: %dx%d (but is trasposed)---\n", md, M, K);
    
    for (int r = md; r < md + tot_reg ; r++) {
        
        printf("Physical Register m%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x] [%02x] [%02x] [%02x] ", 
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");
}

static inline void lib_MLD16L(Iss *iss, int md, uint64_t rs1, uint64_t rs2) {
    
    int rmul = iss->csr.matrix_rmul.value;
    int M = iss->csr.matrix_m.value;
    int K = iss->csr.matrix_k.value;
    
    int dim = RLEN/32;

    if (md % rmul != 0){
        printf("(mld.lhs.h) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul );
        return;
    }

    uint64_t addr = rs1;
    uint64_t stride = rs2;   

    int n_row = dim; 
    int n_col = dim;
    
    int tiles_per_row = (M + n_col - 1) / n_col; 
    int tiles_per_col = (K + n_row - 1) / n_row; 

    int tot_reg = tiles_per_row * tiles_per_col;

    if (md + tot_reg > ISS_NB_MREGS) {             
        printf("(mld.lhs.h) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int k = 0; k < K; k++) {     
        
        uint64_t current_row_addr = addr + (k * stride);
        
        for(int m = 0; m < M; m++) { 
            
            uint64_t elem_addr = current_row_addr + (m * 4);
            
            int rowT = k / n_row;  
            int colT = m / n_col;  
            int mdT = md + (rowT * tiles_per_row) + colT;

            int row = k % n_row;           
            int col = (m % n_col) * 4;       

            uint8_t word_data[4];
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, elem_addr, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[mdT][row][col + 0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 3] = word_data[3];
        }
    }

    // DEBUG
    printf("\n--- (mld.lhs.h)(md=%d) - Logical: %dx%d (but is trasposed)---\n", md, M, K);
    
    for (int r = md; r < md + tot_reg ; r++) {
        
        printf("Physical Register m%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x %02x] [%02x %02x] ", 
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");
}
*/

static inline void lib_MLD32L(Iss *iss, int md, uint64_t rs1, uint64_t rs2) {
    
    int rmul = iss->csr.matrix_rmul.value;
    int M = iss->csr.matrix_m.value;
    int K = iss->csr.matrix_k.value;
    
    int dim = RLEN/32;

    if (md % rmul != 0){
        printf("(mld.lhs.w) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul );
        return;
    }

    uint64_t addr = rs1;
    uint64_t stride = rs2;   

    int n_row = dim; 
    int n_col = dim;
    
    int tiles_per_row = (M + n_col - 1) / n_col; 
    int tiles_per_col = (K + n_row - 1) / n_row; 

    int tot_reg = tiles_per_row * tiles_per_col;

    if (md + tot_reg > ISS_NB_MREGS) {             
        printf("(mld.lhs.w) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int k = 0; k < K; k++) {     
        
        uint64_t current_row_addr = addr + (k * stride);
        
        for(int m = 0; m < M; m++) { 
            
            uint64_t elem_addr = current_row_addr + (m * 4);
            
            int rowT = k / n_row;  
            int colT = m / n_col;  
            int mdT = md + (rowT * tiles_per_row) + colT;

            int row = k % n_row;           
            int col = (m % n_col) * 4;       

            uint8_t word_data[4];
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, elem_addr, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[mdT][row][col + 0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 3] = word_data[3];
        }
    }

    // DEBUG
    printf("\n--- (mld.lhs.w)(md=%d) - Logical: %dx%d (but is trasposed)---\n", md, M, K);
    
    for (int r = md; r < md + tot_reg ; r++) {
        
        printf("Physical Register m%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x %02x %02x %02x] ", 
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");
}

/*
static inline void lib_MLD8R(Iss *iss, int md, uint64_t rs1, uint64_t rs2) {
    
    int cmul = iss->csr.matrix_cmul.value;
    int N = iss->csr.matrix_n.value;
    int K = iss->csr.matrix_k.value;
    
    int dim = RLEN/32;

    if (md % cmul != 0){
        printf("(mld.rhs.b) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", cmul );
        return;
    }

    uint64_t addr = rs1;
    uint64_t stride = rs2;   

    int n_row = dim; 
    int n_col = dim;
    
    int tiles_per_row = (N + n_col - 1) / n_col; 
    int tiles_per_col = (K + n_row - 1) / n_row; 

    int tot_reg = tiles_per_row * tiles_per_col;

    if (md + tot_reg > ISS_NB_MREGS) {             
        printf("(mld.rhs.b) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int k = 0; k < K; k++) {     
        
        uint64_t current_row_addr = addr + (k * stride);
        
        for(int n = 0; n < N; n++) { 
            
            uint64_t elem_addr = current_row_addr + (n * 4);
            
            int rowT = k / n_row;  
            int colT = n / n_col;  
            int mdT = md + (rowT * tiles_per_row) + colT;

            int row = k % n_row;           
            int col = (n % n_col) * 4;       

            uint8_t word_data[4];
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, elem_addr, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[mdT][row][col + 0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 3] = word_data[3];
        }
    }

    // DEBUG
    printf("\n--- (mld.rhs.b)(md=%d) - Logical: %dx%d ---\n", md, K, N);
    
    for (int r = md; r < md + tot_reg ; r++) {
        
        printf("Physical Register m%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x] [%02x] [%02x] [%02x] ", 
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");
}

static inline void lib_MLD16R(Iss *iss, int md, uint64_t rs1, uint64_t rs2) {
    
    int cmul = iss->csr.matrix_cmul.value;
    int N = iss->csr.matrix_n.value;
    int K = iss->csr.matrix_k.value;
    
    int dim = RLEN/32;

    if (md % cmul != 0){
        printf("(mld.rhs.h) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", cmul );
        return;
    }

    uint64_t addr = rs1;
    uint64_t stride = rs2;   

    int n_row = dim; 
    int n_col = dim;
    
    int tiles_per_row = (N + n_col - 1) / n_col; 
    int tiles_per_col = (K + n_row - 1) / n_row; 

    int tot_reg = tiles_per_row * tiles_per_col;

    if (md + tot_reg > ISS_NB_MREGS) {             
        printf("(mld.rhs.h) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int k = 0; k < K; k++) {     
        
        uint64_t current_row_addr = addr + (k * stride);
        
        for(int n = 0; n < N; n++) { 
            
            uint64_t elem_addr = current_row_addr + (n * 4);
            
            int rowT = k / n_row;  
            int colT = n / n_col;  
            int mdT = md + (rowT * tiles_per_row) + colT;

            int row = k % n_row;           
            int col = (n % n_col) * 4;       

            uint8_t word_data[4];
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, elem_addr, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[mdT][row][col + 0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 3] = word_data[3];
        }
    }

    // DEBUG
    printf("\n--- (mld.rhs.h)(md=%d) - Logical: %dx%d ---\n", md, K, N);
    
    for (int r = md; r < md + tot_reg ; r++) {
        
        printf("Physical Register m%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x %02x] [%02x %02x] ", 
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");
}
*/

static inline void lib_MLD32R(Iss *iss, int md, uint64_t rs1, uint64_t rs2) {
    
    int cmul = iss->csr.matrix_cmul.value;
    int N = iss->csr.matrix_n.value;
    int K = iss->csr.matrix_k.value;
    
    int dim = RLEN/32;

    if (md % cmul != 0){
        printf("(mld.rhs.w) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", cmul );
        return;
    }

    uint64_t addr = rs1;
    uint64_t stride = rs2;   

    int n_row = dim; 
    int n_col = dim;
    
    int tiles_per_row = (N + n_col - 1) / n_col; 
    int tiles_per_col = (K + n_row - 1) / n_row; 

    int tot_reg = tiles_per_row * tiles_per_col;

    if (md + tot_reg > ISS_NB_MREGS) {             
        printf("(mld.rhs.w) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int k = 0; k < K; k++) {     
        
        uint64_t current_row_addr = addr + (k * stride);
        
        for(int n = 0; n < N; n++) { 
            
            uint64_t elem_addr = current_row_addr + (n * 4);
            
            int rowT = k / n_row;  
            int colT = n / n_col;  
            int mdT = md + (rowT * tiles_per_row) + colT;

            int row = k % n_row;           
            int col = (n % n_col) * 4;       

            uint8_t word_data[4];
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, elem_addr, 4, word_data, false);

            iss->quadrilatero.mregfile.mregs[mdT][row][col + 0] = word_data[0];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 1] = word_data[1];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 2] = word_data[2];
            iss->quadrilatero.mregfile.mregs[mdT][row][col + 3] = word_data[3];
        }
    }

    // DEBUG
    printf("\n--- (mld.rhs.w)(md=%d) - Logical: %dx%d ---\n", md, K, N);
    
    for (int r = md; r < md + tot_reg ; r++) {
        
        printf("Physical Register m%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x %02x %02x %02x] ", 
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.mregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");
}


static inline void lib_MST (Iss *iss, int ms, iss_reg_t rs1, iss_reg_t rs2){
    
    uint64_t add = rs1;
    uint64_t stride = rs2;    

    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;
    int N = iss->csr.matrix_n.value;
    int M = iss->csr.matrix_m.value;

    int dim = RLEN/32;

    if (ms % (cmul*rmul) != 0){
        printf("(mstore) HARDWARE TRAP: The selected register is invalid; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    int n_row = dim; 
    int n_col = dim;
    
    int tiles_per_row = (N + n_col - 1) / n_col; 
    int tiles_per_col = (M + n_row - 1) / n_row; 

    int tot_reg = tiles_per_row * tiles_per_col;

    if (ms + tot_reg > ISS_NB_MREGS) {             
        printf("(mstore) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, ms);
        return; 
    }

    for(int m=0; m<M; m++){
        for(int n=0; n<N; n++){

            int rowT = m / n_row;  
            int colT = n / n_col;  
            int msT = ms + (rowT * tiles_per_row) + colT;

            int row = m % n_row;           
            int col = (n % n_col) * 4;       

            uint8_t word_data[4];
            word_data[0] = iss->quadrilatero.mregfile.mregs[msT][row][col+0];
            word_data[1] = iss->quadrilatero.mregfile.mregs[msT][row][col+1];
            word_data[2] = iss->quadrilatero.mregfile.mregs[msT][row][col+2];
            word_data[3] = iss->quadrilatero.mregfile.mregs[msT][row][col+3];
            
            iss->quadrilatero.mlsu.Mlsu_io_access(iss, add, 4, word_data, true);

            add += 4;
        }
        add += (stride-16);
    }
    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            MATRIX ALU
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static inline void lib_MMADAH(Iss *iss, int md, int ms1, int ms2) {
    
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;
    int N = iss->csr.matrix_n.value;
    int M = iss->csr.matrix_m.value;
    int K = iss->csr.matrix_k.value;
    
    if (md % (cmul*rmul) != 0){
        printf("(mma.dah) HARDWARE TRAP: The selected register is invalid for result matrix; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    if (ms1 % rmul != 0){
        printf("(mma.dah) HARDWARE TRAP: The selected register is invalid for left matrix; it must be a multiple of %d\n", cmul );
        return;
    }

    if (ms2 % cmul != 0){
        printf("(mma.dah) HARDWARE TRAP: The selected register is invalid for right matrix; it must be a multiple of %d\n", cmul );
        return;
    }

    int dim = RLEN/32;

    int tiles_per_row_A = (K + dim - 1) / dim; 
    int tiles_per_col_A = (M + dim - 1) / dim; 

    int tiles_per_row_B = (K + dim - 1) / dim; 
    int tiles_per_col_B = (N + dim - 1) / dim;

    int tiles_per_row_C = (M + dim - 1) / dim; 
    int tiles_per_col_C = (N + dim - 1) / dim;

    
    int tot_reg = tiles_per_row_A * tiles_per_col_A;

    if (ms1 + tot_reg > ISS_NB_MREGS) {             
        printf("(mma.dah) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, ms1);
        return; 
    }

    tot_reg = tiles_per_row_B * tiles_per_col_B;

    if (ms2 + tot_reg > ISS_NB_MREGS) {             
        printf("(mma.dah) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, ms2);
        return; 
    }

    tot_reg = tiles_per_row_C * tiles_per_col_C;

    if (md + tot_reg > ISS_NB_ACCREGS) {             
        printf("(mma.dah) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int m=0; m<M; m++){
        for(int n=0; n<N; n++){

            int rowC = m / 4;  
            int colC = n / 4;  
            int mdC = md + (rowC * tiles_per_row_C) + colC;
             
            int row = m % 4;
            int col = n % 4;
        
            int64_t accumulator = 0;

            for(int k = 0; k < K; k+=2){

                

                int mA = ms1 + ( (k/4) * tiles_per_row_A) + rowC;
                int mB = ms2 + ( (k/4) * tiles_per_row_B) + colC; 

                int16_t a0 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[mA][k][(row*2)+0] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[mA][k][(row*2)+1] << 8) );
                                           
                int16_t a1 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[mA][k+1][(row*2)+0] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[mA][k+1][(row*2)+1] << 8) );

                int16_t b0 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[mB][k][(col*2)+0] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[mB][k][(col*2)+1] << 8) );
                                           
                int16_t b1 = (int16_t)( ((uint8_t)iss->quadrilatero.mregfile.mregs[mB][k+1][(col*2)+0] << 0) |
                                        ((uint8_t)iss->quadrilatero.mregfile.mregs[mB][k+1][(col*2)+1] << 8) );

                accumulator += (int64_t)a0 * b0;
                accumulator += (int64_t)a1 * b1;

                printf("%d * %d + %d * %d + ",a0, b0, a1, b1);

            }

            printf("\n");

            int32_t prev_md = (int32_t)((iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+0] << 0)  |
                                        (iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+1] << 8)  |
                                        (iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+2] << 16) |
                                        (iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+3] << 24));

            int64_t final_val = accumulator + prev_md;

            if (final_val > INT32_MAX) final_val = INT32_MAX;
            if (final_val < INT32_MIN) final_val = INT32_MIN;

            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+0] = (uint8_t)(final_val & 0xFF);
            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+1] = (uint8_t)((final_val >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+2] = (uint8_t)((final_val >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+3] = (uint8_t)((final_val >> 24) & 0xFF);


        }
    }

    printf("\n--- (mma.dah)(md=%d) - Logical: %dx%d ---\n", md, M, N);
    
    for (int r = md; r < md + tot_reg ; r++) {
        
        printf("Physical Register acc%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x %02x %02x %02x] ", 
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");


}
*/

static inline void lib_MMASAW(Iss *iss, int md, int ms1, int ms2) {
    
    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;
    int N = iss->csr.matrix_n.value;
    int M = iss->csr.matrix_m.value;
    int K = iss->csr.matrix_k.value;
    
    if (md % (cmul*rmul) != 0){
        printf("(mma.saw) HARDWARE TRAP: The selected register is invalid for result matrix; it must be a multiple of %d\n", rmul * cmul);
        return;
    }

    if (ms1 % rmul != 0){
        printf("(mma.saw) HARDWARE TRAP: The selected register is invalid for left matrix; it must be a multiple of %d\n", cmul );
        return;
    }

    if (ms2 % cmul != 0){
        printf("(mma.saw) HARDWARE TRAP: The selected register is invalid for right matrix; it must be a multiple of %d\n", cmul );
        return;
    }

    int dim = RLEN/32;

    int tiles_per_row_A = (K + dim - 1) / dim; 
    int tiles_per_col_A = (M + dim - 1) / dim; 

    int tiles_per_row_B = (K + dim - 1) / dim; 
    int tiles_per_col_B = (N + dim - 1) / dim;

    int tiles_per_row_C = (N + dim - 1) / dim; 
    int tiles_per_col_C = (M + dim - 1) / dim;

    
    int tot_reg = tiles_per_row_A * tiles_per_col_A;

    if (ms1 + tot_reg > ISS_NB_MREGS) {             
        printf("(mma.saw) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, ms1);
        return; 
    }

    tot_reg = tiles_per_row_B * tiles_per_col_B;

    if (ms2 + tot_reg > ISS_NB_MREGS) {             
        printf("(mma.saw) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, ms2);
        return; 
    }

    tot_reg = tiles_per_row_C * tiles_per_col_C;

    if (md + tot_reg > ISS_NB_ACCREGS) {             
        printf("(mma.saw) HARDWARE TRAP: Too many registers required (needs %d, max from md %d)\n", tot_reg, md);
        return; 
    }

    for(int m=0; m<M; m++){
        for(int n=0; n<N; n++){

            int rowC = m / 4;  
            int colC = n / 4;  
            int mdC = md + (rowC * tiles_per_row_C) + colC;
             
            int row = m % 4;
            int col = n % 4;
        
            int64_t accumulator = 0;

            for(int k = 0; k < K; k++){
                
                int colA = k / 4;

                int mA = ms1 + ( rowC * tiles_per_row_A) + colA;
                int mB = ms2 + ( colC * tiles_per_row_B) + colA; 

                int32_t a0 = (int32_t)(  (iss->quadrilatero.mregfile.mregs[mA][k][(row*4)+0] << 0)  |
                                         (iss->quadrilatero.mregfile.mregs[mA][k][(row*4)+1] << 8)  |
                                         (iss->quadrilatero.mregfile.mregs[mA][k][(row*4)+2] << 16) |
                                         (iss->quadrilatero.mregfile.mregs[mA][k][(row*4)+3] << 24));

                int32_t b0 = (int32_t)(  (iss->quadrilatero.mregfile.mregs[mB][k][(col*4)+0] << 0)  |
                                         (iss->quadrilatero.mregfile.mregs[mB][k][(col*4)+1] << 8)  |
                                         (iss->quadrilatero.mregfile.mregs[mB][k][(col*4)+2] << 16) |
                                         (iss->quadrilatero.mregfile.mregs[mB][k][(col*4)+3] << 24));
                
                accumulator += (int64_t)a0 * b0;


            }

            int32_t prev_md = (int32_t)((iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+0] << 0)  |
                                        (iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+1] << 8)  |
                                        (iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+2] << 16) |
                                        (iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+3] << 24));

            int64_t final_val = accumulator + prev_md;

            if (final_val > INT32_MAX) final_val = INT32_MAX;
            if (final_val < INT32_MIN) final_val = INT32_MIN;

            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+0] = (uint8_t)(final_val & 0xFF);
            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+1] = (uint8_t)((final_val >> 8) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+2] = (uint8_t)((final_val >> 16) & 0xFF);
            iss->quadrilatero.mregfile.maccregs[mdC][row][col*4+3] = (uint8_t)((final_val >> 24) & 0xFF);


        }
    }

    printf("\n--- (mma.saw)(md=%d) - Logical: %dx%d ---\n", md, M, N);
    
    for (int r = md; r < 4 ; r++) {
        
        printf("Physical Register acc%d:\n", r);

        for(int i = 0; i < dim; i++) { 
            printf("  Row %d: ", i);
            
            for(int w = 0; w < dim; w++) { 
                printf("[%02x %02x %02x %02x] ", 
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+0],
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+1],
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+2],
                    iss->quadrilatero.mregfile.maccregs[r][i][w*4+3]);
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");


}


/*
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


*/