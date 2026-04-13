#include "cpu/iss/include/iss.hpp"
#include <string.h>

// /////////////////////////////////////////////////////////////////////////////
// //                           QUADRILATERO (Matrix Unit)
// /////////////////////////////////////////////////////////////////////////////

Quadrilatero::Quadrilatero(Iss &iss)
    : mregfile(iss), mlsu(iss), iss(iss)
{
    this->xif_head = 0;
    this->xif_tail = 0;
    this->xif_count = 0;
    this->queue_event = NULL;

    for (int i = 0; i < MAT_TYPE_COUNT; i++)
    {
        this->type_head[i] = 0;
        this->type_tail[i] = 0;
        this->type_count[i] = 0;
        this->type_available[i] = 0;
    }

    for (int i = 0; i < ISS_NB_MREGS; i++)  this->mreg_ready[i] = 0;

    for (int i = 0; i < ISS_NB_ACCREGS; i++) this->acc_ready[i] = 0;
}

void Quadrilatero::build()
{
    this->mlsu.build();

    if (this->queue_event == NULL) this->queue_event = this->iss.top.event_new((vp::Block *)this, &Quadrilatero::queue_event_handler);
    
}

void Quadrilatero::reset(bool active)
{
    if (active) {
        this->mregfile.reset(true);
        this->mlsu.reset(true);

        this->xif_head = 0;
        this->xif_tail = 0;
        this->xif_count = 0;

        for (int i = 0; i < MAT_TYPE_COUNT; i++){

            this->type_head[i] = 0;
            this->type_tail[i] = 0;
            this->type_count[i] = 0;
            this->type_available[i] = 0;
        }

        for (int i = 0; i < ISS_NB_MREGS; i++)  this->mreg_ready[i] = 0;
        
        for (int i = 0; i < ISS_NB_ACCREGS; i++) this->acc_ready[i] = 0;
        
    }
}

void Quadrilatero::isa_init()
{
    const char *matrix_insns[] = {
        "mld.lhs",
        "mld.rhs",
        "mst",
        "mmacc",
        "mzero.m",
        "mzero.a",
        "mmov.am",
        "mmov.aa",
        "mmov.ma",
        "mmov.mm",
        "mcfgk",
        "mcfgm",
        "mcfgn",
        "mmac.dt",
    };

    for (unsigned int i = 0; i < (sizeof(matrix_insns) / sizeof(matrix_insns[0])); i++)
    {
        iss_decoder_item_t *insn = this->iss.decode.get_insn(matrix_insns[i]);
        if (insn != NULL)
        {
            insn->u.insn.stub_handler = &Quadrilatero::insn_stub_handler;
        }
    }
}

bool Quadrilatero::mreg_block_ready(int base, bool is_acc, int64_t now, int block_size) const{

    if (base < 0) return true;

    if (is_acc){
        for (int i = 0; i < block_size; i++){
            int reg = base + i;
            if (reg >= ISS_NB_ACCREGS) break;
            if (this->acc_ready[reg] > now) return false;
        }
    } else {
        for (int i = 0; i < block_size; i++){
            int reg = base + i;
            if (reg >= ISS_NB_MREGS) break;
            if (this->mreg_ready[reg] > now) return false;
        }
    }

    return true;
}

void Quadrilatero::mreg_block_set_ready(int base, bool is_acc, int64_t ready_cycle, int block_size){

    if (base < 0) return;

    if (is_acc){
        for (int i = 0; i < block_size; i++){
            int reg = base + i;
            if (reg >= ISS_NB_ACCREGS) break;
            if (this->acc_ready[reg] < ready_cycle) this->acc_ready[reg] = ready_cycle;
        }
    } else {
        for (int i = 0; i < block_size; i++){
            int reg = base + i;
            if (reg >= ISS_NB_MREGS) break;
            if (this->mreg_ready[reg] < ready_cycle) this->mreg_ready[reg] = ready_cycle;
        }
    }

}

int Quadrilatero::latency_for_insn(const char *label) const
{
    if (label == NULL) return 1;

    int rmul = this->iss.csr.matrix_rmul.value;
    int cmul = this->iss.csr.matrix_cmul.value;
    
    if (rmul < 1) rmul = 1;
    if (cmul < 1) cmul = 1;
    

    if (!strcmp(label, "mld.lhs")) return 6 * rmul;

    if (!strcmp(label, "mld.rhs")) return 6 * cmul;
    
    if (!strcmp(label, "mst")) return 4 * cmul * rmul;
    
    if (!strcmp(label, "mmacc")) return 8;
    

    if (!strcmp(label, "mzero.m") || !strcmp(label, "mzero.a")) return 2;
    

    if (!strcmp(label, "mmov.am") || !strcmp(label, "mmov.aa") ||
        !strcmp(label, "mmov.ma") || !strcmp(label, "mmov.mm")){
        
            return 2;
    }

    if (!strcmp(label, "mcfgk") || !strcmp(label, "mcfgm") ||
        !strcmp(label, "mcfgn") || !strcmp(label, "mmac.dt")){

        return 1;
    }

    return 1;
}

static inline bool quad_label_is(const char *label, const char *name)
{
    return label != NULL && !strcmp(label, name);
}

static inline Quadrilatero::MatType quad_insn_type(const char *label)
{
    if (quad_label_is(label, "mld.lhs") || quad_label_is(label, "mld.rhs") || quad_label_is(label, "mst"))
    {
        return Quadrilatero::MAT_TYPE_MEM;
    }

    if (quad_label_is(label, "mcfgk") || quad_label_is(label, "mcfgm") ||
        quad_label_is(label, "mcfgn") || quad_label_is(label, "mmac.dt"))
    {
        return Quadrilatero::MAT_TYPE_CFG;
    }

    if (quad_label_is(label, "mmacc"))
    {
        return Quadrilatero::MAT_TYPE_MAC;
    }

    if (quad_label_is(label, "mmov.am") || quad_label_is(label, "mmov.aa") ||
        quad_label_is(label, "mmov.ma") || quad_label_is(label, "mmov.mm"))
    {
        return Quadrilatero::MAT_TYPE_MOV;
    }

    if (quad_label_is(label, "mzero.a"))
    {
        return Quadrilatero::MAT_TYPE_MZERO_A;
    }

    return Quadrilatero::MAT_TYPE_MZERO_M;
}

iss_reg_t Quadrilatero::insn_stub_handler(Iss *iss, iss_insn_t *insn, iss_reg_t pc)
{
    Quadrilatero &quad = iss->quadrilatero;
    const char *label = insn->decoder_item->u.insn.label;
    int64_t now = iss->top.clock.get_cycles();

    quad.process_queue(now);

    if (quad.xif_count >= 4) return pc; 

    int rmul = iss->csr.matrix_rmul.value;
    int cmul = iss->csr.matrix_cmul.value;
    
    int block_lhs = rmul < 1 ? 1 : rmul;
    int block_rhs = cmul < 1 ? 1 : cmul;
    int block_acc = block_lhs * block_rhs;

    int block_default = block_acc;

    int md = -1;
    int ms1 = -1;
    int ms2 = -1;
    int accd = -1;
    int accs = -1;

    int block_md = block_default;
    int block_ms1 = block_default;
    int block_ms2 = block_default;
    int block_accd = block_acc;
    int block_accs = block_acc;

    if (quad_label_is(label, "mld.lhs") || quad_label_is(label, "mld.rhs")){

        md = insn->in_regs[0];

        int rs1 = insn->in_regs[1];
        int rs2 = insn->in_regs[2];
        if (iss->regfile.scoreboard_reg_timestamp[rs1] == -1 ||
            iss->regfile.scoreboard_reg_timestamp[rs2] == -1)
        {
            return pc;
        }

        if (quad_label_is(label, "mld.lhs")) block_md = block_lhs;
        else block_md = block_rhs;
        
    } else if (quad_label_is(label, "mst")) {
        
        ms1 = insn->in_regs[0];

        int rs1 = insn->in_regs[1];
        int rs2 = insn->in_regs[2];
        if (iss->regfile.scoreboard_reg_timestamp[rs1] == -1 ||
            iss->regfile.scoreboard_reg_timestamp[rs2] == -1)
        {
            return pc;
        }
    
    } else if (quad_label_is(label, "mmacc")) {
        ms1 = insn->in_regs[0];
        ms2 = insn->in_regs[1];
        accd = insn->in_regs[2];
        accs = insn->in_regs[2];
        int dim = RLEN/32;
        int M = iss->csr.matrix_m.value;
        int N = iss->csr.matrix_n.value;
        int K = iss->csr.matrix_k.value;

        int tiles_per_row_A = (K + dim - 1) / dim;
        int tiles_per_col_A = (M + dim - 1) / dim;
        int tiles_per_row_B = (K + dim - 1) / dim;
        int tiles_per_col_B = (N + dim - 1) / dim;
        int tiles_per_row_C = (N + dim - 1) / dim;
        int tiles_per_col_C = (M + dim - 1) / dim;

        int block_a = tiles_per_row_A * tiles_per_col_A;
        int block_b = tiles_per_row_B * tiles_per_col_B;
        int block_ab = block_a > block_b ? block_a : block_b;
        block_ms1 = block_ab;
        block_ms2 = block_ab;
        block_accd = tiles_per_row_C * tiles_per_col_C;
        block_accs = block_accd;
    } else if (quad_label_is(label, "mzero.m")) {
        
        md = insn->in_regs[0];
    
    } else if (quad_label_is(label, "mzero.a")) {
        
        accd = insn->in_regs[0];
    
    } else if (quad_label_is(label, "mmov.am")) {

        md = insn->in_regs[0];
        accs = insn->in_regs[1];

    } else if (quad_label_is(label, "mmov.aa")) {
        
        accd = insn->in_regs[0];
        accs = insn->in_regs[1];
    
    } else if (quad_label_is(label, "mmov.ma")) {
        
        ms1 = insn->in_regs[0];
        accd = insn->in_regs[1];
    
    } else if (quad_label_is(label, "mmov.mm")) {
        
        md = insn->in_regs[0];
        ms1 = insn->in_regs[1];
    
    }

    if (quad_label_is(label, "mcfgk") || quad_label_is(label, "mcfgm") ||
        quad_label_is(label, "mcfgn") || quad_label_is(label, "mmac.dt"))
    {
        int rs1 = insn->in_regs[0];
        if (iss->regfile.scoreboard_reg_timestamp[rs1] == -1)
        {
            return pc;
        }

        int rd = insn->in_regs[1];
        if (rd != 0)
        {
            iss->regfile.scoreboard_reg_timestamp[rd] = -1;
        }
    }

    if (!quad.mreg_block_ready(ms1, false, now, block_ms1)) return pc;
    
    if (!quad.mreg_block_ready(ms2, false, now, block_ms2)) return pc;
    
    if (!quad.mreg_block_ready(md, false, now, block_md)) return pc;
    
    if (!quad.mreg_block_ready(accs, true, now, block_accs)) return pc;
    
    if (!quad.mreg_block_ready(accd, true, now, block_accd)) return pc;
    

    int latency = quad.latency_for_insn(label);
    Quadrilatero::MatType type = quad_insn_type(label);
    if (quad.type_count[type] >= 2)
    {
        return pc;
    }

    int64_t xif_out_cycle = now + 1;
    int64_t ctrl_cycle = xif_out_cycle + 1;
    int64_t scoreboard_cycle = xif_out_cycle + 2;

    if (md >= 0)
    {
        quad.mreg_block_set_ready(md, false, scoreboard_cycle + latency, block_md);
    }
    if (accd >= 0)
    {
        quad.mreg_block_set_ready(accd, true, scoreboard_cycle + latency, block_accd);
    }

    PendingInsn &pending_insn = iss->top.pending_insn_enqueue(insn, pc);

    iss_reg_t next_pc = insn->stub_handler(iss, insn, pc);

    quad.xif_entries[quad.xif_tail] = { type, ctrl_cycle, scoreboard_cycle, latency, md, accd, block_md, block_accd, &pending_insn };
    quad.xif_tail = (quad.xif_tail + 1) % 4;
    quad.xif_count++;

    quad.schedule_next_event(now);

#ifdef CONFIG_GVSOC_ISS_SCOREBOARD
    if (quad_label_is(label, "mcfgk") || quad_label_is(label, "mcfgm") || quad_label_is(label, "mcfgn"))
    {
        int rd = insn->in_regs[1];
        if (rd != 0)
        {
            iss->regfile.scoreboard_reg_set_timestamp(rd, latency, -1);
        }
    }
#endif

    return next_pc;
}

void Quadrilatero::process_queue(int64_t now)
{
    for (int t = 0; t < MAT_TYPE_COUNT; t++)
    {
        while (this->type_count[t] > 0)
        {
            TypeEntry &entry = this->type_entries[t][this->type_head[t]];
            if (entry.done_cycle > now)
            {
                break;
            }
            if (entry.pending_insn != NULL)
            {
                this->iss.top.insn_commit(entry.pending_insn);
            }
            this->type_head[t] = (this->type_head[t] + 1) % 2;
            this->type_count[t]--;
        }
    }

    while (this->xif_count > 0)
    {
        XifEntry &entry = this->xif_entries[this->xif_head];
        if (entry.ctrl_cycle > now)
        {
            break;
        }

        if (this->type_count[entry.type] >= 2)
        {
            break;
        }

        int64_t start_cycle = entry.ctrl_cycle;
        if (this->type_available[entry.type] > start_cycle)
        {
            start_cycle = this->type_available[entry.type];
        }
        int64_t done_cycle = start_cycle + entry.latency;
        if (entry.scoreboard_cycle > done_cycle)
        {
            done_cycle = entry.scoreboard_cycle;
        }

        if (entry.md >= 0)
        {
            this->mreg_block_set_ready(entry.md, false, done_cycle, entry.md_block);
        }
        if (entry.accd >= 0)
        {
            this->mreg_block_set_ready(entry.accd, true, done_cycle, entry.accd_block);
        }

        this->type_entries[entry.type][this->type_tail[entry.type]] = { done_cycle, entry.pending_insn };
        this->type_tail[entry.type] = (this->type_tail[entry.type] + 1) % 2;
        this->type_count[entry.type]++;
        this->type_available[entry.type] = done_cycle;

        this->xif_head = (this->xif_head + 1) % 4;
        this->xif_count--;
    }

    this->schedule_next_event(now);
}

void Quadrilatero::schedule_next_event(int64_t now)
{
    if (this->queue_event == NULL)
    {
        return;
    }

    int64_t next_cycle = -1;

    if (this->xif_count > 0)
    {
        int64_t ctrl_cycle = this->xif_entries[this->xif_head].ctrl_cycle;
        next_cycle = ctrl_cycle;
    }

    for (int t = 0; t < MAT_TYPE_COUNT; t++)
    {
        if (this->type_count[t] > 0)
        {
            int64_t done_cycle = this->type_entries[t][this->type_head[t]].done_cycle;
            if (next_cycle == -1 || done_cycle < next_cycle)
            {
                next_cycle = done_cycle;
            }
        }
    }

    if (next_cycle == -1)
    {
        return;
    }

    int64_t delay = next_cycle - now;
    if (delay < 1)
    {
        delay = 1;
    }

    if (!this->queue_event->is_enqueued())
    {
        this->queue_event->enqueue(delay);
    }
}

void Quadrilatero::queue_event_handler(vp::Block *__this, vp::ClockEvent *event)
{
    Quadrilatero *_this = (Quadrilatero *)__this;
    int64_t now = _this->iss.top.clock.get_cycles();
    _this->process_queue(now);
}

// /////////////////////////////////////////////////////////////////////////////
// //                           MATRIX REGISTER FILE
// /////////////////////////////////////////////////////////////////////////////

MRegfile::MRegfile(Iss &iss) : iss(iss) {
    this->reset(true);
}

inline void MRegfile::reset(bool active){
    if (active){
        for (int i = 0; i < ISS_NB_MREGS; i++){
            for (int j = 0; j < MAT_DIM_ROWS; j++){
                for (int k = 0; k < MAT_DIM_COLS * 4; k++){
                    this->mregs[i][j][k] = 0;
                }
            }
        }

        for (int i = 0; i < ISS_NB_ACCREGS; i++){
            for (int j = 0; j < MAT_DIM_ROWS; j++){
                for (int k = 0; k < MAT_DIM_COLS * 4; k++){
                    this->maccregs[i][j][k] = 0;
                }
            }
        }
    }
}

// /////////////////////////////////////////////////////////////////////////////
// //                           MATRIX LSU (Load/Store Unit)
// /////////////////////////////////////////////////////////////////////////////

Mlsu::Mlsu(Iss &iss) : iss(iss)
{
    this->event = NULL;
    this->slot_free[0] = 0;
    this->slot_free[1] = 0;
    this->active_type = PENDING_NONE;
}

void Mlsu::reset(bool active)
{
    if (active)
    {
        this->io_retval = 0;
        this->io_pending_addr = 0;
        this->io_pending_size = 0;
        this->io_pending_data = nullptr;
        this->io_pending_is_write = false;
        this->waiting_io_response = false;

        this->pending.clear();
        this->slot_free[0] = 0;
        this->slot_free[1] = 0;
        this->active_type = PENDING_NONE;
    }
}

void Mlsu::data_response(vp::Block *__this, vp::IoReq *req)
{   
    printf("Dentro data response\n");
    Mlsu *_this = (Mlsu *)__this;
    _this->handle_pending_io_access(&_this->iss);
}

void Mlsu::build()
{
    for (int i=0; i<4; i++) 
    {
        this->io_itf[i].set_resp_meth(&Mlsu::data_response);
        this->iss.top.new_master_port("mlsu_" + std::to_string(i), &this->io_itf[i], (vp::Block *)this);
    }

    if (this->event == NULL)
    {
        this->event = this->iss.top.event_new((vp::Block *)this, &Mlsu::queue_event_handler);
    }
}

int64_t Mlsu::enqueue_load(uint64_t addr, int size, uint8_t *dst)
{
    if (size <= 0 || size > 16 || dst == NULL)
    {
        return -1;
    }

    int64_t now = this->iss.top.clock.get_cycles();
    int64_t start = now;

    if (this->active_type != PENDING_LOAD)
    {
        int64_t drain = this->slot_free[0] > this->slot_free[1] ? this->slot_free[0] : this->slot_free[1];
        if (drain > start)
        {
            start = drain;
        }
        this->slot_free[0] = drain;
        this->slot_free[1] = drain;
        this->active_type = PENDING_LOAD;
    }

    int slot = this->slot_free[0] <= this->slot_free[1] ? 0 : 1;
    if (this->slot_free[slot] > start)
    {
        start = this->slot_free[slot];
    }

    int64_t done_cycle = start + 6;
    this->slot_free[slot] = done_cycle;

    (void)this->do_io_access(addr, size, dst, false);

    PendingReq req = { false, true, addr, size, dst, {0}, done_cycle };
    this->pending.push_back(req);
    this->schedule_event(now);

    return done_cycle;
}

int64_t Mlsu::enqueue_store(uint64_t addr, int size, const uint8_t *src)
{
    if (size <= 0 || size > 16 || src == NULL)
    {
        return -1;
    }

    int64_t now = this->iss.top.clock.get_cycles();
    int64_t start = now;

    if (this->active_type != PENDING_STORE)
    {
        int64_t drain = this->slot_free[0] > this->slot_free[1] ? this->slot_free[0] : this->slot_free[1];
        if (drain > start)
        {
            start = drain;
        }
        this->slot_free[0] = drain;
        this->slot_free[1] = drain;
        this->active_type = PENDING_STORE;
    }

    int slot = this->slot_free[0] <= this->slot_free[1] ? 0 : 1;
    if (this->slot_free[slot] > start)
    {
        start = this->slot_free[slot];
    }

    int64_t done_cycle = start + 4;
    this->slot_free[slot] = done_cycle;

    (void)this->do_io_access(addr, size, (uint8_t *)src, true);

    PendingReq req = { true, true, addr, size, NULL, {0}, done_cycle };
    this->pending.push_back(req);
    this->schedule_event(now);

    return done_cycle;
}

int Mlsu::do_io_access(uint64_t addr, int size, uint8_t *data, bool is_write)
{
    uint64_t cur_addr = addr;
    int remaining = size;
    uint8_t *cur_data = data;

    while (remaining > 0)
    {
        uint32_t addr_aligned = cur_addr & ~(4 - 1);
        int chunk = addr_aligned + 4 - cur_addr;
        if (chunk > remaining)
        {
            chunk = remaining;
        }

        int port = (cur_addr / 4) % 4;
        vp::IoReq *req = &this->io_reqs[port];

        req->init();
        req->set_addr(cur_addr);
        req->set_size(chunk);
        req->set_is_write(is_write);
        req->set_data(cur_data);

        if (!this->io_itf[port].is_bound())
        {
            printf("[ERROR] FATAL: The io_itf[%d] port is NOT bound (unconnected)!\n", port);
            return 1;
        }

        int err = this->io_itf[port].req(req);
        if (err == vp::IO_REQ_INVALID)
        {
            return 1;
        }

        cur_data += chunk;
        remaining -= chunk;
        cur_addr += chunk;
    }

    return 0;
}

void Mlsu::process_queue(int64_t now)
{
    while (!this->pending.empty())
    {
        PendingReq &req = this->pending.front();
        if (req.done_cycle > now)
        {
            break;
        }

        this->pending.pop_front();
    }

    this->schedule_event(now);
}

void Mlsu::schedule_event(int64_t now)
{
    if (this->event == NULL || this->pending.empty())
    {
        return;
    }

    int64_t next_cycle = this->pending.front().done_cycle;
    int64_t delay = next_cycle - now;
    if (delay < 1)
    {
        delay = 1;
    }

    if (!this->event->is_enqueued())
    {
        this->event->enqueue(delay);
    }
}

void Mlsu::queue_event_handler(vp::Block *__this, vp::ClockEvent *event)
{
    Mlsu *_this = (Mlsu *)__this;
    int64_t now = _this->iss.top.clock.get_cycles();
    _this->process_queue(now);
}

/*
Iss::Iss(IssWrapper &top)
    : prefetcher(*this), exec(top, *this), insn_cache(*this), decode(*this), timing(*this), core(*this), irq(*this),
      gdbserver(*this), lsu(top, *this), dbgunit(*this), syscalls(top, *this), trace(*this), csr(*this),
      regfile(top, *this), exception(*this), quadrilatero(*this), memcheck(top, *this), top(top)
{
}
*/