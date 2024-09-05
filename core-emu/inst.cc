#include "hart.h"
#include "soc.h"

#include "decode.h"
#include "misc.h"

#include <algorithm>

static inline sword_t sgn(word_t x) {
  return *reinterpret_cast<sword_t *>(&x);
}

void Hart::do_inst() {
  const vaddr_t pc = get_pc();
  vaddr_t dnpc = pc;
  const inst_t inst = inst_fetch();

  //  Illegal instruction 非法指令异常
  #define EXC_II ((Exception {2, inst}))

  // C拓展压缩指令
  if (inst_t c_op = c_opcode(inst); c_op != 0b11) {
    dnpc = pc + 2;

    const inst_t func = bits<15, 13>(inst);
    
    if (c_op == 0b00) {
      if (func == 0b000) { // c.addi4spn
        word_t imm = (bits<12, 11>(inst) << 4) |
                     (bits<10,  7>(inst) << 6) |
                     (bits< 6,  6>(inst) << 2) |
                     (bits< 5,  5>(inst) << 3) ;
        if (imm == 0) throw EXC_II; // reserved
        inst_t rd = 0b01000 | bits<4, 2>(inst);
        gpr_write(rd, gpr_read(2) + imm);
      } else if (func == 0b010) { // c.lw
        word_t imm = (bits<12, 10>(inst) << 3) |
                     (bits< 6,  6>(inst) << 2) |
                     (bits< 5,  5>(inst) << 6) ;
        word_t src1 = gpr_read(0b01000 | bits<9, 7>(inst));
        inst_t rd = 0b01000 | bits<4, 2>(inst);
        word_t data = sext<32>(vaddr_load(src1 + imm, 4));
        gpr_write(rd, data);
      } else if (func == 0b110) { // c.sw
        word_t imm = (bits<12, 10>(inst) << 3) |
                     (bits< 6,  6>(inst) << 2) |
                     (bits< 5,  5>(inst) << 6) ;
        word_t src1 = gpr_read(0b01000 | bits<9, 7>(inst));
        word_t src2 = gpr_read(0b01000 | bits<4, 2>(inst));
        vaddr_store(src1 + imm, 4, src2);
      } else throw EXC_II;
    } else if (c_op == 0b01) {
      if (func == 0b000) { // c.addi
        word_t imm = (bits<12, 12>(inst) << 5) |
                     (bits< 6,  2>(inst) << 0) ;
        imm = sext<6>(imm);
        inst_t rd = bits<11, 7>(inst);
        word_t src = gpr_read(rd);
        gpr_write(rd, src + imm);
      } else if (func == 0b001) { // c.jal
        word_t imm = (bits<12, 12>(inst) << 11) |
                     (bits<11, 11>(inst) <<  4) |
                     (bits<10,  9>(inst) <<  8) |
                     (bits< 8,  8>(inst) << 10) |
                     (bits< 7,  7>(inst) <<  6) |
                     (bits< 6,  6>(inst) <<  7) |
                     (bits< 5,  3>(inst) <<  1) |
                     (bits< 2,  2>(inst) <<  5) ;
        imm = sext<12>(imm);
        dnpc = pc + imm;
        gpr_write(1, pc + 2);
      } else if (func == 0b010) { // c.li
        word_t imm = (bits<12, 12>(inst) << 5) |
                     (bits< 6,  2>(inst) << 0) ;
        imm = sext<6>(imm);
        inst_t rd = bits<11, 7>(inst);
        gpr_write(rd, imm);
      } else if (func == 0b011) {
        inst_t rd = bits<11, 7>(inst);
        if (rd == 2) { // c.addi16sp
          word_t imm = (bits<12, 12>(inst) << 9) |
                       (bits< 6,  6>(inst) << 4) |
                       (bits< 5,  5>(inst) << 6) |
                       (bits< 4,  3>(inst) << 7) |
                       (bits< 2,  2>(inst) << 5) ;
          imm = sext<10>(imm);
          if (imm == 0) throw EXC_II; // reserved
          gpr_write(2, gpr_read(2) + imm);
        } else { // c.lui
          word_t imm = (bits<12, 12>(inst) << 17) |
                       (bits< 6,  2>(inst) << 12) ;
          imm = sext<18>(imm);
          if (imm == 0) throw EXC_II; // reserved
          gpr_write(rd, imm);
        }
      } else if (func == 0b100) {
        inst_t rd = 0b01000 | bits<9, 7>(inst);
        word_t src1 = gpr_read(rd);
        word_t res;
        if (inst_t f = bits<11, 10>(inst); f != 0b11) {
          word_t imm = (bits<12, 12>(inst) << 5) |
                       (bits< 6,  2>(inst) << 0) ;
          switch (f) {
            case 0b00: res = src1 >> (imm & bit_mask(5)); break;
            case 0b01: res = sgn(src1) >> (imm & bit_mask(5)); break;
            case 0b10: res = src1 & sext<6>(imm); break;
            default: assert(0);
          }
        } else {
          if (bits<12, 12>(inst)) throw EXC_II;
          word_t src2 = gpr_read(0b01000 | bits<4, 2>(inst));
          switch (bits<6, 5>(inst)) {
            case 0b00: res = src1 - src2; break;
            case 0b01: res = src1 ^ src2; break;
            case 0b10: res = src1 | src2; break;
            case 0b11: res = src1 & src2; break;
            default: assert(0);
          }
        }
        gpr_write(rd, res);
      } else if (func == 0b101) { // c.j
        word_t imm = (bits<12, 12>(inst) << 11) |
                     (bits<11, 11>(inst) <<  4) |
                     (bits<10,  9>(inst) <<  8) |
                     (bits< 8,  8>(inst) << 10) |
                     (bits< 7,  7>(inst) <<  6) |
                     (bits< 6,  6>(inst) <<  7) |
                     (bits< 5,  3>(inst) <<  1) |
                     (bits< 2,  2>(inst) <<  5) ;
        imm = sext<12>(imm);
        dnpc = pc + imm;
      } else {
        word_t src1 = gpr_read(0b01000 | bits<9, 7>(inst));
        word_t imm = (bits<12, 12>(inst) << 8) |
                     (bits<11, 10>(inst) << 3) |
                     (bits< 6,  5>(inst) << 6) |
                     (bits< 4,  3>(inst) << 1) |
                     (bits< 2,  2>(inst) << 5) ;
        imm = sext<9>(imm);
        bool jump_en = false;
        switch (func) {
          case 0b110: jump_en = src1 == 0; break;
          case 0b111: jump_en = src1 != 0; break;
          default: assert(0);
        }
        if (jump_en) dnpc = pc + imm;
      }
    } else if (c_op == 0b10) {
      if (func == 0b000) { // c.slli
        inst_t rd = bits<11, 7>(inst);
        word_t src1 = gpr_read(rd);
        word_t imm = (bits<12, 12>(inst) << 5) |
                     (bits< 6,  2>(inst) << 0) ;
        gpr_write(rd, src1 << (imm & bit_mask(5)));
      } else if (func == 0b010) { // c.lwsp
        inst_t rd = bits<11, 7>(inst);
        if (rd == 0) throw EXC_II;
        word_t imm = (bits<12, 12>(inst) << 5) |
                     (bits< 6,  4>(inst) << 2) |
                     (bits< 3,  2>(inst) << 6) ;
        word_t data = sext<32>(vaddr_load(gpr_read(2) + imm, 4));
        gpr_write(rd, data);
      } else if (func == 0b100) {
        inst_t rs1 = bits<11, 7>(inst);
        inst_t rs2 = bits<6, 2>(inst);
        if (bits<12, 12>(inst)) {
          // c.ebreak
          if (rs1 == 0 && rs2 == 0) throw Exception {3, 0};
          if (rs2 == 0) { // c.jalr
            word_t src1 = gpr_read(rs1);
            dnpc = src1 & ~1;
            gpr_write(1, pc + 2);
          } else { // c.add
            gpr_write(rs1, gpr_read(rs1) + gpr_read(rs2));
          }
        } else {
          if (rs2 == 0) { // c.jr
            word_t src1 = gpr_read(rs1);
            dnpc = src1 & ~1;
          } else { // c.mv
            gpr_write(rs1, gpr_read(rs2));
          }
        }
      } else if (func == 0b110) { // c.swsp
        word_t src2 = gpr_read(bits<6, 2>(inst));
        word_t imm = (bits<12, 9>(inst) << 2) |
                     (bits< 8, 7>(inst) << 6) ;
        vaddr_store(gpr_read(2) + imm, 4, src2);
      } else throw EXC_II;
    } else assert(0);

    set_pc(dnpc);
    return;
  }

  inst_t op = opcode(inst);
  dnpc = pc + 4;

  if (op == 0b01101) { // LUI
    gpr_write(rd(inst), immU(inst));
  } else if (op == 0b00101) { // AUIPC
    gpr_write(rd(inst), pc + immU(inst));
  } else if (op == 0b11011) { // JAL
    dnpc = pc + immJ(inst);
    gpr_write(rd(inst), pc + 4);
  } else if (op == 0b11001) { // JALR
    dnpc = (gpr_read(rs1(inst)) + immI(inst)) & ~1;
    gpr_write(rd(inst), pc + 4);
  } else if (op == 0b11000) { // BRANCH
    word_t src1 = gpr_read(rs1(inst));
    word_t src2 = gpr_read(rs2(inst));

    bool jump_en = false;
    switch (funct3(inst)) {
      case 0b000: jump_en = (src1 == src2); break;
      case 0b001: jump_en = (src1 != src2); break;
      case 0b100: jump_en = (sgn(src1) < sgn(src2)); break;
      case 0b101: jump_en = (sgn(src1) >= sgn(src2)); break;
      case 0b110: jump_en = (src1 < src2); break;
      case 0b111: jump_en = (src1 >= src2); break;
      default: if constexpr (rt_check) assert(0);
    }

    if (jump_en) {
      dnpc = pc + immB(inst);
    }
  } else if (op == 0b00000) { // LOAD
    vaddr_t addr = gpr_read(rs1(inst)) + immI(inst);
    word_t data = 0;

    switch (funct3(inst)) {
      case 0b000: data = sext<8>(vaddr_load(addr, 1)); break;
      case 0b001: data = sext<16>(vaddr_load(addr, 2)); break;
      case 0b010: data = sext<32>(vaddr_load(addr, 4)); break;
      case 0b100: data = vaddr_load(addr, 1); break;
      case 0b101: data = vaddr_load(addr, 2); break;
      default: if constexpr (rt_check) assert(0);
    }

    gpr_write(rd(inst), data);
  } else if (op == 0b01000) { // STORE
    vaddr_t addr = gpr_read(rs1(inst)) + immS(inst);
    word_t data = gpr_read(rs2(inst));

    switch (funct3(inst)) {
      case 0b000: vaddr_store(addr, 1, data); break;
      case 0b001: vaddr_store(addr, 2, data); break;
      case 0b010: vaddr_store(addr, 4, data); break;
      default: if constexpr (rt_check) assert(0);
    }
  } else if (op == 0b00100) { // CALRI
    word_t src1 = gpr_read(rs1(inst));
    word_t imm = immI(inst);
    word_t res = 0;

    switch (funct3(inst)) {
      case 0b000: res = src1 + imm; break;
      case 0b010: res = sgn(src1) < sgn(imm); break;
      case 0b011: res = src1 < imm; break;
      case 0b100: res = src1 ^ imm; break;
      case 0b110: res = src1 | imm; break;
      case 0b111: res = src1 & imm; break;
      case 0b001:
        if constexpr (rt_check) assert(funct7(inst) == 0);
        res = src1 << imm;
      break;
      case 0b101:
        switch (funct7(inst)) {
          case 0b0000000: res = src1 >> imm; break;
          case 0b0100000: res = sgn(src1) >> (imm & bit_mask(5)); break;
          default: if constexpr (rt_check) assert(0);
        }
      break;
      default: if constexpr (rt_check) assert(0);
    }

    gpr_write(rd(inst), res);
  } else if (op == 0b01100) { // CALRR
    constexpr word_t WORD_MIN = static_cast<word_t>(1) << (xlen - 1);
    constexpr word_t ALL_ONE = static_cast<word_t>(-1);
    inst_t f7 = funct7(inst), f3 = funct3(inst);
    word_t src1 = gpr_read(rs1(inst)), src2 = gpr_read(rs2(inst));
    word_t res = 0;
    if (f7 & 1) { // RV32M
      if constexpr (rt_check) assert(f7 == 1);
      switch (f3) {
        case 0b000: res = src1 * src2; break;
        case 0b001: res = (static_cast<int64_t>(sgn(src1)) * static_cast<int64_t>(sgn(src2))) >> 32; break;
        case 0b010: res = (static_cast<int64_t>(sgn(src1)) * static_cast<int64_t>(src2)) >> 32; break;
        case 0b011: res = (static_cast<uint64_t>(src1) * static_cast<uint64_t>(src2)) >> 32; break;
        case 0b100: // div
          if (src1 == WORD_MIN && src2 == ALL_ONE) res = src1;
          else if (src2) res = sgn(src1) / sgn(src2);
          else res = ALL_ONE;
        break;
        case 0b101: // divu
          if (src2) res = src1 / src2;
          else res = ALL_ONE;
        break;
        case 0b110: // rem
          if (src1 == WORD_MIN && src2 == ALL_ONE) res = 0;
          else if (src2) res = sgn(src1) % sgn(src2);
          else res = src1;
        break;
        case 0b111: // remu
          if (src2) res = src1 % src2;
          else res = src1;
        break;
        default: if constexpr (rt_check) assert(0);
      }
    } else {
      switch (f3) {
        case 0b000: // add sub
          switch (f7) {
            case 0b0000000: res = src1 + src2; break;
            case 0b0100000: res = src1 - src2; break;
            default: if constexpr (rt_check) assert(0);
          }
        break;
        case 0b001: // sll
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 << (src2 & bit_mask(5));
        break;
        case 0b010: // slt
          if constexpr (rt_check) assert(f7 == 0);
          res = sgn(src1) < sgn(src2);
        break;
        case 0b011: // sltu
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 < src2;
        break;
        case 0b100: // xor
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 ^ src2;
        break;
        case 0b101: // srl sra
          switch (f7) {
            case 0b0000000: res = src1 >> (src2 & bit_mask(5)); break;
            case 0b0100000: res = sgn(src1) >> (src2 & bit_mask(5)); break;
            default: if constexpr (rt_check) assert(0);
          }
        break;
        case 0b110: // or
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 | src2;
        break;
        case 0b111: // and
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 & src2;
        break;
        default: if constexpr (rt_check) assert(0);
      }
    }

    gpr_write(rd(inst), res);
  } else if (op == 0b11100) { // SYS
    inst_t f3 = funct3(inst);
    if (f3) { // zicsr
      const word_t csr_addr = funct12(inst);
      // TVM=1，在S模式操作satp会触发非法指令异常
      if (priv == PRIV_S && csr_addr == 0x180 && mstatus_TVM) {
        throw EXC_II;
      }
      word_t csr_rdata;
      try {
        csr_rdata = csr_read(csr_addr);
      } catch (...) {
        // 非法CSR读取，非法指令异常
        throw EXC_II;
      }

      word_t csr_wdata = 0;
      word_t opnd;
      if (f3 & 0b100) opnd = zimm(inst);
      else opnd = gpr_read(rs1(inst));
      switch (f3 & 0b11) {
        case 0b01: csr_wdata = opnd; break;
        case 0b10: csr_wdata = csr_rdata | opnd; break;
        case 0b11: csr_wdata = csr_rdata & ~opnd; break;
        default: if constexpr (rt_check) assert(0);
      }

      if (csr_wdata != csr_rdata) { // CSR写入
        try {
          csr_write(csr_addr, csr_wdata);
        } catch (...) {
          // 非法CSR写入，非法指令异常
          throw EXC_II;
        }
      }
      
      gpr_write(rd(inst), csr_rdata);
    } else {
      if ((inst & ~(bit_mask(10) << 15)) == 0x12000073) { // sfence.vma
        // TVM=1，在S模式执行SFENCE.VMA会触发非法指令异常
        if (priv == PRIV_S && mstatus_TVM) throw EXC_II;
      } else switch (inst) {
        // ecall
        case 0x00000073: throw Exception {8 | priv, 0};
        // ebreak
        case 0x00100073: throw Exception {3, 0};
        // mret
        case 0x30200073:
          if (priv < PRIV_M) throw EXC_II;
          dnpc = csr.mepc;
          // 将mstatus.MPIE恢复到mstatus.MIE
          csr.mstatus = (csr.mstatus & ~(1 << 3)) | (mstatus_MPIE << 3);
          // 将mstatus.MPIE置为1
          csr.mstatus = csr.mstatus | (1 << 7);
          // 将特权级设置为mstatus.MPP
          priv = mstatus_MPP;
          // 若mstatus.MPP不为M模式，将mstatus_MPRV设为0
          if (mstatus_MPP != 0b11) csr.mstatus = (csr.mstatus & ~(1 << 17));
          // 将mstatus.MPP设为U模式，即0
          csr.mstatus = (csr.mstatus & ~(0b11 << 11));
        break;
        // sret
        case 0x10200073:
          if (priv < PRIV_S) throw EXC_II;
          // 若TSR=1，在S模式执行SRET会触发非法指令异常
          if (mstatus_TSR && priv == PRIV_S) throw EXC_II;
          dnpc = csr.sepc;
          // 将mstatus.SPIE恢复到mstatus.SIE
          csr.mstatus = (csr.mstatus & ~(1 << 1)) | (mstatus_SPIE << 1);
          // 将mstatus.SPIE置为1
          csr.mstatus = csr.mstatus | (1 << 5);
          // 将特权级设置为mstatus.SPP
          priv = mstatus_SPP;
          // 若mstatus.SPP不为M模式（总是成立），将mstatus_MPRV设为0
          csr.mstatus = (csr.mstatus & ~(1 << 17));
          // 将mstatus.SPP设为U模式，即0
          csr.mstatus = (csr.mstatus & ~(1 << 8));
        break;
        // wfi
        case 0x10500073:
          // TW=1，在U模式执行wfi直接抛出非法指令异常
          if (mstatus_TW && priv == PRIV_U) throw EXC_II;
          // 否则nop
        break;
        default: if constexpr (rt_check) assert(0);
      }
    }
  } else if (op == 0b00011) { // FENCE
    // nop
  } else if (op == 0b01011) { // A 原子指令拓展
    static vaddr_t reserved_addr = 0;
    if (funct3(inst) != 0b010) throw EXC_II;
    inst_t f5 = funct5(inst);
    word_t vaddr = gpr_read(rs1(inst)); // src1
    if (f5 == 0b00010) { // LR
      if (rs2(inst)) throw EXC_II;
      word_t data = vaddr_load(vaddr, 4);
      gpr_write(rd(inst), sext<32>(data));
      reserved_addr = vaddr;
    } else if (f5 == 0b00011) { // SC
      if (reserved_addr == vaddr) {
        word_t src2 = gpr_read(rs2(inst));
        vaddr_store(vaddr, 4, src2);
        gpr_write(rd(inst), 0);
      } else {
        gpr_write(rd(inst), 1);
      }
      reserved_addr = 0;
    } else { // AMO
      word_t src2 = gpr_read(rs2(inst));
      word_t paddr = mmu_translate(vaddr, ACS_STORE);
      word_t t;
      try {
        t = paddr_read(paddr, 4);
      } catch (...) {
        throw Exception {7, vaddr};
      }
      word_t data;
      switch (f5) {
        case 0b00001: data = src2; break;
        case 0b00000: data = t + src2; break;
        case 0b00100: data = t ^ src2; break;
        case 0b01100: data = t & src2; break;
        case 0b01000: data = t | src2; break;
        case 0b10000: data = std::min(sgn(t), sgn(src2)); break;
        case 0b10100: data = std::max(sgn(t), sgn(src2)); break;
        case 0b11000: data = std::min(t, src2); break;
        case 0b11100: data = std::max(t, src2); break;
        default: throw EXC_II;
      }
      try {
        paddr_write(paddr, 4, data);
      } catch (...) {
        assert(0);
      }
      gpr_write(rd(inst), sext<32>(t));
    }
  } else {
    Log("Warning: invalid instruction: %08x", inst);
    throw EXC_II;
  }

  set_pc(dnpc);
}
