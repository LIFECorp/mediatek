#ifndef MTK_TEST_SURFACE_DUMP_H
#define MTK_TEST_SURFACE_DUMP_H

#include <utils/Singleton.h>

#include <stdio.h>


class Dump
{
public:
    virtual void dump() const = 0;
    virtual ~Dump() {};
};

class DumpFile : public android::Singleton<DumpFile>
{
};

#endif // MTK_TEST_SURFACE_DUMP_H
