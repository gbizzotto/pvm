#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// for check:
#include <algorithm>
#include <iterator>
#include <iostream>

std::vector<std::string> read_file(const char * filneame)
{
	std::ifstream file(filneame);
	std::vector<std::string> ff;
	std::string line;
	while (std::getline(file, line))
		ff.emplace_back(std::move(line));
	return ff;
}

struct coords
{
	size_t x,y;
};

coords find_coords(const std::vector<std::string> & code)
{
	for (size_t y=0 ; y<code.size() ; y++)
	{
		auto & line = code[y];
		for (size_t x=0 ; x<line.size() ; x++)
			if (line[x] == '$')
				return {x,y};
	}
	return {0,0};
}

struct PATHProgram
{
	struct Context
	{
		enum Direction {
			Right,
			Up,
			Left,
			Down,
		};
		PATHProgram & program;
		coords cursor;
		std::deque<std::int8_t> memory = {0};
		int memory_ptr = 0;
		Direction direction = Direction::Right;
		bool valid()
		{
			return cursor.y < program.code.size() 
				&& cursor.x < program.code[cursor.y].size()
				&& program.code[cursor.y][cursor.x] != '#'
				&& memory_ptr < 1024*1024
				;
		}
		void next()
		{
			switch(direction)
			{
				case Direction::Left:  --cursor.x; break;
				case Direction::Right: ++cursor.x; break;
				case Direction::Up:    --cursor.y; break;
				case Direction::Down:  ++cursor.y; break;
			}
		}
		char bytecode()
		{
			return program.code[cursor.y][cursor.x];
		}
		auto & bytememory()
		{
			return memory[memory_ptr];
		}
	};

	const std::vector<std::string> code;
	const Context starting_context;

	PATHProgram(std::vector<std::string> && code_)
		: code(std::move(code_))
		, starting_context{*this, find_coords(code)}
	{}

	void run()
	{
		Context ctx = starting_context;
		while(ctx.valid())
			run_one(ctx);
	}
	void debug()
	{
		Context ctx = starting_context;
		while(ctx.valid())
		{
			std::cout << ctx.program.code[ctx.cursor.y] << std::endl;
			std::cout << std::string(ctx.cursor.x, ' ') << '^' << std::endl;
			std::string memory_string;
			size_t pos = 0;
			for (int mem_ptr = (ctx.memory_ptr<15?0:(ctx.memory_ptr-15)) ; mem_ptr < ctx.memory_ptr+16 && mem_ptr < ctx.memory.size() ; mem_ptr++)
			{
				if (mem_ptr == ctx.memory_ptr)
					pos = memory_string.size();
				memory_string.append(std::to_string((int) ctx.memory[mem_ptr])).append(" ");
			}
			std::cout << memory_string << std::endl;
			std::cout << std::string(pos, ' ') << "^" << std::endl;
			char dummy;
			std::cin >> dummy;
			if (dummy == 'r')
				while(ctx.valid() && ctx.bytecode() != 'b')
					run_one(ctx);
			else
				run_one(ctx);
		}
	}
	void run_one(Context & ctx)
	{
		switch (ctx.bytecode())
		{
			case '+': ++ctx.memory[ctx.memory_ptr]; break;
			case '-': --ctx.memory[ctx.memory_ptr]; break;
			case '}': ++ctx.memory_ptr; while (ctx.memory_ptr>=ctx.memory.size()) {ctx.memory.push_back(0);} break;
			case '{': --ctx.memory_ptr; while (ctx.memory_ptr<0) {ctx.memory.push_front(0);ctx.memory_ptr++;} break;
			case '/':
				     if (ctx.direction == Context::Direction::Left ) ctx.direction = Context::Direction::Down;
				else if (ctx.direction == Context::Direction::Down ) ctx.direction = Context::Direction::Left;
				else if (ctx.direction == Context::Direction::Right) ctx.direction = Context::Direction::Up;
				else if (ctx.direction == Context::Direction::Up   ) ctx.direction = Context::Direction::Right;
				break;
			case '\\':
				     if (ctx.direction == Context::Direction::Left ) ctx.direction = Context::Direction::Up;
				else if (ctx.direction == Context::Direction::Up   ) ctx.direction = Context::Direction::Left;
				else if (ctx.direction == Context::Direction::Right) ctx.direction = Context::Direction::Down;
				else if (ctx.direction == Context::Direction::Down ) ctx.direction = Context::Direction::Right;
				break;
			case '!': ctx.next(); break;
			case '^': if (ctx.bytememory()) ctx.direction = Context::Direction::Up;    break;
			case 'v': if (ctx.bytememory()) ctx.direction = Context::Direction::Down;  break;
			case '>': if (ctx.bytememory()) ctx.direction = Context::Direction::Right; break;
			case '<': if (ctx.bytememory()) ctx.direction = Context::Direction::Left;  break;
			case ',': std::cin >> ctx.bytememory(); break;
			case '.': std::cout << std::string(1, ctx.bytememory()); break;
		}
		ctx.next();
	}
};

int main(int argc, char ** argv)
{
	PATHProgram prog(read_file(argv[argc-1]));

	if (std::string("-d") == argv[1])
		prog.debug();
	else
		prog.run();
	std::cout << std::endl;
	return 0;
}