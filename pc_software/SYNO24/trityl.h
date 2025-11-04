#ifndef TRITYL_H
#define TRITYL_H
#include "qglobal.h"
#include "macro.h"
class trityl
{
public:
    uint16_t speacial_base[MAX_SEQUENCE_OF_WELL];
    bool bTritylProcessEnable;
    trityl();
    void cleardata();
};

#endif // TRITYL_H
