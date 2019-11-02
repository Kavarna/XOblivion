#include "Game.h"
#include <stdio.h>

#include <HasMethod.h>


int main()
{
    try
    {
        auto joc = Game::Get();
        joc->run();
        joc->reset();
    }
    catch (const std::exception& e)
    {
        ERROR(e.what());
    }
    catch (...)
    {
        ERROR("Unexpected error");
    }

    std::ofstream logs("logs.txt");

    Logger::dumpJson(logs);

    logs.close();
#if DEBUG || _DEBUG
    Logger::dumpJson(std::cout);
#endif
    return 0;

}