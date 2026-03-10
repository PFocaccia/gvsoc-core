#ifndef QUADRILATERO_HPP
#define QUADRILATERO_HPP

#include "cpu/iss/include/types.hpp"

#define LIB_CALL3(name, s0, s1, s2) name(iss, s0, s1, s2)
#define LIB_CALL4(name, s0, s1, s2, s3) name(iss, s0, s1, s2, s3)
#define LIB_CALL5(name, s0, s1, s2, s3, s4) name(iss, s0, s1, s2, s3, s4)
#define LIB_CALL6(name, s0, s1, s2, s3, s4, s5) name(iss, s0, s1, s2, s3, s4, s5)
#define LIB_CALL7(name, s0, s1, s2, s3, s4, s5, s6) name(iss, s0, s1, s2, s3, s4, s5, s6)
#define LIB_CALL8(name, s0, s1, s2, s3, s4, s5, s6, s7) name(iss, s0, s1, s2, s3, s4, s5, s6, s7)

#define REG_IN(reg) (insn->in_regs[reg])
#define REG_OUT(reg) (insn->out_regs[reg])

#define ISS_NB_MREGS 16
#define ISS_NB_ACCREGS 4      
#define RLEN 128
#define MAT_DIM_ROWS 4      
#define MAT_DIM_COLS 4      

class MRegfile
{
public:

    MRegfile(Iss &iss);

    inline void reset(bool active);

    uint8_t mregs[ISS_NB_MREGS][MAT_DIM_ROWS][MAT_DIM_COLS*4];
    uint8_t maccregs[ISS_NB_ACCREGS][MAT_DIM_ROWS][MAT_DIM_COLS*4];

private:
    Iss &iss;
};

class Mlsu 
{
public:
    Mlsu(Iss &iss);

    void build();
    void reset(bool active);
    
    static void data_response(vp::Block *__this, vp::IoReq *req);

    int Mlsu_io_access(Iss *iss, uint64_t addr, int size, uint8_t *data, bool is_write);
    void handle_pending_io_access(Iss *iss);

    vp::IoMaster io_itf[4];
    vp::IoReq io_req;
    vp::ClockEvent *event;
    int io_retval;
    uint64_t io_pending_addr;
    int io_pending_size;
    uint8_t *io_pending_data;
    bool io_pending_is_write;
    bool waiting_io_response;

private:
    Iss &iss;
};

class Quadrilatero
{
public:
    Quadrilatero(Iss &iss);

    void build();
    void reset(bool active);

    uint32_t mtype;         
    uint32_t active_rows = MAT_DIM_ROWS;  
    uint32_t active_cols = MAT_DIM_COLS;

    MRegfile mregfile;
    Mlsu mlsu;

};

#endif 
