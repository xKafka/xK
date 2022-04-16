//
// Created by kafka on 2/3/2022.
//

#include <application/app_core.h>
#include <utility/log.h>

int main(const int argc, const char* argv[])
{
    Application application{ argc, argv };

    try
    {
        application.run();
    }
    catch(const std::exception& e)
    {
        xgk::log::critical("{}", e.what());
    }

    return 0;
}

