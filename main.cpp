#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include <memory>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <boost/format.hpp>


#include <thread>

#define CHIP8_ASSERT(x, msg) BOOST_ASSERT_MSG(x, msg)

void vec_print(std::vector<std::uint8_t> &vec) {
    using namespace std;
    int c = 0;
    for (auto e : vec) {
        cout << hex << setfill('0') << setw(2) << (int) e << " ";
        ++c;
        if (c == 16) {
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
        for (int i = 0; i < MEM_END; i++) {
            m_mem[i] = 0;
        }
        // Init fonts
        store(0x0, FONTS);
    }

    void store(std::uint16_t addr, std::vector<std::uint8_t> bytes) {
        size_t len = bytes.size();
        CHIP8_ASSERT(len < addr - MEM_END, "Fatal, read more bytes than available RAM");
        for (int i = 0; i < len; ++i) {
            m_mem[addr + i] = bytes[i];
        }
    }

    void store(std::uint16_t addr, std::uint16_t val) {
        m_mem[addr] = (uint8_t) ((val >> 8) & 0xFF);
        m_mem[addr + 1] = (uint8_t) ((val & 0x00FF));
    }

    void store(std::uint16_t addr, std::uint8_t val) {
        m_mem[addr] = (uint8_t) ((val) & 0xFF);
    }

    std::uint16_t loadDW(std::uint16_t addr) {
        uint16_t val = (uint16_t) ((m_mem[addr] << 8) | (m_mem[addr + 1] & 0xFF));
        return (uint16_t) ((m_mem[addr] << 8) | (m_mem[addr + 1] & 0xFF));
    }

    std::uint8_t loadW(std::uint16_t addr) {
        return (std::uint8_t) (m_mem[addr]);
    }

    void dump() {
        std::vector<std::uint8_t> v;
        v.assign(m_mem, m_mem + MEM_END);
        std::cout << "Mem dump:" << std::endl;
        vec_print(v);
    }

private:
    std::uint8_t m_mem[MEM_END];
    const std::vector<std::uint8_t> FONTS = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                                            0x20, 0x60, 0x20, 0x20, 0x70, // 1
                                            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                                            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                                            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                                            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                                            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                                            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                                            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                                            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                                            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                                            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                                            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                                            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                                            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                                            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };


};

class CPU {
public:
    static const int REG_NUMBER = 16;

    void init(std::unique_ptr<Mem> memory) {
        m_mem = std::move(memory);
        std::cout << "Initializing CPU registers" << std::endl;
        for (int i = 0; i < REG_NUMBER; ++i) {
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
            for (int i = 0; i < REG_NUMBER; ++i) {
                cout << boost::format("V%1$X  ") % i;
            }
            cout << endl;
            for (int i = 0; i < REG_NUMBER; ++i) {
                cout << boost::format("%1$02x  ") % (int) V[i];
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
        uint16_t data = m_mem->loadDW(m_regs.PC);
        m_regs.PC += 2;
        return data;
    }

    bool decode(std::uint16_t op) {
        if ((op & 0xF000) == 0x6000) {
            auto reg = (op & 0x0F00) >> 8;
            auto val = (op & 0x00FF);
            std::cout << "mov " << val << " V" << reg << std::endl;
            m_regs.V[reg] = (uint8_t) val;
        }
        else if ((op & 0xF000) == 0xa000) {
            auto val = (op & 0x0FFF);
            std::cout << "mvi " << ((op & 0x0FFF)) << std::endl;
            m_regs.I = (uint16_t) val;
        }
        else if ((op & 0xF000) == 0xd000) {
            std::cout << boost::format("sprite %1$02x %2$02x %3$02x\n") % ((op & 0x0F00) >> 8) % ((op & 0x00F0) >> 4) %
                         (op & 0x000F);
            // TODO
        }
        else if ((op & 0xF000) == 0x2000) {
            std::cout << boost::format("call %1$03x\n") % (op & 0x0FFF);
            m_mem->store(m_regs.SP--, (uint16_t) m_regs.PC);
            m_regs.PC = (uint16_t) (op & 0x0FFF);
        }
        else if (op == 0x00EE) {
            m_regs.SP++;
            auto addr = m_mem->loadDW(m_regs.SP);
            m_regs.PC = addr;
            std::cout << "ret" << std::endl;
        }
        else if ((op & 0xF000) == 0xF000 && (op & 0x00FF) == 0x0033) {
            auto val = m_regs.V[(op & 0x0F00) >> 8];
            uint8_t units = (uint8_t) (val % 10);
            uint8_t tens = (uint8_t) ((val / 10) % 10);
            uint8_t hundreds = (uint8_t) ((val / 100) % 10);
            uint16_t addr = m_regs.I;
            m_mem->store(addr++, hundreds);
            m_mem->store(addr++, tens);
            m_mem->store(addr, units);
        }
        else if ((op & 0xF000) == 0xF000 && (op & 0x00FF) == 0x0065) {
            auto reg_end = (op & 0x0F00) >> 8;
            for (int i = 0; i < reg_end; ++i) {
                m_regs.V[i] = m_mem->loadW(m_regs.I++);
            }
        }
        else if ((op& 0xF000) == 0xF000 && (op & 0x00FF) == 0x0029) {
            auto val = (op & 0x0F00) >> 8;
            CHIP8_ASSERT( val < REG_NUMBER, "Invalid Reg number in instruction Fx29");
            auto chr = m_regs.V[val]; // Char in vx
            std::uint16_t addr = (uint16_t) (chr * 5); // Calculate address of font
            m_regs.I = addr;
        }
        else if ( (op & 0xF000) == 0x7000) {
            std::uint8_t add = (uint8_t) (op & 0x00FF);
            auto reg = (op & 0x0F00) >> 8;
            CHIP8_ASSERT( reg < REG_NUMBER, "Invalid reg number");
            m_regs.V[reg] += add;
        }
        else if ( (op & 0xF000) == 0xF000 && (op & 0x00FF) == 0x0015) {
            auto reg = (op & 0x0F00) >> 8;
            CHIP8_ASSERT( reg < REG_NUMBER, "Invalid reg number");
            m_regs.DT = m_regs.V[reg];
        }
        else if ( (op & 0xF000) == 0xF000 && (op & 0x000F) == 0x0007) {
            auto reg = (uint8_t) ( op & 0x0F00 ) >> 8;
            CHIP8_ASSERT( reg < REG_NUMBER, "Fatal, wrong register");
            m_regs.V[reg] = m_regs.DT;
        }
        else if ( (op & 0xF000 ) == 0x3000 ) {
            auto reg = ( op & 0x0F00 ) >> 8;
            uint16_t  val = (uint16_t) (op & 0x00FF);
            if ( m_regs.V[reg] == val )
                m_regs.PC += 2;
        }
        else if ( (op & 0xF000 ) == 0x1000 ) {
            uint16_t addr = (uint16_t) (op & 0x0FFF );
            m_regs.PC = addr;
            std::cout << boost::format( "LD %1$03x PC\n") % addr;
        }
        else {
            std::cout << boost::format("Unknown opcode %1$04x\n") % (int) op;
            return false;
        }
        return true;
    }

private:

};

int main(int argc, char *argv[]) {
    using namespace std;
    if (argc != 2) {
        cerr << "Usage" << argv[0] << " ROM" << std::endl;
        exit(-1);
    }
    ifstream input(argv[1], ios::binary);
    if (!input.is_open()) {
        cerr << "Cannot open file: " << argv[1] << std::endl;
    }
    input.unsetf(ios::skipws);

    vector<uint8_t> rom;
    rom.insert(rom.begin(), istream_iterator<uint8_t>(input), istream_iterator<uint8_t>());

    std::unique_ptr<Mem> mem(new Mem);
    mem->init();
    mem->store(Mem::PROG_BEGIN, rom);

    CPU cpu;
    cpu.init(std::move(mem));


#if defined DEBUG
    using clock_tick = std::chrono::duration<float, std::ratio<1,4>>;
#else
    using clock_tick = std::chrono::duration<float, std::ratio<1,60>>;
#endif
    std::cout << "One clock cycle is: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(clock_tick(1)).count()
              << " ms" << std::endl;

    auto tick = clock_tick(1);
    while (true) {

        auto op = cpu.fetch();
        if (!cpu.decode(op)) {
            break;
        }
        std::this_thread::sleep_for(tick);
        std::cout << boost::format( "tick: %1$03x\n" ) % op;
    }

    cpu.m_mem->dump();
    cpu.m_regs.reg_dump();
    return 0;
}