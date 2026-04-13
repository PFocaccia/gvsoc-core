#ifndef QUADRILATERO_HPP
#define QUADRILATERO_HPP

#include "cpu/iss/include/types.hpp"
#include <deque>
#include <vp/vp.hpp>

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

#ifndef RLEN
#define RLEN 128
#endif

#define MAT_DIM_ROWS (RLEN / 32)
#define MAT_DIM_COLS (RLEN / 32)

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

    int64_t enqueue_load(uint64_t addr, int size, uint8_t *dst);
    int64_t enqueue_store(uint64_t addr, int size, const uint8_t *src);

    vp::IoMaster io_itf[4];
    vp::IoReq io_reqs[4];
    vp::ClockEvent *event;
    int io_retval;
    uint64_t io_pending_addr;
    int io_pending_size;
    uint8_t *io_pending_data;
    bool io_pending_is_write;
    bool waiting_io_response;

private:
    enum PendingType
    {
        PENDING_NONE = -1,
        PENDING_LOAD = 0,
        PENDING_STORE = 1,
    };

    struct PendingReq
    {
        bool is_write;
        bool committed;
        uint64_t addr;
        int size;
        uint8_t *dst;
        uint8_t data[16];
        int64_t done_cycle;
    };

    int do_io_access(uint64_t addr, int size, uint8_t *data, bool is_write);
    void process_queue(int64_t now);
    void schedule_event(int64_t now);
    static void queue_event_handler(vp::Block *__this, vp::ClockEvent *event);

    std::deque<PendingReq> pending;
    int64_t slot_free[2];
    PendingType active_type;

    Iss &iss;
};

class Quadrilatero
{
public:
    Quadrilatero(Iss &iss);

    void build();
    void reset(bool active);
    void isa_init();

    static iss_reg_t insn_stub_handler(Iss *iss, iss_insn_t *insn, iss_reg_t pc);

    enum MatType
    {
        MAT_TYPE_MEM = 0,
        MAT_TYPE_CFG,
        MAT_TYPE_MAC,
        MAT_TYPE_MOV,
        MAT_TYPE_MZERO_A,
        MAT_TYPE_MZERO_M,
        MAT_TYPE_COUNT,
    };

    bool mreg_block_ready(int base, bool is_acc, int64_t now, int block_size) const;
    void mreg_block_set_ready(int base, bool is_acc, int64_t ready_cycle, int block_size);
    int latency_for_insn(const char *label) const;

    uint32_t mtype;         
    uint32_t active_rows = MAT_DIM_ROWS;  
    uint32_t active_cols = MAT_DIM_COLS;

    MRegfile mregfile;
    Mlsu mlsu;

    int64_t mreg_ready[ISS_NB_MREGS];
    int64_t acc_ready[ISS_NB_ACCREGS];

private:
    struct XifEntry
    {
        MatType type;
        int64_t ctrl_cycle;
        int64_t scoreboard_cycle;
        int latency;
        int md;
        int accd;
        int md_block;
        int accd_block;
        PendingInsn *pending_insn;
    };

    struct TypeEntry
    {
        int64_t done_cycle;
        PendingInsn *pending_insn;
    };

    void process_queue(int64_t now);
    void schedule_next_event(int64_t now);
    static void queue_event_handler(vp::Block *__this, vp::ClockEvent *event);

    XifEntry xif_entries[4];
    int xif_head;
    int xif_tail;
    int xif_count;

    TypeEntry type_entries[MAT_TYPE_COUNT][2];
    int type_head[MAT_TYPE_COUNT];
    int type_tail[MAT_TYPE_COUNT];
    int type_count[MAT_TYPE_COUNT];
    int64_t type_available[MAT_TYPE_COUNT];

    vp::ClockEvent *queue_event;

    Iss &iss;

};

#endif 
