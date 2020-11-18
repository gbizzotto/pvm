
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct PATHVM
{
	enum Bytecode : char
	{
		ADD = '+',
		SUB = '-',
		DPADD = '}',
		DPSUB = '{',
		CLEFT = '<',
		CRIGHT = '>',
		CUP = '^',
		CDOWN = 'v',
		SKIP = '!',
		LURD = '\\',
		RULD = '/',
		IN = ',',
		OUT = '.',
		Start = '$',
		End = '#',
		Breakpoint = 'b',
	};
	struct Context;
	struct Program
	{
		using VM = PATHVM;
		std::vector<std::string> code;
		Bytecode bytecode(const Context & ctx) const { return Bytecode(code[ctx.IP_y][ctx.IP_x]); }
		void print_location(const Context & ctx) const {
			std::cout << "l." << ctx.IP_y+1 << ": " << code[ctx.IP_y] << std::endl;
			std::cout << "l." << ctx.IP_y+1 << ": " << std::string(ctx.IP_x, ' ') << '^' << std::endl;
		}
	};
	struct Context
	{
		enum Direction {
			Right,
			Up,
			Left,
			Down,
		};
		std::deque<char> memory = std::deque<char>(128);
		size_t IP_x, IP_y;
		size_t DP;
		Direction direction = Direction::Right;
		std::istream & in = std::cin;
		std::ostream & out = std::cout;
		bool is_valid(const Program & p) const { 
			return IP_y < p.code.size() 
				&& IP_x < p.code[IP_y].size()
				&& p.bytecode(*this) != Bytecode::End
				&& DP < memory.size()
				;
		}
		bool is_breakpoint(const Program & program) const {
			return program.bytecode(*this) == Bytecode::Breakpoint;
		}
		void print_debug_info() const {
			std::string memory_string = "@.";
			size_t pos = 0;
			memory_string.append(std::to_string(DP)).append(" ");
			for (int mem_ptr = (DP<15?0:(DP-15)) ; mem_ptr < DP+16 && mem_ptr < memory.size() ; mem_ptr++)
			{
				if (mem_ptr == DP)
					pos = memory_string.size();
				memory_string.append(std::to_string((int) memory[mem_ptr])).append(" ");
			}
			std::cout << memory_string << std::endl;
			std::cout << std::string(pos, ' ') << "^" << std::endl;
		}
		void next() {
			switch(direction)
			{
				case Left:  --IP_x; break;
				case Right: ++IP_x; break;
				case Up:    --IP_y; break;
				case Down:  ++IP_y; break;
			}
		}
		void run_one(Program & p) {
			switch ((char)p.bytecode(*this))
			{
				case ADD: ++memory[DP]; break;
				case SUB: --memory[DP]; break;
				case DPADD: ++DP; while (DP>=memory.size()) {memory.push_back(0);} break;
				case DPSUB: --DP; while (DP<0) {memory.push_front(0);DP++;} break;
				case RULD:
					     if (direction == Left ) direction = Down;
					else if (direction == Down ) direction = Left;
					else if (direction == Right) direction = Up;
					else if (direction == Up   ) direction = Right;
					break;
				case LURD:
					     if (direction == Left ) direction = Up;
					else if (direction == Up   ) direction = Left;
					else if (direction == Right) direction = Down;
					else if (direction == Down ) direction = Right;
					break;
				case SKIP: next(); break;
				case CUP:    if (memory[DP]) direction = Up;    break;
				case CDOWN:  if (memory[DP]) direction = Down;  break;
				case CRIGHT: if (memory[DP]) direction = Right; break;
				case CLEFT:  if (memory[DP]) direction = Left;  break;
				case IN: std::cin >> memory[DP]; break;
				case OUT: std::cout << memory[DP]; break;
				case Breakpoint:
					{ int x=0; } // just so we can set a breakpoint here
					break;
				default: break;
			}
			next();
		}
	};

	static Program load(std::string path) {
		std::ifstream file(path);
		Program program;
		std::string line;
		while (std::getline(file, line))
			program.code.emplace_back(std::move(line));
		return program;
	}
	static Context make_context(const Program & p) {
		Context ctx;
		for (ctx.IP_y = 0 ; ctx.is_valid(p) ; ctx.IP_y++,ctx.IP_x=0)
			for ( ; ctx.is_valid(p) ; ctx.IP_x++)
				if (p.bytecode(ctx) == Bytecode::Start)
					return ctx;
		ctx.IP_x = 0;
		ctx.IP_y = 0;
		return ctx;
	}
};

template<typename P>
int run(P & program, bool debug)
{
	auto ctx = P::VM::make_context(program);
	if ( ! debug)
		while(ctx.is_valid(program))
			ctx.run_one(program);
	else
	{
		while(ctx.is_valid(program))
		{
			program.print_location(ctx);
			ctx.print_debug_info();
			char dummy;
			std::cin >> dummy;
			if (dummy == 'r') {
				if (ctx.is_valid(program))
					ctx.run_one(program);
				while(ctx.is_valid(program) && ! ctx.is_breakpoint(program))
					ctx.run_one(program);
			}
			else
				ctx.run_one(program);
		}
	}
	return 0;
}

int main(int argc, char ** argv)
{
	bool debug = std::find_if(&argv[0], &argv[argc], [](const char * param){ return strncmp("-d", param, 2) == 0; }) != &argv[argc];
	auto program = PATHVM::load(argv[argc-1]);
	auto x = run(program, debug);
	std::cout << std::endl;
	return x;
}
