#include "cpu/iss/include/iss.hpp"

// /////////////////////////////////////////////////////////////////////////////
// //                           QUADRILATERO (Matrix Unit)
// /////////////////////////////////////////////////////////////////////////////

Quadrilatero::Quadrilatero(Iss &iss)
    : mregfile(iss), mlsu(iss)
{
}

void Quadrilatero::build()
{
    this->mlsu.build();
}

void Quadrilatero::reset(bool active)
{
    if (active) {
        this->mregfile.reset(true);
        this->mlsu.reset(true);
    }
}

// /////////////////////////////////////////////////////////////////////////////
// //                           MATRIX REGISTER FILE
// /////////////////////////////////////////////////////////////////////////////

MRegfile::MRegfile(Iss &iss) : iss(iss) {
    this->reset(true);
}

inline void MRegfile::reset(bool active){
    if (active){
        for (int i=0; i<16; i++){
            for (int j=0; j<4; j++){
                for (int k=0; k<16; k++){
                    this->mregs[i][j][k] = 0;
                }
            }
        }

        for (int i=0; i<4; i++){
            for (int j=0; j<4; j++){
                for (int k=0; k<16; k++){
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
}

/*
Iss::Iss(IssWrapper &top)
    : prefetcher(*this), exec(top, *this), insn_cache(*this), decode(*this), timing(*this), core(*this), irq(*this),
      gdbserver(*this), lsu(top, *this), dbgunit(*this), syscalls(top, *this), trace(*this), csr(*this),
      regfile(top, *this), exception(*this), quadrilatero(*this), memcheck(top, *this), top(top)
{
}
*/