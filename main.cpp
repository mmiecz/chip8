#include <iostream>
#include <cstdint>
#include <vector>
#include <cassert>
#include <cstring>
#include <memory>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <boost/format.hpp>

#include "instruction_set.h"

void vec_print(std::vector<std::uint8_t> &vec) {
    using namespace std;
    int c = 0;
    for( auto e : vec ) {
        cout << hex << setfill('0') << setw(2) << (int)e << " ";
        ++c;
        if( c == 16) {
            cout << endl;
            c = 0;
        }
    }
    cout << endl;
}

class Mem {
public:
    static constexpr std::uint16_t MEM_START = 0x0;
    static constexpr std::uint16_t PROG_BEGIN = 0x200;
    static constexpr std::uint16_t ETI_BEGIN = 0x600;
    static constexpr std::uint16_t MEM_END = 0xFFF;

    enum Stack {
        STACK_ADDR_BEGIN = 0xEA0,
        STACK_ADDR_END = 0xEFF,
    };

    void init() {
        for(int i = 0; i < MEM_END; i++ ) {
            m_mem[i] = 0;
        }
        // Init fonts
    }

    void store(std::uint16_t addr, std::vector<std::uint8_t> bytes ) {
        size_t len = bytes.size();
        for( int i = 0; i < len; ++i ) {
            m_mem[addr + i] = bytes[i];
        }
    }
    void store(std::uint16_t addr, std::uint16_t val ) {
        m_mem[addr] = (uint8_t) ((val >> 8) & 0xFF);
        m_mem[addr+1] = (uint8_t) ((val & 0x00FF));
        std::cout << boost::format("store %1$04x at %2$04x\n") % val % addr;
    }

    std::uint16_t load( std::uint16_t addr ) {
        uint16_t  val = (uint16_t) ((m_mem[addr] << 8) | (m_mem[addr + 1] & 0xFF));
        std::cout << boost::format("load %1$04x from %2$04x\n") % val % addr;
        return (uint16_t) ((m_mem[addr] << 8) | (m_mem[addr + 1] & 0xFF));
    }
    void dump() {
        std::vector<std::uint8_t> v;
        v.assign(m_mem, m_mem+MEM_END);
        std::cout << "Mem dump:" << std::endl;
        vec_print(v);
    }

private:
    std::uint8_t m_mem[MEM_END];

};

class CPU {
public:
    static const int REG_NUMBER = 16;

    void init(std::unique_ptr<Mem> memory) {
        m_mem = std::move( memory );
        std::cout << "Initializing CPU registers" << std::endl;
        for( int i = 0; i < REG_NUMBER; ++i ) {
            m_regs.V[i] = 0x00;
        }
        m_regs.VF = 0x0;
        m_regs.PC = 0x200;
        m_regs.DT = 0x00;
        m_regs.ST = 0x00;
        m_regs.SP = Mem::Stack::STACK_ADDR_END;

        m_regs.reg_dump();
    }

    struct Registers {
        std::uint8_t V[REG_NUMBER]; // 16 Registers
        std::uint16_t VF; // Flags register
        std::uint16_t PC; // Program counter
        std::uint16_t SP; // Stack Pointer
        std::uint16_t I; // Index register
        std::uint8_t DT;
        std::uint8_t ST;

        void reg_dump() {
            using namespace std;
            for(int i = 0; i < REG_NUMBER; ++i ) {
                cout << boost::format( "V%1$X  " ) % i;
            }
            cout << endl;
            for(int i = 0; i < REG_NUMBER; ++i ) {
                cout << boost::format("%1$02x  ") % (int)V[i];
            }
            cout << endl;
            cout << boost::format("VF: 0x%1$02x\n") % (int) VF;
            cout << boost::format("PC: 0x%1$02x\n") % (int) PC;
            cout << boost::format("SP: 0x%1$02x\n") % (int) SP;
            cout << boost::format("DT: 0x%1$02x\n") % (int) DT;
            cout << boost::format("ST: 0x%1$02x\n") % (int) ST;
            cout << boost::format("I:  0x%1$02x") % (int) I;
            cout << endl;
        }
    };
    struct Registers m_regs;
    std::unique_ptr<Mem> m_mem;

    std::uint16_t fetch() {
        uint16_t  data = m_mem->load(m_regs.PC);
        m_regs.PC += 2;
        return data;
    }

    bool decode( std::uint16_t op ) {
        if( (op & 0x6000) == 0x6000) {
            auto reg = (op & 0x0F00) >> 8;
            auto val = (op & 0x00FF);
            std::cout << "mov " << val << " V" << reg << std::endl;
            m_regs.V[reg] = (uint8_t) val;
            return true;
        }
        else if( (op & 0xa000) == 0xa000 ) {
            auto val = (op & 0x0FFF);
            std::cout << "mvi " << ((op & 0x0FFF)) << std::endl;
            m_regs.I = (uint16_t) val;
            return true;
        }
        else if( (op & 0xd000) == 0xd000) {
            std::cout << boost::format("sprite %1$02x %2$02x %3$02x\n") % ((op & 0x0F00) >> 8) % ((op & 0x00F0) >> 4) % ( op & 0x000F);
            return true;
        }
        else if( (op & 0x2000) == 0x2000 ) {
            std::cout << boost::format("call %1$03x\n") % (op & 0x0FFF);
            m_mem->store(m_regs.SP--, (uint16_t) m_regs.PC);
            m_regs.PC = (uint16_t) ( op & 0x0FFF);
            return true;
        }
        else if ( op == 0x00EE) {
            m_regs.SP++;
            auto addr = m_mem->load(m_regs.SP);
            m_regs.PC = addr;
            std::cout << "ret" << std::endl;
            return true;
        }
        else {
            std::cout << boost::format( "Unknown opcode %1$04x\n") % (int)op;
        }
        return false;
    }
private:

};

int main(int argc, char* argv[]) {
    using namespace std;
    if(argc != 2 ) {
        cerr << "Usage" << argv[0] << " ROM" << std::endl;
        exit(-1);
    }
    ifstream input(argv[1], ios::binary);
    if(!input.is_open()) {
        cerr << "Cannot open file: " << argv[1] << std::endl;
    }
    input.unsetf(ios::skipws);

    vector<uint8_t> rom;
    rom.insert(rom.begin(), istream_iterator<uint8_t>(input), istream_iterator<uint8_t>());

    std::unique_ptr<Mem> mem(new Mem);
    mem->init();
    mem->store(Mem::PROG_BEGIN, rom);

    CPU cpu;
    cpu.init( std::move( mem ) );


    while( true ) {
        auto op = cpu.fetch();
        if(!cpu.decode(op)) {
            break;
        }
        cpu.m_regs.reg_dump();
    }
    return 0;
}