#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>

enum OPERATION_TYPE
{
    INC        = '>',
    DEC        = '<',
    ADD        = '+',
    SUB        = '-',
    OUT        = '.',
    IN         = ',',
    JUMP_FRONT = '[',
    JUMP_BACK  = ']'
};

struct Operation
{
    OPERATION_TYPE type;

    union
    {
        int count;
        int jump_location; // if we have a jump instruction
    };                     // we only store where to jump

    int line;
    int column;
};

int main(int argc, char **argv)
{
    if (argc <= 1 || argc > 2 )
    {
        std::cerr << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    char *input = argv[1];

    std::ifstream file(input);
    if(!file.is_open())
    {
        std::cerr << "failed to open file: " << argv[1] << '\n';
        return 1;
    }

    //collect operations
    std::vector<Operation> operations;
    int line = 1, column = 0;
    char instruction = 0;
    while(file.get(instruction))
    {
        if (instruction == '\n')
        {
            line++;
            column = 0;
        }
        else
        {
            column++;
        }

        switch (instruction)
        {
        case INC:
        case DEC:
        case ADD:
        case SUB:
        case OUT:
        case IN :
            if (operations.size() > 0 && operations[operations.size() - 1].type == instruction)
            {
                operations[operations.size() - 1].count++;
            }
            else
            {
                Operation op = {.type = (OPERATION_TYPE)instruction, .count = 1};
                operations.push_back(op);
            }
            break;

        case JUMP_FRONT:
        case JUMP_BACK:
            {
                Operation op = {.type = (OPERATION_TYPE)instruction, .jump_location = -1, .line = line, .column = column};
                operations.push_back(op);
            }
            break;

        default:
            continue;
            break;
        }
    }
    file.close();

    //fill out the jump instructions
    bool error = false;
    for (int i = 0; i < operations.size(); i++)
    {
        if (operations[i].type == JUMP_FRONT && i + 1 < operations.size())
        {
            int count = 0;
            for (int j = i + 1; j < operations.size(); j++)
            {
                if(operations[j].type == JUMP_BACK)
                {
                    if (count == 0)
                    {
                        operations[i].jump_location = j;
                        operations[j].jump_location = i;
                        break;
                    }
                    else
                    {
                        count--;
                    }
                }
                else if(operations[j].type == JUMP_FRONT)
                {
                    count++;
                }
            }
            if (operations[i].jump_location < 0)
            {
                std::cerr << input << ':' << operations[i].line << ':' << operations[i].column << ": unclosed jump front instruction\n";
                error = true;
            }
        }
    }
    for(int i = 0; i < operations.size(); i++)
    {
        if (operations[i].type == JUMP_BACK && operations[i].jump_location < 0)
        {
            std::cerr << input << ':' << operations[i].line << ':' << operations[i].column << ": unopened jump back instruction\n";
            error = true;
        }
    }
    if (error)
    {
        return 1;
    }

    size_t size = 512;
    uint8_t *data = (uint8_t*)malloc(size);
    if (!data)
    {
        std::cerr << "failed to create array\n";
        return 1;
    }

    memset(data, 0, size); // fill with 0s to not have garbage
    int pointer = 0; //data "pointer"

    //run
    for(int i = 0; i < operations.size(); i++) // i is the instruction "pointer"
    {
        Operation op = operations[i];
        switch (op.type)
        {
        case INC:
            pointer += op.count;
            if (pointer >= size)
            {
                size_t newSize = size * 1.5f;
                uint8_t *newData = (uint8_t*)malloc(newSize);
                if(!newData)
                {
                    std::cerr << "failed to resize array\n";
                    return 1;
                }

                memset(newData, 0, newSize); //fill with 0s to not have garbage
                memcpy(newData, data, size);
                size = newSize;
                data = newData;
            }
            break;
        case DEC:
            pointer -= op.count;
            if (pointer < 0)
            {
                pointer = 0;
            }
            break;

        case ADD:
            data[pointer] += op.count;
            break;
        case SUB:
            data[pointer] -= op.count;
            break;

        case OUT:
            for(int c = 0; c < op.count; c++)
            {
                std::cout << data[pointer];
            }
            break;
        case IN:
            for(int c = 0; c < op.count; c++)
            {
                std::cin >> data[pointer];
            }
            break;

        case JUMP_FRONT:
            if (data[pointer] == 0)
            {
                i = op.jump_location;
            }
            break;
        case JUMP_BACK:
            if (data[pointer] != 0)
            {
                i = op.jump_location;
            }
            break;

        default:
            break;
        }
    }
}
